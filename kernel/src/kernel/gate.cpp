#include <os/interrupt.h>
#include <os/assert.h>
#include <os/debug.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define SYSCALL_SIZE 64

extern "C" {

    handler_t syscall_table[SYSCALL_SIZE];

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

        syscall_table[0] = (void*)sys_test;
    }
}