#ifndef _H_TASK
#define _H_TASK

#include <os/types.h>
#include <os/log.h>
#include <os/memory.h>
#include <os/list.h>

typedef u32 target_t();

#define KERNEL_USER 0
#define NORMAL_USER 1

#define TASK_NAME_LEN 16
#define NR_TASKS 64

typedef struct task_frame
{
    u32 edi;
    u32 esi;
    u32 ebx;
    u32 ebp;
    u32 (*eip)(void);
} task_frame;

typedef enum task_state
{
    TASK_INIT,     // 初始化
    TASK_RUNNING,  // 执行
    TASK_READY,    // 就绪
    TASK_BLOCKED,  // 阻塞
    TASK_SLEEPING, // 睡眠
    TASK_WAITING,  // 等待
    TASK_DIED,     // 死亡
} task_state;

typedef struct task {
    u32 *stack;               // 栈顶指针（运行时需要）
    ListNode node;            // 任务阻塞链表节点
    task_state state;         // 任务状态
    u32 priority;             // 任务优先级
    u32 ticks;                // 剩余时间片
    u32 jiffies;              // 上次执行时全局时间片
    char name[TASK_NAME_LEN]; // 任务名
    u32 uid;                  // 用户 id
    page_entry_t* pde;        // 页目录物理地址
    Bitmap* vmap;             // 进程虚拟内存位图
    u32 magic;                // 内核魔数，用于检测栈溢出
} task;

class TaskManager {
    public:
        static task* task_table[NR_TASKS];  // 任务表
        static List block_list;             // 任务阻塞链表

        TaskManager();
        ~TaskManager();

        static void init(KernelVirtualMemory& kvm);

        static task* create(KernelVirtualMemory& kvm, target_t target, const char *name, u32 priority, u32 uid);
        static task* get_free_task(KernelVirtualMemory& kvm);
        static task* running_task();
        static void schedule();
        static task* task_search(task_state state);
        static void task_block(task *t, task_state state);
        static void task_unblock(task *t);

};


#endif