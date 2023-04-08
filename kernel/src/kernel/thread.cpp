#include <os/interrupt.h>
#include <os/syscall.h>
#include <os/debug.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

extern "C" {
    void idle_thread() {
        set_interrupt_state(true);
        u32 counter = 0;
        while (true)
        {
            // LOGK("idle task.... %d\n", counter++);
            asm volatile(
                "sti\n" // 开中断
                "hlt\n" // 关闭 CPU，进入暂停状态，等待外中断的到来
            );
            yield(); // 放弃执行权，调度执行其他任务
        }
    }

    void init_thread() {
        set_interrupt_state(true);

        while (true)
        {
            // LOGK("init task....\n");
            // test();
        }
    }
}
