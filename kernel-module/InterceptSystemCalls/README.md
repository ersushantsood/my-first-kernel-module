### Info about system calls (Ref: [Linux Kernel Module Programming Guide](https://sysprog21.github.io/lkmpg/))

This program killed the open() system call. After killing open() syscall,
one could not open any files, one could not run any programs, and one could not 
shutdown the system. One need to restart the virtual machine. No important files 
got annihilated, but if you are doing this on some live mission critical system 
then that could have been a possible outcome.To ensure you do not lose any files, 
even within a test environment, please run sync right before you do the insmod and the rmmod.

Basics : The real process to kernel communication mechanism, the one used by all processes, 
is system calls. When a process requests a service from the kernel (such as opening a file, 
forking to a new process, or requesting more memory), this is the mechanism used.

In general, a process is not supposed to be able to access the kernel. It can
not access kernel memory and it can’t call kernel functions. The hardware of
the CPU enforces this (that is the reason why it is called “protected mode” or
“page protection”).

System calls are an exception to this general rule. What happens is that the
process fills the registers with the appropriate values and then calls a special
instruction which jumps to a previously defined location in the kernel (of course,
that location is readable by user processes, it is not writable by them).

UnderIntel CPUs, this is done by means of interrupt 0x80. The hardware knows that
once you jump to this location, you are no longer running in restricted user
mode, but as the operating system kernel — and therefore you’re allowed to do
whatever you want.

The location in the kernel a process can jump to is called system_call.
The procedure at that location checks the system call number, which tells
the kernel what service the process requested. Then, it looks at the table
of system calls (sys_call_table) to see the address of the kernel function
to call. Then it calls the function, and after it returns, does a few system
checks and then return back to the process (or to a different process, if the
process time ran out). If you want to read this code, it is at the source file
arch/$(architecture)/kernel/entry.S, after the line ENTRY(system_call).

Now Intercepting syscalls:
So, if we want to change the way a certain system call works, what we need
to do is to write our own function to implement it (usually by adding a bit of our
own code, and then calling the original function) and then change the pointer at
sys_call_table to point to our function. Because we might be removed later
and we don’t want to leave the system in an unstable state, it’s important for
cleanup_module to restore the table to its original state.

To modify the content of sys_call_table, we need to consider the control
register. A control register is a processor register that changes or controls the
general behavior of the CPU.

For x86 architecture, the cr0 register has various
control flags that modify the basic operation of the processor. The WP flag in
cr0 stands for write protection. Once the WP flag is set, the processor disallows
further write attempts to the read-only sections Therefore, we must disable the
WP flag before modifying sys_call_table. Since Linux v5.3, the write_cr0
function cannot be used because of the sensitive cr0 bits pinned by the security
issue, the attacker may write into CPU control registers to disable CPU protec-
tions like write protection. As a result, we have to provide the custom assembly
routine to bypass it.

However, sys_call_table symbol is unexported to prevent misuse. But
there have few ways to get the symbol, manual symbol lookup and kallsyms_lookup_name.
Here we use both depend on the kernel version.Because of the control-flow integrity, 
which is a technique to prevent the redirect execution code from the attacker,
for making sure that the indirect calls go to the expected addresses and the return 
addresses are not changed. 

Since Linux v5.7, the kernel patched the series of control-flow enforcement (CET) for
x86, and some configurations of GCC, like GCC versions 9 and 10 in Ubuntu,
will add with CET (the -fcf-protection option) in the kernel by default.
Using that GCC to compile the kernel with retpoline off may result in CET
being enabled in the kernel. You can use the following command to check out
the -fcf-protection option is enabled or not:
$ gcc -v -Q -O2 --help=target | grep protection
Using built-in specs.
COLLECT_GCC=gcc
COLLECT_LTO_WRAPPER=/usr/lib/gcc/x86_64-linux-gnu/9/lto-wrapper
...
gcc version 9.3.0 (Ubuntu 9.3.0-17ubuntu1~20.04)
COLLECT_GCC_OPTIONS='-v' '-Q' '-O2' '--help=target' '-mtune=generic' '-march=x86-64'
/usr/lib/gcc/x86_64-linux-gnu/9/cc1 -v ... -fcf-protection ...
GNU C17 (Ubuntu 9.3.0-17ubuntu1~20.04) version 9.3.0 (x86_64-linux-gnu)
...
But CET should not be enabled in the kernel, **it may break the Kprobes and bpf**.
Consequently, CET is disabled since v5.11. To guarantee the manual symbol
lookup worked, we only use up to v5.4.

Unfortunately, since Linux v5.7 kallsyms_lookup_name is also unexported,
it needs certain trick to get the address of kallsyms_lookup_name. If CONFIG_KPROBES
is enabled, we can facilitate the retrieval of function addresses by means of
Kprobes to dynamically break into the specific kernel routine. Kprobes inserts
a breakpoint at the entry of function by replacing the first bytes of the probed
instruction. When a CPU hits the breakpoint, registers are stored, and the
control will pass to Kprobes. It passes the addresses of the saved registers and
the Kprobe struct to the handler you defined, then executes it. Kprobes can be
registered by symbol name or address. Within the symbol name, the address
will be handled by the kernel.

