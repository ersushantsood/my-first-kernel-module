## Kernel modules

Kernel modules are a way of extending functionality in existing kernel without rebuilding the current kernel.In order to create new kernel modules,
developers need to install kernel-headers for the specific kernel version on the machine.

```bash

sudo apt-get install kmod linux-headers-`uname -r`

```

### Compile the kernel module

I created very first kernel module and its code is written in kernel-module/firstKernelModule.c. To compile the kernel module, use the build 
tool of the kernel on the machine. Makefile is created in the kernel-module folder which contains the build command to build the kernel module

run make to compile the kernel module

```bash
make
```

Output of the make will be as below

![compilation](/kernel-module/images/kmodule-build.png)

### Verify kernel module , install and remove kernel module

Verify the information of the kernel module using below command

```bash
sudo modinfo firstKernelModule.ko
```

Install the kernel module into the kernel

```bash
sudo insmod firstKernelModule.ko
```

Remove the kernel module from the kernel

```bash
sudo rmmod firstKernelModule.ko
```

list journalctl logs to check the logs of the kernel module 

```bash
sudo journalctl -since "1 hour ago" | grep kernel
```

References
[Linux programming guide](https://sysprog21.github.io/lkmpg/)
[Linux source tree Makefile](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/drivers/char/Makefile)