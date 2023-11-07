/*
sysfs allows you to interact with the running kernel from userspace by reading or
setting variables inside of modules. This can be useful for debugging purposes.
You can find sysfs directories and files under the /sys directory on your system.

Attributes can be exported for kobjects in the form of regular files in the
filesystem. Sysfs forwards file I/O operations to methods defined for the attributes, 
providing a means to read and write kernel attributes.

Below module creates reads/updates attribute in a file in sysfs under its module directory.
*/
<
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/sysfs.h>

static struct kobject *mysysfsmodule;

/* the variable you want to change*/
static int syscustomvariable = 0;

static ssize_t syscustomvariable_show(struct kobject *kobj,
                                      struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", syscustomvariable);
}

static ssize_t syscustomvariable_store(struct kobject *kobj,
                                      struct kobj_attribute *attr, char *buf,
                                      size_t count)
{
    sscanf(buf, "%du", &syscustomvariable);
    return count;
}

// setting the setter and getter in kobject attribute
/*To read or write attributes, show() or store() method must be specified
when declaring the attribute. For the common cases include/linux/sysfs.h provides 
convenience macros (__ATTR, __ATTR_RO, __ATTR_WO, etc.) to make defining attributes 
easier as well as making code more concise and readable.*/
static struct kobj_attribute syscustomvariable_attribute =
    __ATTR(syscustomvariable, 0660, syscustomvariable_show, (void *)syscustomvariable_store);

// creating the sysfs file using module init and assigning the attribute to the file
static int __init kernelmodulesysfs_init(void)
{
    int error = 0;
    pr_info("kernelmodulesysfs_init: initialized\n");

    mysysfsmodule = kobject_create_and_add("mysysfsmodule", kernel_kobj);
    if (!mysysfsmodule)
        return -ENOMEM;
    
    error = sysfs_create_file(mysysfsmodule, &syscustomvariable_attribute.attr);
    if (error) {
        pr_info("failed to create the myvariable file "
                "in /sys/kernel/mymodule\n");
    }

    return error;
}

static void __exit kernelmodulesysfs_exit(void)
{
    pr_info("kernelmodulesysfs: Exit success\n");
    kobject_put(kernelmodulesysfs);
}

module_init(kernelmodulesysfs_init);
module_exit(kernelmodulesysfs_exit);

MODULE_LICENSE("GPL");

/*
sudo insmod hello-sysfs.ko

Check that it exists:
    sudo lsmod | grep hello_sysfs

What is the current value of myvariable ?
    cat /sys/kernel/mymodule/myvariable
Set the value of myvariable and check that it changed.
    echo "32" > /sys/kernel/mymodule/myvariable
    cat /sys/kernel/mymodule/myvariable
Finally, remove the test module:
    sudo rmmod hello_sysfs

In the above case, we use a simple kobject to create a directory under
sysfs, and communicate with its attributes. Since Linux v2.6.0, the kobject

structure made its appearance. It was initially meant as a simple way of uni-
fying kernel code which manages reference counted objects. After a bit of

mission creep, it is now the glue that holds much of the device model and
its sysfs interface together. For more information about kobject and sysfs,
see Documentation/driver-api/driver-model/driver.rst and https://lwn.net/
Articles/51437/.
*/