#ifndef _H_TASK
#define _H_TASK

#include <os/types.h>
#include <os/log.h>

typedef u32 target_t();

typedef struct task_t
{
    // 任务开始的地方，保存了栈顶指针
    u32 *stack; // 内核栈
} task_t;

typedef struct task_frame_t
{
    u32 edi;
    u32 esi;
    u32 ebx;
    u32 ebp;
    u32 (*eip)(void);
} task_frame_t;

void task_init();

/* 需要虚拟内存的支持，需要把 Task 对象分配到指定内存中 */
class Task {
    private:
        u32* pcb_eip;
        u32* stack_top;

    public:
        Task(Task* task, u32 (*target)(void));
        ~Task();

        void push(void* data);
        static void init();
        static void schedule(Task a, Task b);
        static Task* running_task();
};


#endif