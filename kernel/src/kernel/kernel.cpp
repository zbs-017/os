#include <os/os.h>
#include <os/memory.h>
#include <os/task.h>
#include <os/syscall.h>

extern "C" {
    void interrupt_init();
    void clock_init();
    void time_init();
    void rtc_init();
    void memory_map_init();
    void syscall_init();
    void hang();
    void init_thread();
    void idle_thread();
    void test_thread();
}

#include <os/debug.h>
#include <os/interrupt.h>
#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

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
    TaskManager::idle_task = TaskManager::create(kernel_virtual_memory, (target_t*)idle_thread, "idle", 1, KERNEL_USER);
    TaskManager::create(kernel_virtual_memory, (target_t*)init_thread, "init", 5, NORMAL_USER);
    TaskManager::create(kernel_virtual_memory, (target_t*)test_thread, "test", 5, NORMAL_USER);

    // 初始化系统调用
    syscall_init();

    // 打开中断
    set_interrupt_state(true);

    // hang();
}
