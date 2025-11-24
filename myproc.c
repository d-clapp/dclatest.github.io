#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define MAX_LEN 4096

static struct proc_dir_entry *proc_entry;
static char *infoBuf;
static int infoLen = 0;

/* READ */
ssize_t read_proc(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
    int bytes;

    if (*ppos > 0 || infoLen == 0)
        return 0;

    bytes = (count < infoLen) ? count : infoLen;

    if (copy_to_user(user_buf, infoBuf, bytes))
        return -EFAULT;

    *ppos = bytes;

    printk(KERN_INFO "procfs_read: read %d bytes\n", bytes);
    return bytes;
}

/* WRITE */
ssize_t write_proc(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
    int len = (count < MAX_LEN - 1) ? count : MAX_LEN - 1;

    if (copy_from_user(infoBuf, user_buf, len))
        return -EFAULT;

    infoBuf[len] = '\0';
    infoLen = len;

    printk(KERN_INFO "procfs_write: wrote %d bytes\n", len);
    return count;
}

/* Must use struct proc_ops for modern kernels */
static const struct proc_ops proc_fops = {
    .proc_read  = read_proc,
    .proc_write = write_proc,
};

int init_module(void)
{
    infoBuf = kmalloc(MAX_LEN, GFP_KERNEL);
    if (!infoBuf)
        return -ENOMEM;

    infoLen = 0;

    proc_entry = proc_create("myproc", 0666, NULL, &proc_fops);
    if (!proc_entry) {
        kfree(infoBuf);
        printk(KERN_INFO "Could not create /proc/myproc\n");
        return -ENOMEM;
    }

    printk(KERN_INFO "myproc created\n");
    return 0;
}

void cleanup_module(void)
{
    if (proc_entry)
        remove_proc_entry("myproc", NULL);

    if (infoBuf)
        kfree(infoBuf);

    printk(KERN_INFO "myproc removed\n");
}

MODULE_LICENSE("GPL");
