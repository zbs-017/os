#include <os/stdlib.h>

extern "C" void delay(u32 count)
{
    while (count--)
        ;
}

extern "C" void hang()
{
    while (true)
        ;
}