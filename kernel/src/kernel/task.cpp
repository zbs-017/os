#include <os/task.h>
#include <os/log.h>

#define PAGE_SIZE 0x1000

task_t *a = (task_t *)0x1000;
task_t *b = (task_t *)0x2000;

extern "C" void task_switch(task_t *next);

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
        schedule();
    }
}

u32 thread_b()
{
    Log log = Log();
    while (true)
    {
        log.printk("B");
        schedule();
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