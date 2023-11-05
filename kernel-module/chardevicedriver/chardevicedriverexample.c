/*
* chardevicedriverexample.c Creates a read only char device that says how many times
* you have read from the dev file.
* NOTE: kernel modules requires Tabs and not spaces in indentation.
*/

#include <linux/atomic.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h> /*Needed for sprintf function*/
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/types.h> 
#include <linux/uaccess.h> /*Needed for get_user and put_user*/
#include <asm/errno.h>


/*These will be moved to its own header file*/
static int device_open(struct inode *,struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t,loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "mychardev" //this name will show up in /proc/devices
#define BUFFER_LEN 80 /* Max length of the message from the device */

/* Global variables are declared as static, so are global within the file. */

static int major; //major number which will be defined to the driver

enum {
    CDEV_NOT_USED = 0;
    CDEV_EXCLUSIVE_OPEN = 1;
};

/* Check if device is open. Its used to prevent multiple access to device*/
static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED);

static char msg[BUF_LEN + 1]; /* The msg the device will give when asked */

static struct class *cls;

/*function pointers read,write,open,release in file_operations struct called
* by kernel syscall read,write,open,release pointing to mychardev driver functions 
*device_read, device_write,device_open, device_release */
static struct file_operations mychardev_fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release,
}

// module's init function starts
static int __init mychardev_init(void)
{
    //registering the character device driver and asking kernel to dynamically generate major number(by passing 0 as first arg) for device driver
    /*
    Ref: Linux Kernel Module Programming Guide Sec 6.2 Registering a device
    If you pass a major number of 0 to register_chrdev, the return value will
    be the dynamically allocated major number. The downside is that you can not
    make a device file in advance, since you do not know what the major number
    will be. There are a couple of ways to do this. First, the driver itself can print
    the newly assigned number and we can make the device file by hand. Second,
    the newly registered device will have an entry in /proc/devices, and we can
    either make the device file by hand or write a shell script to read the file in and
    make the device file. The third method is that we can have our driver make the
    device file using the device_create function after a successful registration and
    device_destroy during the call to cleanup_module.*/
    major = register_chardev(0, DEVICE_NAME, &mychardev_fops);

    if (major < 0) {
        pr_alert("Registering char device failed with %d\n", major);
        return major;
    }

    pr_info("Character Device Driver assigned major number %d.\n", major);

    cls = class_create(THIS_MODULE, DEVICE_NAME);
    device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);

    pr_info("Device created on /dev/%s\n", DEVICE_NAME);

    return SUCCESS;
}

// driver module cleanup function starts
static void __exit chardev_exit(void)
{
    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);

    /* Unregister the device */
    unregister_chrdev(major, DEVICE_NAME);
}

/*Driver methods definition starts 
static int device_open(struct inode *,struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t,loff_t *);
*/

