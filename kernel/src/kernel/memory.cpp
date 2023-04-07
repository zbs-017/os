#include <os/memory.h>
#include <os/os.h>
#include <os/debug.h>
#include <os/assert.h>
#include <os/stdlib.h>
#include <os/string.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define ZONE_VALID 1    // ards 可用内存区域
#define ZONE_RESERVED 2 // ards 不可用区域

#define IDX(addr) ((u32)addr >> 12)             // 获取 addr 的页索引
#define DIDX(addr) (((u32)addr >> 22) & 0x3ff)  // 获取 addr 的页目录索引
#define TIDX(addr) (((u32)addr >> 12) & 0x3ff)  // 获取 addr 的页表索引
#define PAGE(idx) ((u32)idx << 12)              // 获取页索引 idx 对应的页开始的位置
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

page_entry_t* VirtualMemory::pde = nullptr;
u32 VirtualMemory::kernel_page_table[] = {
    0x2000,
    0x3000,
};

#define KERNEL_MEMORY_SIZE (0x100000 * sizeof(VirtualMemory::kernel_page_table))

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

    // 判断物理内存是否够内核使用
    if (PhysicalMemory::memory_size < KERNEL_MEMORY_SIZE)
    {
        panic("System memory is %dM too small, at least %dM needed\n",
              PhysicalMemory::memory_size / MEMORY_BASE, KERNEL_MEMORY_SIZE / MEMORY_BASE);
    }
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

/* 获取 cr3 寄存器 */
u32 get_cr3() {
    // 直接将 mov eax, cr3，返回值在 eax 中
    asm volatile("movl %cr3, %eax\n");
}

/* 设置 cr3 寄存器 */
void set_cr3(u32 pde) {
    ASSERT_PAGE(pde);
    asm volatile("movl %%eax, %%cr3\n" ::"a"(pde));
}

/* 启用分页 */
void enable_page() {
    // 0b1000_0000_0000_0000_0000_0000_0000_0000
    // 0x80000000
    asm volatile(
        "movl %cr0, %eax\n"
        "orl $0x80000000, %eax\n"
        "movl %eax, %cr0\n");
}

/* 初始化页表项 */
void entry_init(page_entry_t *entry, u32 index) {
    *(u32 *)entry = 0;      // 清空页表项 
    entry->present = 1;     // 在内存
    entry->write = 1;       // 可写
    entry->user = 1;        // 所有人都可访问
    entry->index = index;   // 页表索引
}

/* 初始化内存映射 */
extern "C" void mapping_init()
{
    // 内核页目录
    page_entry_t *pde = (page_entry_t *)KERNEL_PAGE_DIR;
    String::memset(pde, 0, PAGE_SIZE);

    idx_t index = 0;

    // 将前 8M 虚拟内存映射到物理内存前 8M
    for (idx_t didx = 0; didx < (sizeof(VirtualMemory::kernel_page_table) / 4); didx++) {
        page_entry_t *pte = (page_entry_t *)VirtualMemory::kernel_page_table[didx];
        String::memset(pte, 0, PAGE_SIZE);

        page_entry_t *dentry = &pde[didx];
        entry_init(dentry, IDX((u32)pte));

        for (idx_t tidx = 0; tidx < 1024; tidx++, index++)
        {
            // 第 0 页不映射，为造成空指针访问，缺页异常，便于排错
            if (index == 0)
                continue;

            page_entry_t *tentry = &pte[tidx];
            entry_init(tentry, index);
            PhysicalMemory::memory_map[index] = 1; // 设置物理内存数组，该页被占用
        }
    }

    // 将最后一个页表指向页目录自己，方便修改
    page_entry_t *entry = &pde[1023];
    entry_init(entry, IDX(KERNEL_PAGE_DIR));

    // 设置 cr3 寄存器
    set_cr3((u32)pde);

    BMB;
    // 分页有效
    enable_page();
}

page_entry_t* get_pde() {
    return (page_entry_t *)(0xfffff000);
}

page_entry_t* get_pte(u32 vaddr)
{
    return (page_entry_t *)(0xffc00000 | (DIDX(vaddr) << 12));
}

// 刷新虚拟地址 vaddr 的 块表 TLB
void flush_tlb(u32 vaddr)
{
    asm volatile("invlpg (%0)" ::"r"(vaddr)
                 : "memory");
}

extern "C" void memory_test()
{
    BMB;

    // 将 20 M 0x1400000 内存映射到 64M 0x4000000 的位置

    // 我们还需要一个页表，0x900000

    u32 vaddr = 0x4000000; // 线性地址几乎可以是任意的
    u32 paddr = 0x1400000; // 物理地址必须要确定存在
    u32 table = 0x900000;  // 页表也必须是物理地址

    page_entry_t *pde = get_pde();

    page_entry_t *dentry = &pde[DIDX(vaddr)];
    entry_init(dentry, IDX(table));

    page_entry_t *pte = get_pte(vaddr);
    page_entry_t *tentry = &pte[TIDX(vaddr)];

    entry_init(tentry, IDX(paddr));

    BMB;

    char *ptr = (char *)(0x4000000);
    ptr[0] = 'a';

    BMB;

    entry_init(tentry, IDX(0x1500000));
    flush_tlb(vaddr);

    BMB;

    ptr[2] = 'b';

    BMB;
}