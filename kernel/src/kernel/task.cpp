#include <os/task.h>
#include <os/log.h>
#include <os/string.h>
#include <os/interrupt.h>
#include <os/assert.h>
#include <os/list.h>

#define PAGE_SIZE 0x1000

extern "C" void switch_task(task* next);

extern "C" void task_yield() {
    TaskManager::schedule();
}

task* TaskManager::task_table[NR_TASKS];
list_t TaskManager::block_list;
task* TaskManager::idle_task = nullptr;

TaskManager::TaskManager() { }
TaskManager::~TaskManager() { }

void TaskManager::init(KernelVirtualMemory& kvm) {

    for (size_t i = 0; i < NR_TASKS; i++) {
        task_table[i] = nullptr;
    }

    list_init(&TaskManager::block_list);

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

    // 如果当前所有任务都阻塞，就执行空闲任务
    if (t == nullptr && state == TASK_READY) {
        t = TaskManager::idle_task;
    }

    return t;
}

void TaskManager::schedule() {
    assert(!get_interrupt_state());

    task* current = running_task();
    task* next = task_search(TASK_READY);

    assert(next != nullptr);
    assert(next->magic == OS_MAGIC);

    if (current->state == TASK_RUNNING) {
        current->state = TASK_READY;
    }

    if (!current->ticks) {
        current->ticks = current->priority;
    }

    next->state = TASK_RUNNING;
    if (next == current) return;

    switch_task(next);
}

void TaskManager::task_block(task* t, list_t* blist, task_state state) {
    assert(!get_interrupt_state());
    assert(t->node.prev == nullptr);
    assert(t->node.prev == nullptr);

    if (blist == nullptr) {
        blist = &TaskManager::block_list;
    }

    list_push(blist, &t->node);  // 加入阻塞队列

    assert(state != TASK_READY && state != TASK_RUNNING);

    t->state = state;

    task* current = TaskManager::running_task();
    if (current == t) {
        TaskManager::schedule();
    }
}

void TaskManager::task_unblock(task* t) {
    assert(!get_interrupt_state());

    list_remove(&t->node);

    assert(t->node.next == nullptr);
    assert(t->node.prev == nullptr);

    t->state = TASK_READY;
}