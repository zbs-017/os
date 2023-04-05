#include <os/os.h>
#include <os/console.h>
#include <os/global.h>
#include <os/debug.h>

extern "C" void kernel_init() {
    Console console = Console();
    console.clear();

    BMB;
    // 将全局描述符表保存在内核中
    gdt_init();

    return;
}
