#include <os/os.h>
#include <os/debug.h>

extern "C" {
    void interrupt_init();
    void clock_init();
    void time_init();
    void rtc_init();
    void memory_map_init();
    void virtual_memory_map_init();
    void bitmap_test();
    void hang();
}

extern "C" void kernel_init() {

    // 初始化物理内存页数组
    memory_map_init();

    // 初始化虚拟内存映射
    virtual_memory_map_init();

    // 初始化中断描述符表
    interrupt_init();

    // 初始化时钟中断
    clock_init();

    // 初始化时间
    // time_init();

    // 初始化实时时钟
    // rtc_init();

    bitmap_test();

    // asm volatile("sti");
    hang();
}
