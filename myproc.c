#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define MAX_LEN       4096
static struct proc_dir_entry *proc_entry;

static char *infoBuf;
static int   infoLen = 0;


/* read from /proc/myproc */
ssize_t read_proc(struct file *f, char __user *user_buf, size_t count, loff_t *off )
{
	int bytesToCopy;

	/* only allow one read, then return 0 next time */
	if (*off > 0 || infoLen == 0)
		return 0;

	if (count < infoLen)
		bytesToCopy = count;
	else
		bytesToCopy = infoLen;

	if (copy_to_user(user_buf, infoBuf, bytesToCopy))
		return -EFAULT;

	*off = bytesToCopy;

	printk(KERN_INFO "procfs_read: read %d bytes\n", bytesToCopy);

	return bytesToCopy;
}


/* write to /proc/myproc */
ssize_t write_proc(struct file *f, const char __user *user_buf, size_t count, loff_t *off)
{
	int len;

	if (count > MAX_LEN - 1)
		len = MAX_LEN - 1;
	else
		len = count;

	if (copy_from_user(infoBuf, user_buf, len))
		return -EFAULT;

	infoBuf[len] = '\0';     /* make it a string */
	infoLen      = len;

	printk(KERN_INFO "procfs_write: write %d bytes\n", len);

	return count;
}


/* file operations for /proc/myproc */
static struct file_operations proc_fops = {
	.owner = THIS_MODULE,
	.read  = read_proc,
	.write = write_proc,
};


/* module init */
int init_module(void)
{
	int ret = 0;

	infoBuf = kmalloc(MAX_LEN, GFP_KERNEL);
	if (!infoBuf) {
		printk(KERN_INFO "could not allocate buffer\n");
		return -ENOMEM;
	}

	infoLen = 0;

	proc_entry = proc_create("myproc", 0666, NULL, &proc_fops);
	if (proc_entry == NULL) {
		printk(KERN_INFO "could not create /proc/myproc\n");
		kfree(infoBuf);
		return -ENOMEM;
	}

	printk(KERN_INFO "test_proc created.\n");

	return ret;
}


/* module cleanup */
void cleanup_module(void)
{
	if (proc_entry) {
		remove_proc_entry("myproc", NULL);
		proc_entry = NULL;
	}

	if (infoBuf) {
		kfree(infoBuf);
		infoBuf = NULL;
	}

	printk(KERN_INFO "test_proc deleted.\n");
}

MODULE_LICENSE("GPL");
