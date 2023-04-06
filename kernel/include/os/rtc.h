#ifndef _H_RTC
#define _H_RTC

#include <os/types.h>

class RTC {
    public:
        RTC();
        ~RTC();

        static u8 cmos_read(u8 addr);
        static void cmos_write(u8 addr, u8 value);
        static void set_alarm(u32 secs);
};

#endif