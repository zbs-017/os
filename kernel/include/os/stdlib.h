#ifndef _H_STDLIB
#define _H_STDLIB

#include <os/types.h>

extern "C" {
    void delay(u32 count);
    void hang();
}

u8 bcd_to_bin(u8 value);
u8 bin_to_bcd(u8 value);

#endif