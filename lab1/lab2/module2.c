#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#define MAX_SIZE 10
static int int_var = 2020;
static char *str_var = "tongzhixin";
static int int_array[MAX_SIZE] = {};
static int n_para = 10;
module_param(int_var,int,S_IRUGO);
module_param(str_var,charp,S_IRUGO);
module_param_array(int_array,int,&n_para,S_IRUGO);
static int __init homework2_init(void)
{
int i;
printk(KERN_INFO "finish homework2 init()!!");
printk(KERN_INFO "int_var=%d",int_var);
printk(KERN_INFO "str_var=%s",str_var);
for (i=0;i<n_para;i++)
{
printk(KERN_INFO "int_array[%d]=%d",i,int_array[i]);
}
return 0;
}
static void __exit homework2_exit(void)
{
printk(KERN_INFO "finish homework2 exit()!!");
}

module_init(homework2_init);
module_exit(homework2_exit);
MODULE_LICENSE("GPL");