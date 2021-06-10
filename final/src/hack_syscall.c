#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/unistd.h>
#include <linux/kallsyms.h>


#define syscall_num __NR_clone
static unsigned long __lkm_order;

// 用于保存 sys_call_table 地址
unsigned long *sys_call_table;
// 用于保存被劫持的系统调用
static long (*anything_saved)(unsigned long, unsigned long, int __user *,
                              int __user *, unsigned long);

static inline unsigned long lkm_read_cr0(void)
{
    unsigned long val;
    asm volatile("mov %%cr0,%0\n\t"
                 : "=r"(val), "=m"(__lkm_order));
    return val;
}

static inline void lkm_write_cr0(unsigned long val)
{
    asm volatile("mov %0,%%cr0"
                 :
                 : "r"(val), "m"(__lkm_order));
}

// 关闭写保护
void disable_write_protection(void)
{
    unsigned long cr0 = lkm_read_cr0();
    clear_bit(16, &cr0);
    lkm_write_cr0(cr0);
}

// 开启写保护
void enable_write_protection(void)
{
    unsigned long cr0 = lkm_read_cr0();
    set_bit(16, &cr0);
    lkm_write_cr0(cr0);
}

unsigned long *get_sys_call_table(void)
{
    unsigned long i = kallsyms_lookup_name("sys_call_table");
    if (i == 0)
    {
        return NULL;
    }
    return (unsigned long *)i;
}

asmlinkage long my_syscall(unsigned long para1,
                         unsigned long para2,
                         int __user *para3,
                         int __user *para4,
                         unsigned long para5)
{
    
    long return_val = anything_saved(para1, para2, para3, para4, para5);
    printk(KERN_INFO "hello, I have hacked this syscall!!  current pid: %d (name: %s)", current->pid, current->comm);
    return return_val;
}

static int __init init_syscall(void)
{
    // 关闭写保护
    disable_write_protection();
    // 得到系统调用表地址
    sys_call_table = get_sys_call_table();
    printk(KERN_INFO "syscall table address: %px\n", sys_call_table);
    // 保存系统调用的地址
    anything_saved = sys_call_table[syscall_num];
    // 将系统调用更改为为自定义函数
    sys_call_table[syscall_num] = (unsigned long)my_syscall;
    // 开启写保护
    enable_write_protection();
    printk("modify syscall success!!\n");
    return 0;
}

static void __exit exit_syscall(void)
{
    // 关闭写保护
    disable_write_protection();
    // 恢复系统调用
    sys_call_table[syscall_num] = (unsigned long)anything_saved;
    // 开启写保护
    enable_write_protection();
    printk("exit syscall!!\n");
}

module_init(init_syscall);
module_exit(exit_syscall);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tongzhixin");
