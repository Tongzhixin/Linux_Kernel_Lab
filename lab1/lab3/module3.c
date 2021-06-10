#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

static struct proc_dir_entry *entry;
static int hello_proc_show(struct seq_file *m, void *v) {
  seq_printf(m, "homework3 finished, the file can be read!\n");
  return 0;
}

static int hello_proc_open(struct inode *inode, struct  file *file) {
  return single_open(file, hello_proc_show, NULL);
}
static const struct file_operations fops={
    .owner =THIS_MODULE,
    .read = seq_read,
    .open = hello_proc_open,
};

static int __init homework3_init(void)
{
    printk(KERN_INFO "homework3 init()!!");
    
    entry = proc_create("hello_proc",0444,NULL,&fops);
    if(!entry)
    return -1;
    else{
        printk(KERN_INFO "create successfully!\n");
        return 0;
    } 
}
static void __exit homework3_exit(void)
{
  printk(KERN_INFO "finish homework3 exit()!!");
  remove_proc_entry("hello_proc", NULL);
  
}

module_init(homework3_init);
module_exit(homework3_exit);
MODULE_LICENSE("GPL");