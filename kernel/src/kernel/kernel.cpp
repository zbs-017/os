#include <os/os.h>
#include <os/memory.h>

extern "C" {
    void interrupt_init();
    void clock_init();
    void time_init();
    void rtc_init();
    void memory_map_init();
    void hang();
}

#include <os/debug.h>
#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

void test_kernel_virtual_memory_bitmap(KernelVirtualMemory& kvm) {
    u32* pages = (u32*)0x200000;  // 分配到的页的存放位置
    u32 count = 0x6fe;
    for (size_t i = 0; i < count; i++) {
        pages[i] = kvm.alloc_kpage(1);
        LOGK("0x%x\n", i);
    }

    for (size_t i = 0; i < count; i++){
        kvm.free_kpage(pages[i], 1);
    }
}

extern "C" void kernel_init() {

    // 初始化物理内存页数组
    memory_map_init();

    // 初始化虚拟内存映射
    KernelVirtualMemory kernel_virtual_memory = KernelVirtualMemory();

    // 初始化中断描述符表
    interrupt_init();

    // 初始化时钟中断
    clock_init();

    // 初始化时间
    // time_init();

    // 初始化实时时钟
    // rtc_init();

    test_kernel_virtual_memory_bitmap(kernel_virtual_memory);

    // asm volatile("sti");
    hang();
}
