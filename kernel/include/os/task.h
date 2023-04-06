#ifndef _H_TASK
#define _H_TASK

#include <os/types.h>
#include <os/log.h>

typedef u32 target_t();

typedef struct task_t
{
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


#endif