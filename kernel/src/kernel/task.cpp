#include <os/task.h>
#include <os/log.h>

#define PAGE_SIZE 0x1000

task_t *a = (task_t *)0x1000;
task_t *b = (task_t *)0x2000;

extern "C" void task_switch(task_t *next);
extern "C" void switch_task(Task* next);

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

u32 thread_a()
{
    Log log = Log();
    while (true)
    {
        log.printk("A");
        Task::schedule();
    }
}

u32 thread_b()
{
    Log log = Log();
    while (true)
    {
        log.printk("B");
        Task::schedule();
    }
}

static void task_create(task_t *task, target_t target)
{
    u32 stack = (u32)task + PAGE_SIZE;

    stack -= sizeof(task_frame_t);
    task_frame_t *frame = (task_frame_t *)stack;
    frame->ebx = 0x11111111;
    frame->esi = 0x22222222;
    frame->edi = 0x33333333;
    frame->ebp = 0x44444444;
    frame->eip = target;

    task->stack = (u32 *)stack;
}

void task_init()
{
    task_create(a, thread_a);
    task_create(b, thread_b);
    schedule();
}

Task::Task(Task* task, u32 (*target)(void)) {
    // task: 任务创建的地址
    // target: 任务执行的程序
    this->stack_top = (u32*)((u32)task + PAGE_SIZE);  // 获得栈顶
    this->push((void*)target);             // eip
    this->push((void*)0x44444444);  // ebp
    this->push((void*)0x11111111);  // ebx
    this->push((void*)0x22222222);  // esi
    this->push((void*)0x33333333);  // edi
    *(u32*)task = (u32)this->stack_top;
}

Task::~Task() {

}

/* 向栈中压入数据 */
void Task::push(void* d) {
    u32* data = (u32*)d;
    this->stack_top--;
    *this->stack_top = (u32)data;
}

void Task::init() {

    Task a = Task((Task*)0x1000, thread_a);
    Task b = Task((Task*)0x2000, thread_b);
    Task::schedule();
}

void Task::schedule() {
    Task* current = Task::running_task();
    Task* next = current == (Task*)0x1000 ? (Task*)0x2000 : (Task*)0x1000;
    switch_task(next);
}

Task* Task::running_task() {
    asm volatile(
        "movl %esp, %eax\n"
        "andl $0xfffff000, %eax\n");
}