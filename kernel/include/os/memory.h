#ifndef _H_MEMORY_P
#define _H_MEMORY_P

#include <os/types.h>

extern "C" {
    void memory_init(u32 magic, u32 addr);
    void memory_map_init();
}

#define PAGE_SIZE 0x1000     // 一页的大小 4K
#define MEMORY_BASE 0x100000 // 1M，可用内存开始的位置



class PhysicalMemory {
    public:
        static u32 memory_base;
        static u32 memory_size;
        static u32 total_pages;
        static u32 free_pages;

        static u32 start_page;       // 可分配物理内存起始位置
        static u8* memory_map;       // 物理内存数组(存放在 1M 内存开始的位置)
        static u32 memory_map_pages; // 物理内存数组占用的页数

        /* 分配一页物理内存 */
        static u32 get_page();

        /* 释放一页物理内存 */
        static void put_page(u32 addr);
};

#endif