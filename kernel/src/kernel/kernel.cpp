#include <os/os.h>
#include <os/console.h>
#include <os/log.h>

extern "C" void kernel_init() {
    Console console = Console();
    console.clear();

    Log log = Log();
    log.printk("Print by printk() function!\n");
    log.printk("Print format string: %d\n", 10);
}
