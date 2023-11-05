/*
 * macrokernelmodule.c - Example of the module_init() and module_exit() macros.
 * This pattern is preferred over using init_module() and cleanup_module() as
 * these init_module and cleanup_module are older patterns.
 */

#include <linux/init.h>  // header file required to include macros
#include <linux/module.h> // required by all the kernel modules to be functional
#include <linux/printk.h>

static int __init macro_module_init(void)
{
    pr_info("Hey This is macro module...");
    return 0;
}

static void __exit macro_module_exit(void)
{
    pr_info("Hey macro module is exiting...");
}

module_init(macro_module_init);
module_exit(macro_module_exit);

MODULE_LICENSE("GPL");