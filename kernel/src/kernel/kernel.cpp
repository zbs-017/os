#include <os/os.h>
#include <os/console.h>
#include <os/rtc.h>

extern "C" {
    void interrupt_init();
    void gdt_init();
    void clock_init();
    void time_init();
    void rtc_init();
    void hang();
}

extern "C" void kernel_init() {
    Console console = Console();
    console.clear();

    // 将全局描述符表保存在内核中
    gdt_init();

    // 初始化中断描述符表
    interrupt_init();

    // 初始化时钟中断
    clock_init();

    // 初始化时间
    time_init();

    // 初始化实时时钟
    rtc_init();
    RTC::set_alarm(2);

    asm volatile("sti");
    hang();
}
