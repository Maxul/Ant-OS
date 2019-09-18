#include "ok.h"

void sys_write(const char *buf)
{
    uart_puts(buf);
}

int sys_clone(u64 stack)
{
    return clone_thread(0, 0, 0, stack);
}

u64 sys_allocate()
{
    u64 addr = get_free_page();

    if (!addr)
        return -1;
    return addr;
}

void sys_exit()
{
    exit_process();
}

void * const syscall_tbl[] = {
    sys_write,
    sys_allocate,
    sys_clone,
    sys_exit,
};


