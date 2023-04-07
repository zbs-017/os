#include <os/os.h>
#include <os/memory.h>
#include <os/task.h>

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

u32 _ofp thread_a()
{
    set_interrupt_state(true);

    Log log = Log();
    while (true)
    {
        log.printk("A");
    }
}

u32 _ofp thread_b()
{
    set_interrupt_state(true);

    Log log = Log();
    while (true)
    {
        log.printk("B");
    }
}

u32 _ofp thread_c()
{
    set_interrupt_state(true);

    Log log = Log();
    while (true)
    {
        log.printk("C");
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

    // 初始化任务
    TaskManager::init(kernel_virtual_memory);
    TaskManager::create(kernel_virtual_memory, thread_a, "a", 5, KERNEL_USER);
    TaskManager::create(kernel_virtual_memory, thread_b, "b", 5, KERNEL_USER);
    TaskManager::create(kernel_virtual_memory, thread_c, "c", 5, KERNEL_USER);

    // 打开中断
    set_interrupt_state(true);

    hang();
}
