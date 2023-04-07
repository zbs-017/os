#include <os/interrupt.h>
#include <os/assert.h>
#include <os/debug.h>
#include <os/syscall.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define SYSCALL_SIZE 64

extern "C" {

    handler_t syscall_table[SYSCALL_SIZE];

    void task_yield();

    void syscall_check(u32 nr)
    {
        if (nr >= SYSCALL_SIZE)
        {
            panic("syscall nr error!!!");
        }
    }

    static void sys_default()
    {
        panic("syscall not implemented!!!");
    }

    static u32 sys_test()
    {
        LOGK("syscall test...\n");
        return 255;
    }

    void syscall_init()
    {
        for (size_t i = 0; i < SYSCALL_SIZE; i++)
        {
            syscall_table[i] = (void*)sys_default;
        }

        syscall_table[SYS_NR_TEST] = (void*)sys_test;
        syscall_table[SYS_NR_YIELD] = (void*)task_yield;
    }
}