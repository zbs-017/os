#include <os/interrupt.h>
#include <os/global.h>
#include <os/debug.h>
#include <os/log.h>

#define ENTRY_SIZE 0x20

gate_t idt[IDT_SIZE];
pointer_t idt_ptr;

// 中断处理函数
// extern "C" void interrupt_handler();
/* 针对与不同的中断，应该由不同的中断处理函数来处理
 * 定义了一个中断处理函数表(handler_entry_table)，里面保存着对应中断的中断处理函数地址
 */
handler_t handler_table[IDT_SIZE];                 // 由高级语言实现的中断处理函数（由默认中断处理函数调用的）
extern handler_t handler_entry_table[ENTRY_SIZE];  // 中断处理函数指针（中断发生时调用的）

static char* messages[] = {
    "#DE Divide Error\0",
    "#DB RESERVED\0",
    "--  NMI Interrupt\0",
    "#BP Breakpoint\0",
    "#OF Overflow\0",
    "#BR BOUND Range Exceeded\0",
    "#UD Invalid Opcode (Undefined Opcode)\0",
    "#NM Device Not Available (No Math Coprocessor)\0",
    "#DF Double Fault\0",
    "    Coprocessor Segment Overrun (reserved)\0",
    "#TS Invalid TSS\0",
    "#NP Segment Not Present\0",
    "#SS Stack-Segment Fault\0",
    "#GP General Protection\0",
    "#PF Page Fault\0",
    "--  (Intel reserved. Do not use.)\0",
    "#MF x87 FPU Floating-Point Error (Math Fault)\0",
    "#AC Alignment Check\0",
    "#MC Machine Check\0",
    "#XF SIMD Floating-Point Exception\0",
    "#VE Virtualization Exception\0",
    "#CP Control Protection Exception\0",
};

// 由高级语言编写的默认中断处理函数
void exception_handler(int vector)
{
    Log log = Log();
    char *message = nullptr;
    if (vector < 22)
    {
        message = messages[vector];
    }
    else
    {
        message = messages[15];
    }

    log.printk("Exception : [0x%02X] %s \n", vector, messages[vector]);
    // 阻塞
    while (true)
        ;
}


// 初始化中断向量表
void interrupt_init() {
    // 循环，依次配置每一个中断向量描述符
    for (size_t i = 0; i < IDT_SIZE; i++) {
        gate_t* gate = &idt[i];
        handler_t handler = handler_entry_table[i];
        gate->offset0 = (u32)handler & 0xffff;           // 中断处理函数所在内存地址的低 16 位
        gate->offset1 = ((u32)handler >> 16) & 0xffff;   // 中断处理函数所在内存地址的高 16 位
        gate->selector = 1 << 3; // 代码段
        gate->reserved = 0;      // 保留不用
        gate->type = 0b1110;     // 中断门
        gate->segment = 0;       // 系统段
        gate->DPL = 0;           // 内核态
        gate->present = 1;       // 有效
    }  // 为 256 个中断描述符设置默认中断处理函数完毕

    // 无论哪个中断发生，都调用由高级语言实现的 默认 中断处理函数
    for (size_t i = 0; i < 0x20; i++)
    {
        handler_table[i] = (void*)exception_handler;
    }
    
    // 设置 idt 指针
    idt_ptr.base = (u32)idt;
    idt_ptr.limit = sizeof(idt) - 1;

    // 加载 idt
    asm volatile("lidt idt_ptr\n");
}