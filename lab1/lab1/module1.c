#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
static int __init homework1_init(void)
{
printk(KERN_INFO "finish homework1 init()!!");
return 0;
}
static void __exit homework1_exit(void)
{
printk(KERN_INFO "finish homework1 exit()!!");
printk(KERN_INFO "");
}

module_init(homework1_init);
module_exit(homework1_exit);
MODULE_LICENSE("GPL");
