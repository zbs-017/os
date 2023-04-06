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

    return;
}
