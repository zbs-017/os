#include <os/os.h>
#include <os/console.h>
#include <os/global.h>
#include <os/debug.h>
#include <os/interrupt.h>
#include <os/task.h>
#include <os/stdlib.h>

extern "C" void kernel_init() {
    Console console = Console();
    console.clear();

    // 将全局描述符表保存在内核中
    gdt_init();

    // 初始化中断描述符表
    interrupt_init();

    Task::init();

    // 打开中断，允许 CPU 响应中断
    asm volatile("sti");

    /* 主程序运行的时候，一直会收到来自外部的中断信号
     * 这个中断信号是 时钟 送来的。 
     */
    u32 counter = 0;
    while (true)
    {
        DEBUGK("looping in kernel init %d...\n", counter++);
        delay(100000000);
    }

    return;
}
