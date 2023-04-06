#ifndef _H_MEMORY_P
#define _H_MEMORY_P

#include <os/types.h>

extern "C" {
    void memory_init(u32 magic, u32 addr);
}

#define PAGE_SIZE 0x1000     // 一页的大小 4K
#define MEMORY_BASE 0x100000 // 1M，可用内存开始的位置



class PhysicalMemory {
    public:
        static u32 memory_base;
        static u32 memory_size;
        static u32 total_pages;
        static u32 free_pages;

};

#endif