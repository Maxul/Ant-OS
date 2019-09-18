#pragma once

#include "ok.h"

#define PF_KTHREAD		0x00200000	/* I am a kernel thread */

#define TLS_SIZE (4096)
#define NR_TASKS (16)

enum {
    TASK_RUNNING,
    TASK_ZOMBIE,
};

struct cpu_context {
    u64 x19, x20, x21, x22, x23, x24, x25, x26, x27, x28;
    u64 fp, sp, pc;
};

struct task_struct {
    struct cpu_context cpu_context;
    long state;
    long counter, priority;
    long preempt_count;
    u64 stack;
    u64 flags;
};

extern int nr_tasks;
extern struct task_struct *tasks[NR_TASKS], *current;

#define INIT_TASK { \
    /* cpu_context */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, \
    /* state etc. */  0, 0, 1, 0, 0, PF_KTHREAD \
}

