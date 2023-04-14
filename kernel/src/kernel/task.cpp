#include <os/task.h>
#include <os/log.h>
#include <os/string.h>
#include <os/interrupt.h>
#include <os/assert.h>

#define PAGE_SIZE 0x1000

extern "C" void switch_task(task* next);

extern "C" void task_yield() {
    TaskManager::schedule();
}

extern "C" u32 volatile jiffies;
extern "C" u32 jiffy;

task* TaskManager::task_table[NR_TASKS];
List TaskManager::block_list;  // 这里调用的是默认构造函数
task *TaskManager::idle_task = nullptr;
List TaskManager::sleep_list;

TaskManager::TaskManager() { }
TaskManager::~TaskManager() { }

void TaskManager::init(KernelVirtualMemory& kvm) {
    task* t = running_task();
    t->magic = OS_MAGIC;
    t->ticks = 1;   // 调度完初始化任务后，就开始调度其他任务

    // 初始化任务表
    String::memset(TaskManager::task_table, 0, sizeof(TaskManager::task_table));
    // 初始化睡眠链表
    block_list.head.next = &block_list.tail;
    block_list.tail.prev = &block_list.head;
    // 初始化睡眠链表
    sleep_list.head.next = &sleep_list.tail;
    sleep_list.tail.prev = &sleep_list.head;
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

    // 如果当前任务都阻塞，就执行空闲任务
    if (t == nullptr && state == TASK_READY) {
        t = idle_task;
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

void TaskManager::task_block(task *t, task_state state) {
    assert(!get_interrupt_state());
    assert(t->node.prev == nullptr);
    assert(t->node.prev == nullptr);

    block_list.push(&t->node);  // 加入阻塞队列

    assert(state != TASK_READY && state != TASK_RUNNING);

    t->state = state;

    task *current = running_task();
    if (current == t) {
        schedule();
    }
}

void TaskManager::task_unblock(task *t) {
    assert(!get_interrupt_state());

    block_list.remove(&t->node);

    assert(t->node.next == nullptr);
    assert(t->node.prev == nullptr);

    t->state = TASK_READY;
}

void TaskManager::task_sleep(u32 ms) {
    assert(!get_interrupt_state()); // 不可中断

    u32 ticks = ms / jiffy;        // 需要睡眠的时间片
    ticks = ticks > 0 ? ticks : 1; // 至少休眠一个时间片

    // 记录目标全局时间片，在那个时刻需要唤醒任务
    task* current = running_task();
    current->ticks = jiffies + ticks;

    // 从睡眠链表找到第一个比当前任务唤醒时间点更晚的任务，进行插入排序
    ListNode *anchor = &sleep_list.tail;

    for (ListNode *ptr = sleep_list.head.next; ptr != &sleep_list.tail; ptr = ptr->next)
    {
        task* t = element_entry(task, node, ptr);

        if (t->ticks > current->ticks)
        {
            anchor = ptr;
            break;
        }
    }

    assert(current->node.next == nullptr);
    assert(current->node.prev == nullptr);

    // 插入链表
    sleep_list.insert_before(anchor, &current->node);

    // 阻塞状态是睡眠
    current->state = TASK_SLEEPING;

    // 调度执行其他任务
    schedule();
}

void TaskManager::task_wakeup() {
    assert(!get_interrupt_state()); // 不可中断

    // 从睡眠链表中找到 ticks 小于等于 jiffies 的任务，恢复执行
    for (ListNode *ptr = sleep_list.head.next; ptr != &sleep_list.tail;)
    {
        task* t = element_entry(task, node, ptr);
        if (t->ticks > jiffies)
        {
            break;
        }

        // unblock 会将指针清空
        ptr = ptr->next;

        t->ticks = 0;
        task_unblock(t);
    }
}

