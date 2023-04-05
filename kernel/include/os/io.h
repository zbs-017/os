#ifndef _IO_H
#define _IO_H

#include <os/types.h>

extern "C" u8 inb(u16 port);  // 输入一个字节
extern "C" u16 inw(u16 port); // 输入一个字

extern "C" void outb(u16 port, u8 value);  // 输出一个字节
extern "C" void outw(u16 port, u16 value); // 输出一个字

#endif