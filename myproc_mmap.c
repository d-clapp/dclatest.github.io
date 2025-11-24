#include <linux/module.h>
#include <linux/list.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kthread.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/string.h>

static struct proc_dir_entry *tempdir, *tempinfo;
static unsigned char *buffer;
static unsigned char array[12] = {0,1,2,3,4,5,6,7,8,9,10,11};

static void allocate_memory(void);
static void clear_memory(void);
static int my_map(struct file *filp, struct vm_area_struct *vma);

/* use struct proc_ops for /proc/mydir/myinfo */
static const struct proc_ops myproc_fops = {
	.proc_mmap = my_map,
};

static int my_map(struct file *filp, struct vm_area_struct *vma)
{
	unsigned long size;
	struct page *page;
	unsigned long pfn;

	size = vma->vm_end - vma->vm_start;

	/* only map one page */
	if (size > PAGE_SIZE) {
		pr_info("myproc: requested size too big\n");
		return -EINVAL;
	}

	if (!buffer) {
		pr_info("myproc: buffer is NULL\n");
		return -EINVAL;
	}

	/* get the page and PFN for our buffer */
	page = virt_to_page(buffer);
	pfn  = page_to_pfn(page);

	/* map the kernel page into user space */
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    pfn,
			    size,
			    vma->vm_page_prot)) {
		pr_info("myproc: remap_pfn_range failed\n");
		return -EAGAIN;
	}

	pr_info("myproc: mmap successful\n");
	return 0;
}

static int init_myproc_module(void)
{
	tempdir = proc_mkdir("mydir", NULL);
	if (tempdir == NULL) {
		pr_info("mydir is NULL\n");
		return -ENOMEM;
	}

	tempinfo = proc_create("myinfo", 0, tempdir, &myproc_fops);
	if (tempinfo == NULL) {
		pr_info("myinfo is NULL\n");
		remove_proc_entry("mydir", NULL);
		return -ENOMEM;
	}

	pr_info("init myproc module successfully\n");

	allocate_memory();

	return 0;
}

static void allocate_memory(void)
{
	/* allocate one page of memory */
	buffer = (unsigned char *)kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!buffer) {
		pr_info("myproc: kmalloc failed\n");
		return;
	}

	/* copy the array into the beginning of the page */
	memcpy(buffer, array, sizeof(array));

	/* on newer kernels SetPageReserved is deprecated, so we skip it */
	/* SetPageReserved(virt_to_page(buffer)); */
}

static void clear_memory(void)
{
	if (!buffer)
		return;

	/* ClearPageReserved(virt_to_page(buffer)); */

	kfree(buffer);
	buffer = NULL;
}

static void exit_myproc_module(void)
{
	clear_memory();

	if (tempinfo)
		remove_proc_entry("myinfo", tempdir);

	if (tempdir)
		remove_proc_entry("mydir", NULL);

	pr_info("remove myproc module successfully\n");
}

module_init(init_myproc_module);
module_exit(exit_myproc_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student");
MODULE_DESCRIPTION("Proc mmap example for CS3502");
