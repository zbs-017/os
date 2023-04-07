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

// 页表项
typedef struct page_entry_t {
    u8 present : 1;  // 在内存中
    u8 write : 1;    // 0 只读 1 可读可写
    u8 user : 1;     // 1 所有人 0 超级用户 DPL < 3
    u8 pwt : 1;      // page write through 1 直写模式，0 回写模式
    u8 pcd : 1;      // page cache disable 禁止该页缓冲
    u8 accessed : 1; // 被访问过，用于统计使用频率
    u8 dirty : 1;    // 脏页，表示该页缓冲被写过
    u8 pat : 1;      // page attribute table 页大小 4K/4M
    u8 global : 1;   // 全局，所有进程都用到了，该页不刷新缓冲
    u8 ignored : 3;  // 该安排的都安排了，送给操作系统吧
    u32 index : 20;  // 页索引
} _packed page_entry_t;


#define KERNEL_PAGE_DIR 0x1000       // 内核页目录

class VirtualMemory {
    public:
        static page_entry_t* pde;                   // 内核页目录
        static u32 kernel_page_table[];   // 内核页表

        VirtualMemory();
        ~VirtualMemory();

        static void mapping_init();
        static void entry_init(page_entry_t* entry, u32 index);
        static page_entry_t* get_pde();
        static page_entry_t* get_pte(u32 vaddr);
        static void flush_tlb(u32 vaddr);
    
    private:
        static void set_cr3(u32 pde);
        static u32 get_cr3();
        static void enable_page();
};

#endif