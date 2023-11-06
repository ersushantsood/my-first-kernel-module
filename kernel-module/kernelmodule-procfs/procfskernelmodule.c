/*
 *Here a simple example showing how to use a /proc file. This is the process file read handler for the /proc filesystem. 
 *There are three parts: create the file /proc/procfs_myread in the function init_module, return a value 
 *(and a buffer) when the file /proc/procfs_myread is read in the callback function procfile_read, and delete
 *the file /proc/procfs_myread in the function cleanup_module.

 *The /proc/procfs_myread is created when the module is loaded with the function proc_create. The return value 
 *is a struct proc_dir_entry, and it will be used to configure the file /proc/procfs_myread (for example, the owner of this
 *file). A null return value means that the creation has failed.Every time the file /proc/procfs_myread is read,
 *the function procfile_read is called. Two parameters of this function are very important: the buffer (the
 *second parameter) and the offset (the fourth one). The content of the buffer will be returned to the application 
 *which read it (for example the cat command).The offset is the current position in the file. If the return value
 *of the function is not null, then this function is called again. So be careful with this function,
 *if it never returns zero, the read function is called endlessly.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#inlcude <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

#define procfs_name "procfs_myread"

static struct proc_dir_entry *our_proc_file;

static ssize_t procfile_read(struct file *file_pointer, char __user *buffer, size_t buffer_length, loff_t *offset)
{
    char s[31] = "MyModule-ProcFile-ReadHandler\n";
    int len = sizeof(s);
    ssize_t ret = len;

    if (*offset >= len || copy_to_user(buffer, s, len)) {
        pr_info("copy_to_user failed\n");
        ret = 0;
    } else {
        pr_info("procfile read %s\n", file_pointer->f_path.dentry->d_name.name);
        *offset += len;
    }

    return ret;
}

#ifdef HAVA_PROC_OPS
static const struct proc_ops proc_file_fops = {
    .proc_read = procfile_read,
};
#else
static const struct file_operations proc_file_fops = {
    .read = procfile_read,
};
#endif

// initialization of the module starts
static int __init procfs_init(void)
{
    our_proc_file = proc_create(procfs_name, 0644, NULL, &proc_file_fops);
    if (NULL == our_proc_file) {
        proc_remove(our_proc_file);
        pr_alert("Error: Could not initialize /proc/%s\n", procfs_name);
        return -ENOMEM;
    }

    pr_info("/proc/%s created\n",procfs_name);
    return 0;
}

// cleanup function of the module
static void __exit procfs_exit(void)
{
    proc_remove(our_proc_file);
    pr_info("/proc/%s removed\n",procfs_name);
}

module_init(procfs_init);
module_exit(procfs_exit);

MODULE_LICENSE("GPL");