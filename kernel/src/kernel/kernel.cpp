#include <os/os.h>
#include <os/io.h>

#define CRT_ADDR 0x3d4
#define CRT_DATA 0x3d5
#define CRT_CURS_H 0xe
#define CRT_CURS_L 0xf

extern "C" void kernel_init() {
    /* 测试 io 输入和输出 */
    outb(CRT_ADDR, CRT_CURS_H);
    outb(CRT_DATA, 0);
    outb(CRT_ADDR, CRT_CURS_L);
    outb(CRT_DATA, 240);

    int pos = 0;
    outb(CRT_ADDR, CRT_CURS_L);
    pos = inb(CRT_DATA);
    outb(CRT_ADDR, CRT_CURS_H);
    pos |= inb(CRT_DATA) << 8;
}