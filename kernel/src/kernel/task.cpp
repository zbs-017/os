#include <os/task.h>
#include <os/log.h>
#include <os/string.h>
#include <os/interrupt.h>
#include <os/assert.h>

#define PAGE_SIZE 0x1000

task_t *a = (task_t *)0x1000;
task_t *b = (task_t *)0x2000;

extern "C" void task_switch(task_t *next);
extern "C" void switch_task(task* next);

task_t *running_task()
{
    // esp 中存放着当前的栈顶指针
    // 通过栈顶指针的内存位置可以判断当前执行的是哪个程序
    asm volatile(
        "movl %esp, %eax\n"
        "andl $0xfffff000, %eax\n");
}

void schedule()
{
    task_t *current = running_task();
    task_t *next = current == a ? b : a;
    task_switch(next);
}

task* TaskManager::task_table[NR_TASKS];

TaskManager::TaskManager() { }
TaskManager::~TaskManager() { }

void TaskManager::init(KernelVirtualMemory& kvm) {
    task* t = running_task();
    t->magic = OS_MAGIC;
    t->ticks = 1;   // 调度完初始化任务后，就开始调度其他任务

    // 初始化任务表
    String::memset(TaskManager::task_table, 0, sizeof(TaskManager::task_table));
}

task* TaskManager::create(KernelVirtualMemory& kvm, target_t target, const char *name, u32 priority, u32 uid) {
    task* t = get_free_task(kvm);      // 获取一个空闲的任务
    String::memset(t, 0, PAGE_SIZE);   // 初始化任务内存空间

    u32 stack = (u32)t + PAGE_SIZE;    // 获得 task 栈顶

    stack -= sizeof(task_frame);  // 开始初始化栈
    task_frame* frame = (task_frame*) stack;
    frame->ebx = 0x11111111;
    frame->esi = 0x22222222;
    frame->edi = 0x33333333;
    frame->ebp = 0x44444444;
    frame->eip = target;

    String::strcpy((char*)t->name, name);

    t->stack = (u32*) stack;
    t->priority = priority;
    t->ticks = t->priority;
    t->jiffies = 0;
    t->state = TASK_READY;
    t->uid = uid;
    t->pde = kvm.get_pde();
    t->vmap = kvm.get_kernel_map();
    t->magic = OS_MAGIC;

    return t;
}

task* TaskManager::get_free_task(KernelVirtualMemory& kvm) {
    for (size_t i = 0; i < NR_TASKS; i++) {
        if (task_table[i] == nullptr) {
            task_table[i] = (task*)kvm.alloc_kpage(1);
            return task_table[i];
        }
    }

    panic("No more tasks!\n");
}

task* TaskManager::running_task() {
    asm volatile(
        "movl %esp, %eax\n"
        "andl $0xfffff000, %eax\n");
}

task* TaskManager::task_search(task_state state) {
    assert(!get_interrupt_state());
    task* t = nullptr;
    task* current = running_task();

    for (size_t i = 0; i < NR_TASKS; i++) {
        task* ptr = task_table[i];
        if (ptr == nullptr) continue;
        if (ptr->state != state) continue;
        if (ptr == current) continue;
        if (t == nullptr || t->ticks < ptr->ticks || ptr->jiffies < t->jiffies)
            t = ptr;
    }

    return t;
}

void TaskManager::schedule() {
    task* current = running_task();
    task* next = task_search(TASK_READY);

    assert(next != nullptr);
    assert(next->magic == OS_MAGIC);

    if (current->state == TASK_RUNNING) {
        current->state = TASK_READY;
    }

    next->state = TASK_RUNNING;
    if (next == current) return;

    switch_task(next);
}