/* Info about system calls

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

System call "stealing" sample.

Disables page protection at a processor level by changing the 16th bit in the
cr0 register (could be Intel specific)
* Based on example by Peter Jay Salzman and
* https://bbs.archlinux.org/viewtopic.php?id=139406
*/

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h> //to accept params
#include <linux/unistd.h> // The list of system calls
#include <linux/cred.h> // to get current_uid()
#include <linux/uidgid.h> // for __kuid_val()
#include <linux/version.h>

/* For the current (process) structure, we need this to know who the
* current user is.
*/
#include <linux/sched.h>
#include <linux/uaccess.h>

/* The way we access "sys_call_table" varies as kernel internal changes.
* - Prior to v5.4 : manual symbol lookup
* - v5.5 to v5.6 : use kallsyms_lookup_name()
* - v5.7+ : Kprobes or specific kernel module parameter
*/

/* The in-kernel calls to the ksys_close() syscall were removed in Linux
   v5.11+.
*/

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 7, 0))

#if LINUX_VERSION_CODE <= KERNEL_VERSION(5, 7, 0)
#define HAVE_KSYS_CLOSE 1
#include <linux/syscalls.h> // For ksys_close()
#else
#include <linux/kallsyms.h> // For kallsyms_lookup_name
#endif

#else

#if defined(CONFIG_KPROBES)
#define HAVE_KPROBES 1
#include <linux/kprobes.h>
#else
#define HAVE_PARAM 1
#include <linux/kallsyms.h> /* For sprint_symbol */
/* The address of the sys_call_table, which can be obtained with looking up
 * "/boot/System.map" or "/proc/kallsyms". When the kernel version is v5.7+,
 * without CONFIG_KPROBES, you can input the parameter or the module will look
 * up all the memory.
 */