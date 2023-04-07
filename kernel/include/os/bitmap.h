#ifndef _H_BITMAP
#define _H_BITMAP

#include <os/types.h>

class Bitmap {
    private:
        u8* bits;
        u32 length;
        u32 offset;

    public:
        Bitmap();
        ~Bitmap();

        void init(u8* bits, u32 length, u32 offset);
        bool test(u32 index);
        void set(u32 index, bool value);
        int scan(u32 count);
};


#endif