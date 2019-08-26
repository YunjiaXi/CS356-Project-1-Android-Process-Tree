#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/sched.h>
#include<linux/unistd.h>
#include <linux/syscalls.h>
#include <linux/slab.h>     //alloction 
#include <linux/uaccess.h>	//kernel--user
#include <linux/list.h>     //list_head

MODULE_LICENSE("Dual BSD/GPL");
#define __NR_pstreecall 356

struct prinfo {
    pid_t parent_pid; /* process id of parent */
    pid_t pid; /* process id */
    pid_t first_child_pid; /* pid of youngest child */
    pid_t next_sibling_pid; /* pid of older sibling */
    long state; /* current state of process */
    long uid; /* user id of process owner */
    char comm[64]; /* name of program executed */
};

void transTaskToPrinfo(struct task_struct *task, struct prinfo *prin)
{
    prin->parent_pid = task->parent->pid;   
    //definition of task_struct is in /include/linux/sched.h 
    prin->pid = task->pid;
    prin->state = task->state;
    prin->uid = task->cred->uid;    
    //task->cred points to the subjective context that defines the 
    //details of how that task is going to act upon another object.
    //definition of cred is in /include/linux/cred.h 
    get_task_comm(prin->comm, task);

    //children and sibling of task_struct are list_head, see /include/linux/list.h 
    prin->first_child_pid = (list_empty(&(task->children)))? 0:list_entry((&task->children)->next,struct task_struct,sibling)->pid;
    
    if (list_empty(&(task->sibling)))
        prin->next_sibling_pid = 0;
    else
    {
        pid_t sibling_pid = list_entry(&task->sibling,struct task_struct,sibling)->pid;
        prin->next_sibling_pid = (sibling_pid == prin->parent_pid)? 0:sibling_pid;
    }
    
    //if there isn't a sibling, sibling.next point to parent or it's empty
    //children is list head, but sibling is list entry
}

void dfs(struct task_struct *task, struct prinfo *buf, int *nr)
{
    struct task_struct *tmp;
    struct list_head *lst;

    transTaskToPrinfo(task,&buf[*nr]);
    *nr = *nr + 1;

    /*#define list_for_each(pos, head) 
	for (pos = (head)->next; pos != (head); pos = pos->next)*/
    list_for_each(lst, &task->children) 
    {
		tmp = list_entry(lst, struct task_struct, sibling);
		dfs(tmp, buf, nr);
	}
}


static int (*oldcall)(void);
int sys_pstree(struct prinfo *buf, int *nr)
{
    struct prinfo *buf_t;
    int *nr_t;
    buf_t = kmalloc_array(1000, sizeof(*buf), GFP_KERNEL);  //suppose processes less than 200
    nr_t = kmalloc(sizeof(int), GFP_KERNEL);

    if (buf_t == NULL || nr_t == NULL) {
        printk("Allocation initialize failed!\n");
        return -EFAULT;
    }

	*nr_t = 0;

    //dfs
    read_lock(&tasklist_lock);  //avoid sleep
	    dfs(&init_task, buf_t, nr_t);
    read_unlock(&tasklist_lock);

	// copy to user
	if (copy_to_user(buf, buf_t, 1000 * sizeof(*buf_t))) {
		printk("Copy_to_user failed!\n");
		return -EFAULT;
	}
	if (copy_to_user(nr, nr_t, sizeof(int))) {
		printk("Copy_to_user failed!\n");
		return -EFAULT;
	}

	kfree(buf_t);  
    kfree(nr_t);

    return *nr;
}

static int addsyscall_init(void)
{
    long *syscall = (long*)0xc000d8c4;
    oldcall = (int(*)(void))(syscall[__NR_pstreecall]);
    syscall[__NR_pstreecall] = (unsigned long)sys_pstree;
    printk(KERN_INFO "module load!\n");
    return 0;
}

static void addsyscall_exit(void)
{
    long *syscall = (long*)0xc000d8c4;
    syscall[__NR_pstreecall] = (unsigned long)oldcall;
    printk(KERN_INFO "module exit!\n");
}

module_init(addsyscall_init);
module_exit(addsyscall_exit);
