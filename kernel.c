#include <stddef.h>
#include <stdint.h>

#include "sched.h"

static inline void io_halt(void)
{
    asm volatile("wfi");
}

void kernel_thread_A(void)
{
    uint64_t a = 0;
    for (;;) {
        printf("Kernel Task A: Hello, world: %lu\n", a++);
        delay(1000000);
    }
}

void kernel_thread_B(void)
{
    uint64_t b = 0;
    for (;;) {
        printf("Kernel Task B: Hello, world: %lu\n", b++);
        delay(1000000);
    }
}

void user_thread_A(void)
{
    uint64_t a = 0;
    for (;;) {
        printf("User Task A: Hello, world: %lu\n", a++);
        delay(1000000);
    }
}

void user_thread_B(void)
{
    uint64_t b = 0;
    for (;;) {
        printf("User Task B: Hello, world: %lu\n", b++);
        delay(1000000);
    }
}

void user_thread()
{
    char buf[30] = "User thread started\n";
    
    call_sys_write(buf);
    
    unsigned long stack;
    int err;
    
    stack = call_sys_malloc();
    if (stack < 0) {
        printf("Error while allocating stack for thread 1\n\r");
        return;
    }
    err = call_sys_clone((unsigned long)&user_thread_A, 0, stack);
    if (err < 0) {
        printf("Error while clonning thread 1\n\r");
        return;
    }
    stack = call_sys_malloc();
    if (stack < 0) {
        printf("Error while allocating stack for thread 2\n\r");
        return;
    }
    err = call_sys_clone((unsigned long)&user_thread_B, 0, stack);
    if (err < 0) {
        printf("Error while clonning thread 2\n\r");
        return;
    }
    call_sys_exit();

}

void kernel_thread()
{
	printf("Kernel thread started. EL %d\n", get_el());
	
	int err = goto_user((unsigned long)&user_thread);
	if (err < 0){
		printf("Error while moving thread to user mode\n\r");
	}
	printf("Good moving thread to user mode\n\r");
}

void kernel_main(void)
{
    printf("Main: Hello, world!\n");
    
    interrupt_init();
    
    enable_irq();

    int res;

    res = clone_thread(PF_KTHREAD, (unsigned long)&kernel_thread, 0, 0);
	if (res < 0) {
		printf("error while starting kernel thread");
		return;
	}

    res = clone_thread(PF_KTHREAD, (unsigned long)&kernel_thread_A, 0, 0);
    if (res < 0) {
        printf("error while starting thread 1");
        return;
    }
    res = clone_thread(PF_KTHREAD, (unsigned long)&kernel_thread_B, 0, 0);
    if (res < 0) {
        printf("error while starting thread 2");
        return;
    }

    while (1) {
        io_halt();
    }

}
