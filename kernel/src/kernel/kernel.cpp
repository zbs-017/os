#include <os/os.h>

int magic = OS_MAGIC;

extern "C" void kernel_init() {
    char* video = (char*) 0xB8000;
    const char* hello = "Hello Kernel!";

    for (int i = 0; i < 13; i++) {
        video[2 * i] = hello[i];
        video[2 * i + 1] = 0x07;
    }
}