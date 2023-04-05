#include <os/os.h>
#include <os/console.h>
#include <os/log.h>
#include <os/assert.h>

extern "C" void kernel_init() {
    Console console = Console();
    console.clear();

    assert(3 > 5);
    panic("Out Of Memory!!");
}
