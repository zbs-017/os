#include <os/os.h>
#include <os/console.h>
#include <os/string.h>

#define CRT_ADDR 0x3d4
#define CRT_DATA 0x3d5
#define CRT_CURS_H 0xe
#define CRT_CURS_L 0xf

extern "C" void kernel_init() {
    /* 测试 控制台函数 */
    char msg[] = "Write by console!";
    Console console = Console();
    console.clear();
    console.write(msg, String::strlen(msg));
}