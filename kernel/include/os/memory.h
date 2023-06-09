#ifndef _H_MEMORY_P
#define _H_MEMORY_P

#include <os/types.h>
#include <os/assert.h>
#include <os/bitmap.h>

extern "C" {
    void memory_init(u32 magic, u32 addr);
    void memory_map_init();
}

#define PAGE_SIZE 0x1000     // 一页的大小 4K
#define MEMORY_BASE 0x100000 // 1M，可用内存开始的位置

#define ASSERT_PAGE(addr) assert((addr & 0xfff) == 0)

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

class VirtualMemory {
    private:
        page_entry_t* pde;          // 页目录

    public:
        VirtualMemory();
        ~VirtualMemory();

        /* 获取 cr3 寄存器 */
        u32  get_cr3() {
            // 直接将 mov eax, cr3，返回值在 eax 中
            asm volatile("movl %cr3, %eax\n");
        }

        /* 设置 cr3 寄存器 */
        void set_cr3(u32 pde) {
            ASSERT_PAGE(pde);
            asm volatile("movl %%eax, %%cr3\n" ::"a"(pde));
        }

        /* 启用分页 */
        static void _inline enable_page() {
            // 0b1000_0000_0000_0000_0000_0000_0000_0000
            // 0x80000000
            asm volatile(
                "movl %cr0, %eax\n"
                "orl $0x80000000, %eax\n"
                "movl %eax, %cr0\n");
        }

        // 刷新虚拟地址 vaddr 的 块表 TLB
        void _inline flush_tlb(u32 vaddr)
        {
            asm volatile("invlpg (%0)" ::"r"(vaddr)
                        : "memory");
        }

};

class KernelVirtualMemory : public VirtualMemory {
    private:
        page_entry_t* pde;
        Bitmap kernel_map;

    public:
        KernelVirtualMemory();
        ~KernelVirtualMemory();

        void entry_init(page_entry_t *entry, u32 index);

        /* 分配 count 个连续的内存页 */
        u32 alloc_kpage(u32 count);

        /* 释放 count 个连续的内核页 */
        void free_kpage(u32 vaddr, u32 count);

        /* 获取 pde */
        page_entry_t* get_pde();

        /* 获取 kernel_map */
        Bitmap* get_kernel_map();

    protected:
        /* 从位图中扫描 count 个连续的页 */
        u32 scan_page(u32 count);
        
        /* 在位图中释放从 addr 开始的连续 count 个页 */
        void reset_page(u32 addr, u32 count);
};

#endif