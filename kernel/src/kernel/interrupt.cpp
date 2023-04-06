#include <os/interrupt.h>
#include <os/global.h>
#include <os/debug.h>

gate_t idt[IDT_SIZE];
pointer_t idt_ptr;

// 中断处理函数
extern "C" void interrupt_handler();

// 初始化中断向量表
void interrupt_init() {
    // 循环，依次配置每一个中断向量描述符
    for (size_t i = 0; i < IDT_SIZE; i++) {
        gate_t* gate = &idt[i];
        gate->offset0 = (u32)interrupt_handler & 0xffff;           // 中断处理函数所在内存地址的低 16 位
        gate->offset1 = ((u32)interrupt_handler >> 16) & 0xffff;   // 中断处理函数所在内存地址的高 16 位
        gate->selector = 1 << 3; // 代码段
        gate->reserved = 0;      // 保留不用
        gate->type = 0b1110;     // 中断门
        gate->segment = 0;       // 系统段
        gate->DPL = 0;           // 内核态
        gate->present = 1;       // 有效
    }  // 为 256 个中断描述符设置默认中断处理函数完毕
    
    // 设置 idt 指针
    idt_ptr.base = (u32)idt;
    idt_ptr.limit = sizeof(idt) - 1;

    // 加载 idt
    BMB;
    asm volatile("lidt idt_ptr\n");
}