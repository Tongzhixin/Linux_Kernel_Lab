#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>       // for kzalloc, kfree  
#include <linux/uaccess.h>    // for copy_from_user  
#define BUF_SIZE 100
char global_buffer[BUF_SIZE];
#define BASE_DIR_NAME  "hello_proc"
#define FILE_NAME "hello"
static struct proc_dir_entry *file_entry;
static struct proc_dir_entry *dir_entry;

static int hello_proc_show(struct seq_file *m, void *v) {
  seq_printf(m, "Hello proc!\n");
  seq_printf(m, "global_buffer is %s\n", global_buffer);
  return 0;
}

static int hello_proc_open(struct inode *inode, struct  file *file) {
  return single_open(file, hello_proc_show, NULL);
}

static ssize_t hello_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *f_pos){
  int len;
  if(count<BUF_SIZE)len=count;
  else len = BUF_SIZE;
  copy_from_user(global_buffer, buffer, len);
  global_buffer[len-1]='\0';
	return len;
}
static const struct file_operations fops={
    .owner =THIS_MODULE,
    .read = seq_read,
    .open = hello_proc_open,
    .write = hello_proc_write,
    .llseek = seq_lseek,
    .release = single_release,
};

static int __init homework4_init(void)
{
    printk(KERN_INFO "homework4 init()!!");
    dir_entry = proc_mkdir(BASE_DIR_NAME, NULL);
    if(!dir_entry){
      printk(KERN_INFO "proc create %s dir failed\n", BASE_DIR_NAME);
      return -1;
    }
    file_entry = proc_create(FILE_NAME, 0777, dir_entry, &fops);
    if(!file_entry)
    return -1;
    else{
      printk(KERN_INFO "created!");
      return 0;
    } 
} 
static void __exit homework4_exit(void)
{
  printk(KERN_INFO "finish homework4 exit()!!");
  remove_proc_entry(FILE_NAME, dir_entry);
  remove_proc_entry(BASE_DIR_NAME,NULL);
  
}
module_init(homework4_init);
module_exit(homework4_exit);
MODULE_LICENSE("GPL");