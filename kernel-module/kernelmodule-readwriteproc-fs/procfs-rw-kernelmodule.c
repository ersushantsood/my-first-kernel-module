/*
* Create a file in /proc and read and write to the file

We have seen a very simple example for a /proc file where we only read the
file /proc/helloworld. It is also possible to write in a /proc file. It works the
same way as read, a function is called when the /proc file is written. But there
is a little difference with read, data comes from user, so you have to import data
from user space to kernel space (with copy_from_user or get_user)
The reason for copy_from_user or get_user is that Linux memory (on Intel
architecture, it may be different under some other processors) is segmented. This
means that a pointer, by itself, does not reference a unique location in memory,
only a location in a memory segment, and you need to know which memory
segment it is to be able to use it. There is one memory segment for the kernel,
and one for each of the processes.

Note: Well, first of all keep in mind, there are rumors around, claiming that procfs 
is on its way out, consider using sysfs in-stead. Consider using this mechanism, in 
case you want to document something kernel related yourself.

To learn more about procfs, go through chapter 7 procfs and try seq_file,you can read this web page:
• https://lwn.net/Articles/22355/
• https://kernelnewbies.org/Documents/SeqFileHowTo
You can also read the code of fs/seq_file.c in the linux kernel.
*/

#include <linux/kernel.h> /*using kernel functions*/
#include <linux/module.h> /*Needed to create module*/
#include <linux/proc_fs.h> /*Needed because we us proc fs*/
#include <linux/uaccess.h> /*Using copy_from_user functions to copy data from user space and vice versa*/
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

#define PROCFS_MAX_SIZE 1024
#define PROCFS_NAME "buffer1k"

/* This structure holds information about /proc file*/
static struct proc_dir_entry *our_proc_file;

/*This buffer is used to store character for this module*/
static char procfs_buffer[PROCFS_MAX_SIZE];

/*The size of the buffer*/
static unsigned long procfs_buffer_size = 0;

/* This function is a callback when the /proc file is read */
static ssize_t procfile_read(struct file *file_pointer, char __user *buffer, size_t buffer_length, loff_t *offset)
{
    char s[23] = "IamProcFileSayingHola\n";
    int len = sizeof(s);
    ssize_t ret = len;

    if (*offset >= len || copy_to_user(buffer, s, len)) {
        pr_info("copy_to_user failed\n");
        ret = 0;
    } else {
        pr_info("procfile read %s\n",
        file_pointer->f_path.dentry->d_name.name);
        *offset += len;
    }

    return ret;  
}

/* This function is called when the /proc file is written or write operation is invoked on file in /proc */
static ssize_t procfile_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset)
{
    procfs_buffer_size = len;
    if (procfs_buffer_size > PROCFS_MAX_SIZE)
        procfs_buffer_size = PROCFS_MAX_SIZE;
    
    /* taking data written by user in user space using copy_from_user method from user buffer "buffer" to 
       into module's local buffer procfs_buffer*/
    if (copy_from_user(procfs_buffer, buffer, procfs_buffer_size))
        return -EFAULT;
    
    procfs_buffer[procfs_buffer_size & (PROCFS_MAX_SIZE -1)] = '\0';
    *offset += procfs_buffer_size;
    pr_info("procfile write %s\n", procfs_buffer);

    return procfs_buffer_size;
}

// assigning function/hooks to proc_ops struct function pointers
#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_file_fops = {
    .proc_read = procfile_read,
    .proc_write = procfile_write,
};
#else
static const struct file_operations proc_file_fops = {
    .read = procfile_read,
    .write = procfile_write,
};
#endif

static int __init procfs-rw-kernelmodule_init(void)
{
    our_proc_file = proc_create(PROCFS_NAME, 0644, NULL, &proc_file_fops);
    if (NULL == our_proc_file) {
        proc_remove(our_proc_file);
        pr_alert("Error:Could not initialize /proc/%s\n", PROCFS_NAME);
        return -ENOMEM;
    }

    pr_info("/proc/%s created\n", PROCFS_NAME);
    return 0;
}

static void __exit procfs-rw-kernelmodule_exit(void)
{
    proc_remove(our_proc_file);
    pr_info("/proc/%s removed\n", PROCFS_NAME);
}

module_init(procfs-rw-kernelmodule_init);
module_exit(procfs-rw-kernelmodule_exit);

MODULE_LICENSE("GPL");

