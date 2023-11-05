/*
* startkmodule.c
*/
#include <linux/module.h> /*This header is needed by all the modules*/
#include <linux/printk.h> /* This header is needed to print statements*/

void cleanup_module(void)
{
    pr_info("Cleaning the module 1.\n");
}

MODULE_LICENSE("GPL");