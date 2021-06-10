#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/string.h>       // 字符串操作
#include <linux/sched/signal.h> // foreachprocess宏
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h> // for copy_from_user
#include <linux/mm_types.h> //vma
#include <linux/mm.h>

#define FILE_NAME "mtest"
#define BUF_SIZE 100
char global_buffer[BUF_SIZE];
static struct proc_dir_entry *entry;
/*Print all vma of the current process*/
static void mtest_list_vma(void)
{
    struct mm_struct *mm;
    struct vm_area_struct *vma;
    struct task_struct *p;
    p = current;
    printk(KERN_ALERT "the process is \"%s\" (pid %i),and process status is  %i \n", p->comm, p->pid, p->flags);
    if (!p->mm)
    {
        printk(KERN_ALERT "the process is \"%s\" (pid %i),and process status is  %i,没有mm数据 \n", p->comm, p->pid, p->flags);
    }
    mm = p->mm;
    down_read(&mm->mmap_sem);
    if (!p->mm->mmap)
    {
        printk(KERN_ALERT "the process is \"%s\" (pid %i),and process status is  %i,no mmap \n", p->comm, p->pid, p->flags);
        up_read(&mm->mmap_sem);
    }
    for (vma = p->mm->mmap; vma; vma = vma->vm_next)
    {
        printk(KERN_INFO "0x%lx - 0x%lx ", vma->vm_start, vma->vm_end);
        if (vma->vm_flags & VM_READ)
        {
            printk(KERN_INFO "r");
        }
        else
        {
            printk(KERN_INFO "-");
        }

        if (vma->vm_flags & VM_WRITE)
        {
            printk(KERN_INFO "w");
        }
        else
        {
            printk(KERN_INFO "-");
        }

        if (vma->vm_flags & VM_EXEC)
        {
            printk(KERN_INFO "x");
        }
        else
        {
            printk(KERN_INFO "-");
        }

        if (vma->vm_flags & VM_SHARED)
        {
            printk(KERN_INFO "s");
        }
        else
        {
            printk(KERN_INFO "p");
        }
        printk(KERN_INFO "\n");
    }
    up_read(&mm->mmap_sem);
}
/*Find va->pa translation */
static unsigned long mtest_find_page(unsigned long addr)
{
    struct mm_struct *mm;
    struct task_struct *p;
    unsigned long physical_page_addr;
    unsigned long physical_page_addr1;
    unsigned long page_addr = 0;
    unsigned long page_offset = 0;
    p = current;
    mm = p->mm;
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    struct page *page;
    pgd = pgd_offset(mm, addr);
    if (pgd_none(*pgd) || pgd_bad(*pgd))
    {
        printk(KERN_INFO "5 level error \n");
        printk(KERN_INFO "translation not found \n");
        return 0;
    }
    p4d = p4d_offset(pgd, addr);
    if (p4d_none(*p4d) || p4d_bad(*p4d))
    {
        printk(KERN_INFO "4 level error \n");
        printk(KERN_INFO "translation not found \n");
        return 0;
    }
    pud = pud_offset(p4d, addr);
    if (pud_none(*pud) || pud_bad(*pud))
    {
        printk(KERN_INFO "3 level error \n");
        printk(KERN_INFO "translation not found \n");
        return 0;
    }
    pmd = pmd_offset(pud, addr);
    if (pmd_none(*pmd) || pmd_bad(*pmd))
    {
        printk(KERN_INFO "2 level error \n");
        printk(KERN_INFO "translation not found \n");
        return 0;
    }
    if (!(pte = pte_offset_kernel(pmd, addr)))
    {
        printk(KERN_INFO "1 level error \n");
        printk(KERN_INFO "translation not found \n");
        return 0;
    }
    if (!(page = pte_page(*pte)))
    {
        printk(KERN_INFO "translation not found \n");
        return 0;
    }
    page_addr = page_to_phys(page) & PAGE_MASK;
    page_offset = addr & ~PAGE_MASK;
    //printk(KERN_INFO "the page_addr %lx \n", page_addr);
    //printk(KERN_INFO "the page_offset %lx \n", page_offset);
    physical_page_addr = page_addr | page_offset;
    physical_page_addr1 = phys_to_virt(physical_page_addr);
    printk(KERN_INFO "the physical_page_addr is 0x%lx \n", physical_page_addr);
    printk(KERN_INFO "the kernel addr is 0x%lx \n", physical_page_addr1);
    return physical_page_addr;
}
/*Write val to the specified address */
static void mtest_write_val(unsigned long addr, unsigned long val)
{
    struct mm_struct *mm;
    struct vm_area_struct *vma;
    struct task_struct *p;
    p = current;
    mm = p->mm;
    down_read(&mm->mmap_sem);
    if (!p->mm->mmap)
    {
        printk(KERN_ALERT "the process is %s nommap \n", p->comm);
        up_read(&mm->mmap_sem);
        return;
    }
    for (vma = p->mm->mmap; vma; vma = vma->vm_next)
    {
        printk(KERN_INFO "0x%lx - 0x%lx   ", vma->vm_start, vma->vm_end);
        if (addr < vma->vm_end && addr >= vma->vm_start)
        {
            if (vma->vm_flags & VM_WRITE)
            {
                unsigned long physical_page_addr = mtest_find_page(addr);
                up_read(&mm->mmap_sem);
                unsigned long *tmp_val;
                tmp_val = phys_to_virt(physical_page_addr);
                *tmp_val = val;
                printk(KERN_INFO "tmp_val is %lx \n ", *tmp_val);
                printk(KERN_INFO "\n ");
                return;
            }
            else
            {
                up_read(&mm->mmap_sem);
                printk(KERN_INFO "can not write \n");
                return;
            }
        }
    }
    up_read(&mm->mmap_sem);
    printk(KERN_INFO "can not find the address! \n");
}
static ssize_t mtest_proc_write(struct file *file,
                                const char __user *buffer,
                                size_t count, loff_t *data)
{
    int len;
    if (count < BUF_SIZE)
        len = count;
    else
        len = BUF_SIZE;
    copy_from_user(global_buffer, buffer, len);
    global_buffer[len - 1] = ' ';
    global_buffer[len] = '\0';
    char tmp_buffer[BUF_SIZE];
    char *iter = tmp_buffer;
    char delim[] = " ";
    char *token, *cur = strcpy(iter, global_buffer);
    token = strsep(&cur, delim);
    if (strcmp(token, "listvma") == 0)
    {
        mtest_list_vma();
    }
    else if (strcmp(token, "findpage") == 0)
    {
        unsigned long addr;
        while (token = strsep(&cur, delim))
        {
            if (strcmp(token, "") == 0)
                continue;
            kstrtoul(token, 0, &addr);
            break;
        }
        mtest_find_page(addr);
    }
    else if (strcmp(token, "writeval") == 0)
    {
        unsigned long addr;
        unsigned long val;
        int count = 0;
        while (token = strsep(&cur, delim))
        {
            if (strcmp(token, "") == 0)
                continue;
            if (count == 2)
                continue;
            if (count == 0)
            {
                kstrtoul(token, 0, &addr);
                ++count;
            }
            else if (count == 1)
            {
                kstrtoul(token, 0, &val);
                ++count;
            }
        }
        mtest_write_val(addr, val);
    }
    return len + 1;
}
static struct file_operations proc_mtest_operations = {
    .write = mtest_proc_write};
static struct proc_dir_entry *mtest_proc_entry;
static int __init mtest_init(void)
{
    printk(KERN_INFO "mtest init ! \n");
    entry = proc_create(FILE_NAME, 0777, NULL, &proc_mtest_operations);
    if (!entry)
        return -1;
    else
    {
        printk(KERN_INFO "create file successfully!\n");
        return 0;
    }
}
static void __exit mtest_exit(void)
{
    printk(KERN_INFO "mtest exit ! \n");
    remove_proc_entry(FILE_NAME, NULL);
}
module_init(mtest_init);
module_exit(mtest_exit);
MODULE_LICENSE("GPL");