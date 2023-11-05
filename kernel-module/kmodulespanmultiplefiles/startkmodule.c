/*
* startkmodule.c
*/
#include <linux/module.h> /*This header is needed by all the modules*/
#include <linux/printk.h> /* This header is needed to print statements*/

int init_module(void)
{
    pr_info("Hello First Module 1.\n");

    /* A non 0 return means init_module failed; module can't be loaded. */
    return 0;
}

MODULE_LICENSE("GPL");