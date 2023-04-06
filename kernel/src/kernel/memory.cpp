#include <os/memory.h>
#include <os/os.h>
#include <os/debug.h>
#include <os/assert.h>
#include <os/stdlib.h>
#include <os/string.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define ZONE_VALID 1    // ards 可用内存区域
#define ZONE_RESERVED 2 // ards 不可用区域

#define IDX(addr) ((u32)addr >> 12) // 获取 addr 的页索引
#define PAGE(idx) ((u32)idx << 12)  // 获取页索引 idx 对应的页开始的位置
#define ASSERT_PAGE(addr) assert((addr & 0xfff) == 0)


typedef struct ards_t
{
    u64 base; // 内存基地址
    u64 size; // 内存长度
    u32 type; // 类型
} _packed ards_t;

u32 PhysicalMemory::memory_base = 0;
u32 PhysicalMemory::memory_size = 0;
u32 PhysicalMemory::total_pages = 0;
u32 PhysicalMemory::free_pages = 0;
u32 PhysicalMemory::start_page = 0;
u8* PhysicalMemory::memory_map = nullptr; 
u32 PhysicalMemory::memory_map_pages = 0;

/* 从 loader 中获取物理内存检测结果 */
extern "C" void memory_init(u32 magic, u32 addr) {
    u32 count;
    ards_t *ptr;

    // 如果是 onix loader 进入的内核
    if (magic == OS_MAGIC)
    {
        count = *(u32 *)addr;
        ptr = (ards_t *)(addr + 4);
        for (size_t i = 0; i < count; i++, ptr++)
        {
            LOGK("Memory base 0x%p size 0x%p type %d\n",
                 (u32)ptr->base, (u32)ptr->size, (u32)ptr->type);
            if (ptr->type == ZONE_VALID && ptr->size > PhysicalMemory::memory_size)
            {
                PhysicalMemory::memory_base = (u32)ptr->base;
                PhysicalMemory::memory_size = (u32)ptr->size;
            }
        }
    }
    else
    {
        panic("Memory init magic unknown 0x%p\n", magic);
    }

    LOGK("ARDS count %d\n", count);
    LOGK("Memory base 0x%p\n", (u32)PhysicalMemory::memory_base);
    LOGK("Memory size 0x%p\n", (u32)PhysicalMemory::memory_size);

    assert(PhysicalMemory::memory_base == MEMORY_BASE); // 内存开始的位置为 1M
    assert((PhysicalMemory::memory_size & 0xfff) == 0); // 要求按页对齐

    PhysicalMemory::total_pages = IDX(PhysicalMemory::memory_size) + IDX(MEMORY_BASE);
    PhysicalMemory::free_pages = IDX(PhysicalMemory::memory_size);

    LOGK("Total pages %d\n", PhysicalMemory::total_pages);
    LOGK("Free pages %d\n", PhysicalMemory::free_pages);
}

/* 初始化物理内存页数组 */
extern "C" void memory_map_init() {
    // 初始化物理内存数组
    PhysicalMemory::memory_map = (u8 *)PhysicalMemory::memory_base;

    // 计算物理内存数组占用的页数
    PhysicalMemory::memory_map_pages = div_round_up(PhysicalMemory::total_pages, PAGE_SIZE);
    LOGK("Memory map page count %d\n", PhysicalMemory::memory_map_pages);

    PhysicalMemory::free_pages -= PhysicalMemory::memory_map_pages;

    // 清空物理内存数组
    String::memset((void *)PhysicalMemory::memory_map, 0, PhysicalMemory::memory_map_pages * PAGE_SIZE);

    // 前 1M 的内存位置 以及 物理内存数组已占用的页，已被占用
    PhysicalMemory::start_page = IDX(MEMORY_BASE) + PhysicalMemory::memory_map_pages;
    for (size_t i = 0; i < PhysicalMemory::start_page; i++)
    {
        PhysicalMemory::memory_map[i] = 1;
    }

    LOGK("Total pages %d free pages %d\n", PhysicalMemory::total_pages, PhysicalMemory::free_pages);
}

/* 分配一页物理内存 */
u32 PhysicalMemory::get_page() {
    for (size_t i = start_page; i < total_pages; i++)
    {
        // 如果物理内存没有占用
        if (!memory_map[i])
        {
            memory_map[i] = 1;
            free_pages--;
            assert(free_pages >= 0);
            u32 page = ((u32)i) << 12;
            LOGK("GET page 0x%p\n", page);
            return page;
        }
    }
    panic("Out of Memory!!!");
}

/* 释放一页物理内存 */
void PhysicalMemory::put_page(u32 addr) {
    ASSERT_PAGE(addr);

    u32 idx = IDX(addr);

    // idx 大于 1M 并且 小于 总页面数
    assert(idx >= start_page && idx < total_pages);

    // 保证只有一个引用
    assert(memory_map[idx] >= 1);

    // 物理引用减一
    memory_map[idx]--;

    // 若为 0，则空闲页加一
    if (!memory_map[idx])
    {
        free_pages++;
    }

    assert(free_pages > 0 && free_pages < total_pages);
    LOGK("PUT page 0x%p\n", addr);
}

extern "C" void memory_test() {
    u32 pages[10];
    for (size_t i = 0; i < 10; i++) {
        pages[i] = PhysicalMemory::get_page();
    }

    for (size_t i = 0; i < 10; i++) {
        PhysicalMemory::put_page(pages[i]);
    }
    DEBUGK("memory_test success!\n");
}