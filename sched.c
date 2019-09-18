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


void _schedule(void)
{
	disable_preempt();
	int next,c;
	struct task_struct * p;
	while (1) {
		c = -1;
		next = 0;
		for (int i = 0; i < NR_TASKS; i++){
			p = tasks[i];
			if (p && p->state == TASK_RUNNING && p->counter > c) {
				c = p->counter;
				next = i;
			}
		}
		if (c) {
			break;
		}
		for (int i = 0; i < NR_TASKS; i++) {
			p = tasks[i];
			if (p) {
				p->counter = (p->counter >> 1) + p->priority;
			}
		}
	}
	switch_to(tasks[next], next);

/*
    uart_puts("\n_schedule : ");
    uart_puthex(task[next]);
*/

	enable_preempt();
}

void schedule(void)
{
	current->counter = 0;
	_schedule();
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
    current->counter--;
    if (current->counter > 0 || current->preempt_count > 0)
        return;
        
    current->counter = 0;
    
    enable_irq();
    _schedule();
    disable_irq();
}


