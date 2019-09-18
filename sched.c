#include "sched.h"

int nr_tasks = 1;

static struct task_struct init_task = INIT_TASK;
struct task_struct * current = &init_task;
struct task_struct * tasks[NR_TASKS] = {&init_task, };

void disable_preempt(void)
{
    current->preempt_count ++;
}

void enable_preempt(void)
{
    current->preempt_count --;
}

void _sched()
{
    disable_preempt();

    struct task_struct *p;
    int next, ticks;
    for (;;) {
        ticks = -1; next = 0;

        // seek the task owner of the longest remaining ticks
        for (int i = 0; i < NR_TASKS; ++i) {
            p = tasks[i];
            if (p && TASK_RUNNING == p->state && p->left_ticks > ticks) {
                ticks = p->left_ticks;
                next = i;
            }
        }

        if (ticks)
            break;

        for (int i = 0; i < NR_TASKS; ++i) {
            p = tasks[i];
            if (p)
                p->left_ticks = (p->left_ticks >> 1) + p->priority;
        }
    }

    switch_to(tasks[next], next);
/*
    printf("_sched : 0x%x\n", tasks[next]);
*/
    enable_preempt();
}

void schedule(void)
{
	current->left_ticks = 0;
	_sched();
}


void switch_to(struct task_struct * next, int index) 
{
	if (current == next) 
		return;
	struct task_struct * prev = current;
	current = next;
	switch_task(prev, next);
}


void schedule_tail()
{
    enable_preempt();
}

void exit_process()
{
    disable_preempt();
    
    for (int i = 0; i < NR_TASKS; ++i) {
        if (current == tasks[i])
            tasks[i]->state = TASK_ZOMBIE;
    }
    
    if (current->stack) {
        free_page(current->stack);
    }
    
    enable_preempt();
    
    schedule();
}

void timer_tick()
{
    -- current->left_ticks;
    if (current->left_ticks > 0 || current->preempt_count > 0)
        return;
        
    current->left_ticks = 0;
    
    enable_irq();
    _sched();
    disable_irq();
}


