#include <os/memory.h>
#include <os/os.h>
#include <os/debug.h>
#include <os/assert.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define ZONE_VALID 1    // ards 可用内存区域
#define ZONE_RESERVED 2 // ards 不可用区域

#define IDX(addr) ((u32)addr >> 12) // 获取 addr 的页索引

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