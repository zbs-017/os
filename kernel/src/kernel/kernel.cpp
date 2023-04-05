#include <os/os.h>
#include <os/console.h>
#include <os/log.h>
#include <os/debug.h>

extern "C" void kernel_init() {
    Console console = Console();
    console.clear();

    /* 测试 debugk */
    BMB;

    DEBUGK("debug os!!!\n");
}
