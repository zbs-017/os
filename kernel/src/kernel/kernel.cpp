#include <os/os.h>
#include <os/console.h>
#include <os/global.h>
#include <os/debug.h>
#include <os/task.h>

extern "C" void kernel_init() {
    Console console = Console();
    console.clear();

    // 将全局描述符表保存在内核中
    gdt_init();

    task_init();

    return;
}
