#include <os/os.h>
#include <os/rtc.h>

extern "C" {
    void interrupt_init();
    void clock_init();
    void time_init();
    void rtc_init();
    void memory_map_init();
    void memory_test();
    void hang();
}

extern "C" void kernel_init() {

    // 初始化物理内存页数组
    memory_map_init();

    // 初始化中断描述符表
    interrupt_init();

    // 初始化时钟中断
    clock_init();

    // 初始化时间
    time_init();

    // 初始化实时时钟
    rtc_init();

    // 测试 获取物理内存页
    memory_test();

    asm volatile("sti");
    hang();
}
