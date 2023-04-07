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
#include <os/interrupt.h>
#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

/* 涉及到临界区操作的时候，需要先关闭中断，然后执行完以后再开启中断 */
void intr_test() {
    bool intr = interrupt_disable();

    // do something ...

    set_interrupt_state(intr);
}

extern "C" void kernel_init() {

    // 初始化物理内存页数组
    memory_map_init();

    // 初始化虚拟内存映射
    KernelVirtualMemory kernel_virtual_memory = KernelVirtualMemory();

    // 初始化中断描述符表
    interrupt_init();

    // 初始化时钟中断
    // clock_init();

    // 初始化时间
    // time_init();

    // 初始化实时时钟
    // rtc_init();

    BMB;

    bool intr = interrupt_disable();
    set_interrupt_state(true);

    LOGK("%d\n", intr);
    LOGK("%d\n", get_interrupt_state());

    BMB;

    intr = interrupt_disable();

    BMB;
    set_interrupt_state(true);

    LOGK("%d\n", intr);
    LOGK("%d\n", get_interrupt_state());


    hang();
}
