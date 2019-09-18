#include "sched.h"
#include "mm.h"

#include "ok.h"

extern void ret_from_fork(void);

struct pt_regs {
    u64 regs[31];
    u64 sp, pc;
};

struct pt_regs *task_pt_regs(struct task_struct *task)
{
    u64 p = task + TLS_SIZE - sizeof(struct pt_regs);
    return p;
}

int clone_thread(u64 clone_flags, u64 func, u64 args, u64 stack)
{
    disable_preempt();
    
    struct task_struct *p = (struct task_struct *) get_free_page();
    if (!p)
        return -1;

    struct pt_regs *child_regs = task_pt_regs(p);
    memset(child_regs, 0, sizeof(struct pt_regs));
    memset(&p->cpu_context, 0, sizeof(struct cpu_context));

    if (clone_flags & PF_KTHREAD) {
        p->cpu_context.x19 = func;
        p->cpu_context.x20 = args;
    } else {
        struct pt_regs *cur_regs = task_pt_regs(current);
        *cur_regs = *child_regs;
        child_regs->regs[0] = 0;
        child_regs->sp = stack + PAGE_SIZE;
        p->stack = stack;
    }
    p->flags = clone_flags;
    p->left_ticks = p->priority = current->priority;
    p->state = TASK_RUNNING;
    p->preempt_count = 1; // disable preemtion until schedule_tail

    p->cpu_context.sp = child_regs;
    p->cpu_context.pc = ret_from_fork;

    int pid = nr_tasks++;
    tasks[pid] = p;

    enable_preempt();
    return pid;
}

int goto_user(u64 pc)
{
    struct pt_regs *regs = task_pt_regs(current);
    
    memset(regs, 0, sizeof(*regs));
    
    u64 stack = get_free_page(); // allocate new user stack
    if (!stack)
        return -1;
    
    regs->sp = stack + PAGE_SIZE;
    regs->pc = pc;
    
    current->stack = stack;
    return 0;
}