Otherwise, specify the address of sys_call_table from /proc/kallsyms
and /boot/System.map into sym parameter. Following is the sample usage for
/proc/kallsyms:
$ sudo grep sys_call_table /proc/kallsyms
ffffffff82000280 R x32_sys_call_table
ffffffff820013a0 R sys_call_table
ffffffff820023e0 R ia32_sys_call_table
$ sudo insmod syscall.ko sym=0xffffffff820013a0

Using the address from /boot/System.map, be careful about KASLR (Ker-
nel Address Space Layout Randomization). KASLR may randomize the address
of kernel code and data at every boot time, such as the static address listed
in /boot/System.map will offset by some entropy. The purpose of KASLR is
to protect the kernel space from the attacker. Without KASLR, the attacker
may find the target address in the fixed address easily. Then the attacker can
use return-oriented programming to insert some malicious codes to execute or
receive the target data by a tampered pointer. KASLR mitigates these kinds of
attacks because the attacker cannot immediately know the target address, but a
brute-force attack can still work. If the address of a symbol in /proc/kallsyms
is different from the address in /boot/System.map, KASLR is enabled with the
kernel, which your system running on.

$ grep GRUB_CMDLINE_LINUX_DEFAULT /etc/default/grub
GRUB_CMDLINE_LINUX_DEFAULT="quiet splash"
$ sudo grep sys_call_table /boot/System.map-$(uname -r)
ffffffff82000300 R sys_call_table
$ sudo grep sys_call_table /proc/kallsyms
ffffffff820013a0 R sys_call_table
# Reboot
$ sudo grep sys_call_table /boot/System.map-$(uname -r)
ffffffff82000300 R sys_call_table
$ sudo grep sys_call_table /proc/kallsyms
ffffffff86400300 R sys_call_table
If KASLR is enabled, we have to take care of the address from /proc/kallsyms
each time we reboot the machine. In order to use the address from /boot/System.map,
make sure that KASLR is disabled. You can add the nokaslr for disabling KASLR
in next booting time:

$ grep GRUB_CMDLINE_LINUX_DEFAULT /etc/default/grub
GRUB_CMDLINE_LINUX_DEFAULT="quiet splash"
$ sudo perl -i -pe 'm/quiet/ and s//quiet nokaslr/' /etc/default/grub
$ grep quiet /etc/default/grub
GRUB_CMDLINE_LINUX_DEFAULT="quiet nokaslr splash"
$ sudo update-grub

For more information, check out the following:

• [Cook: Security things in Linux v5.3](https://lwn.net/Articles/804849/)
• [Unexporting the system call table](https://lwn.net/Articles/12211/)
• [Control-flow integrity for the kernel](https://lwn.net/Articles/810077/)
• [Unexporting kallsyms_lookup_name](https://lwn.net/Articles/813350/)
• [Kernel Probes (Kprobes)](https://www.kernel.org/doc/Documentation/kprobes.txt)
• [Kernel address space layout randomization](https://lwn.net/Articles/569635/)

The source code here is an example of such a kernel module. We want to
“spy” on a certain user, and to pr_info() a message whenever that user opens
a file. Towards this end, we replace the system call to open a file with our own
function, called our_sys_openat. This function checks the uid (user’s id) of the
current process, and if it is equal to the uid we spy on, it calls pr_info() to
display the name of the file to be opened. Then, either way, it calls the original
openat() function with the same parameters, to actually open the file.
The init_module function replaces the appropriate location in sys_call_table
and keeps the original pointer in a variable. The cleanup_module function uses
that variable to restore everything back to normal. This approach is danger-
ous, because of the possibility of two kernel modules changing the same system
call. Imagine we have two kernel modules, A and B. A’s openat system call
will be A_openat and B’s will be B_openat. Now, when A is inserted into the
kernel, the system call is replaced with A_openat, which will call the original
sys_openat when it is done. Next, B is inserted into the kernel, which replaces
the system call with B_openat, which will call what it thinks is the original
system call, A_openat, when it’s done.
Now, if B is removed first, everything will be well — it will simply restore
the system call to A_openat, which calls the original. However, if A is removed
and then B is removed, the system will crash. A’s removal will restore the
system call to the original, sys_openat, cutting B out of the loop. Then, when
B is removed, it will restore the system call to what it thinks is the original,
A_openat, which is no longer in memory. At first glance, it appears we could
solve this particular problem by checking if the system call is equal to our open
function and if so not changing it at all (so that B won’t change the system call
when it is removed), but that will cause an even worse problem. When A is
removed, it sees that the system call was changed to B_openat so that it is no
longer pointing to A_openat, so it will not restore it to sys_openat before it is
removed from memory. Unfortunately, B_openat will still try to call A_openat
which is no longer there, so that even without removing B the system would
crash.

Note that all the related problems make syscall stealing unfeasible for pro-
duction use. In order to keep people from doing potential harmful things
sys_call_table is no longer exported. This means, if you want to do some-
thing more than a mere dry run of this example, you will have to patch your
current kernel in order to have sys_call_table exported.