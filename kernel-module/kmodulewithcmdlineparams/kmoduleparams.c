/*
*Passing command line arguments to the module
*
*/

#include <linux/init.h>
#include <linux/kernel.h> // to use ARRAY_SIZE() macro
#incldue <linux/module.h>
#include <linux/moduleparams.h>
#include <linux/printk.h>
#include <linux/stat.h>

MODULE_LICENSE("GPL")

// declaring the variables
static short int shortarg = 1;
static int intarg = 43;
static long int longarg = 8888;
static char *stringarg = "cmdline";
static int arrayarg[2] = {1,2};
static int arr_argc = 0;

/* module_param(foo, int, 0000)
* The first param is the parameters name.
* The second param is its data type.
* The final argument is the permissions bits,
* for exposing parameters in sysfs (if non-zero) at a later stage.
*/
module_param(shortarg, short, S_IRUSR| S_IWUSR| S_IRGRP| S_IWGRP);
MODULE_PARM_DESC(shortarg, "A short integer");
module_param(intarg, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(intarg, "An integer");
module_param(longarg, long, S_IRUSR);
MODULE_PARM_DESC(longarg, "A long integer");
module_param(stringarg, charp, 0000);
MODULE_PARM_DESC(stringarg, "A character string");

/* module_param_array(name, type, num, perm);
* The first param is the parameter's (in this case the array's) name.
* The second param is the data type of the elements of the array.
* The third argument is a pointer to the variable that will store the number
* of elements of the array initialized by the user at module loading time.
* The fourth argument is the permission bits.
*/
module_param_array(arrayarg, int, &arr_argc, 0000);
MODULE_PARM_DESC(arrayarg, "An array of integers");

static int __init hello_5_init(void)
{
    int i;

    pr_info("Hello, world 5\n=============\n");
    pr_info("myshort is a short integer: %hd\n", myshort);
    pr_info("myint is an integer: %d\n", myint);
    pr_info("mylong is a long integer: %ld\n", mylong);
    pr_info("mystring is a string: %s\n", mystring);

    for (i = 0; i < ARRAY_SIZE(myintarray); i++)
        pr_info("myintarray[%d] = %d\n", i, myintarray[i]);

    pr_info("got %d arguments for myintarray.\n", arr_argc);
    return 0;
}

static void __exit hello_5_exit(void)
{
    pr_info("Goodbye, world 5\n");
}

module_init(hello_5_init);
module_exit(hello_5_exit);