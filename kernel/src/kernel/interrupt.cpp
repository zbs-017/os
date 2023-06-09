#include <os/interrupt.h>
#include <os/global.h>
#include <os/debug.h>
#include <os/log.h>
#include <os/io.h>
#include <os/stdlib.h>
#include <os/task.h>
#include <os/assert.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)
// #define LOGK(fmt, args...)

#define ENTRY_SIZE 0x30

#define PIC_M_CTRL 0x20 // 主片的控制端口
#define PIC_M_DATA 0x21 // 主片的数据端口
#define PIC_S_CTRL 0xa0 // 从片的控制端口
#define PIC_S_DATA 0xa1 // 从片的数据端口
#define PIC_EOI 0x20    // 通知中断控制器中断结束

gate_t idt[IDT_SIZE];
pointer_t idt_ptr;

// 中断处理函数
// extern "C" void interrupt_handler();
/* 针对与不同的中断，应该由不同的中断处理函数来处理
 * 定义了一个中断处理函数表(handler_entry_table)，里面保存着对应中断的中断处理函数地址
 */
handler_t handler_table[IDT_SIZE];                 // 由高级语言实现的中断处理函数（由默认中断处理函数调用的）
extern handler_t handler_entry_table[ENTRY_SIZE];  // 中断处理函数指针（中断发生时调用的）
extern "C" void syscall_handler();

static char* messages[] = {
    (char*)"#DE Divide Error\0",
    (char*)"#DB RESERVED\0",
    (char*)"--  NMI Interrupt\0",
    (char*)"#BP Breakpoint\0",
    (char*)"#OF Overflow\0",
    (char*)"#BR BOUND Range Exceeded\0",
    (char*)"#UD Invalid Opcode (Undefined Opcode)\0",
    (char*)"#NM Device Not Available (No Math Coprocessor)\0",
    (char*)"#DF Double Fault\0",
    (char*)"    Coprocessor Segment Overrun (reserved)\0",
    (char*)"#TS Invalid TSS\0",
    (char*)"#NP Segment Not Present\0",
    (char*)"#SS Stack-Segment Fault\0",
    (char*)"#GP General Protection\0",
    (char*)"#PF Page Fault\0",
    (char*)"--  (Intel reserved. Do not use.)\0",
    (char*)"#MF x87 FPU Floating-Point Error (Math Fault)\0",
    (char*)"#AC Alignment Check\0",
    (char*)"#MC Machine Check\0",
    (char*)"#XF SIMD Floating-Point Exception\0",
    (char*)"#VE Virtualization Exception\0",
    (char*)"#CP Control Protection Exception\0",
};

// 通知中断控制器，中断处理结束
void send_eoi(int vector)
{
    if (vector >= 0x20 && vector < 0x28)
    {
        outb(PIC_M_CTRL, PIC_EOI);
    }
    if (vector >= 0x28 && vector < 0x30)
    {
        outb(PIC_M_CTRL, PIC_EOI);
        outb(PIC_S_CTRL, PIC_EOI);
    }
}

void set_interrupt_handler(u32 irq, handler_t handler)
{
    assert(irq < 16);
    handler_table[IRQ_MASTER_NR + irq] = handler;
}

void set_interrupt_mask(u32 irq, bool enable)
{
    assert(irq < 16);
    u16 port;
    if (irq < 8)
    {
        port = PIC_M_DATA;
    }
    else
    {
        port = PIC_S_DATA;
        irq -= 8;
    }
    if (enable)
    {
        outb(port, inb(port) & ~(1 << irq));
    }
    else
    {
        outb(port, inb(port) | (1 << irq));
    }
}

// 清除 IF 位，返回设置之前的值
bool interrupt_disable()
{
    asm volatile(
        "pushfl\n"        // 将当前 eflags 压入栈中
        "cli\n"           // 清除 IF 位，此时外中断已被屏蔽
        "popl %eax\n"     // 将刚才压入的 eflags 弹出到 eax
        "shrl $9, %eax\n" // 将 eax 右移 9 位，得到 IF 位
        "andl $1, %eax\n" // 只需要 IF 位
    );
}

// 获得 IF 位
bool get_interrupt_state()
{
    asm volatile(
        "pushfl\n"        // 将当前 eflags 压入栈中
        "popl %eax\n"     // 将压入的 eflags 弹出到 eax
        "shrl $9, %eax\n" // 将 eax 右移 9 位，得到 IF 位
        "andl $1, %eax\n" // 只需要 IF 位
    );
}

// 设置 IF 位
void set_interrupt_state(bool state)
{
    if (state)
        asm volatile("sti\n");
    else
        asm volatile("cli\n");
}

// 由高级语言编写的默认 外中断 处理函数
void ex_handler(int vector) {
    send_eoi(vector);
    DEBUGK("[%x] default interrupt called %d...\n", vector);
}

// 由高级语言编写的默认中断处理函数
void exception_handler(int vector,
    u32 edi, u32 esi, u32 ebp, u32 esp,
    u32 ebx, u32 edx, u32 ecx, u32 eax,
    u32 gs, u32 fs, u32 es, u32 ds,
    u32 vector0, u32 error, u32 eip, u32 cs, u32 eflags)
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

    log.printk("\nEXCEPTION : %s \n", messages[vector]);
    log.printk("   VECTOR : 0x%02X\n", vector);
    log.printk("    ERROR : 0x%08X\n", error);
    log.printk("   EFLAGS : 0x%08X\n", eflags);
    log.printk("       CS : 0x%02X\n", cs);
    log.printk("      EIP : 0x%08X\n", eip);
    log.printk("      ESP : 0x%08X\n", esp);
    // 阻塞
    hang();
}

// 初始化中断控制器
void pic_init()
{
    outb(PIC_M_CTRL, 0b00010001); // ICW1: 边沿触发, 级联 8259, 需要ICW4.
    outb(PIC_M_DATA, 0x20);       // ICW2: 起始端口号 0x20
    outb(PIC_M_DATA, 0b00000100); // ICW3: IR2接从片.
    outb(PIC_M_DATA, 0b00000001); // ICW4: 8086模式, 正常EOI

    outb(PIC_S_CTRL, 0b00010001); // ICW1: 边沿触发, 级联 8259, 需要ICW4.
    outb(PIC_S_DATA, 0x28);       // ICW2: 起始端口号 0x28
    outb(PIC_S_DATA, 2);          // ICW3: 设置从片连接到主片的 IR2 引脚
    outb(PIC_S_DATA, 0b00000001); // ICW4: 8086模式, 正常EOI

    outb(PIC_M_DATA, 0b11111111); // 关闭所有中断
    outb(PIC_S_DATA, 0b11111111); // 关闭所有中断
}


// 初始化中断向量表
void idt_init() {
    // 循环，依次配置每一个中断向量描述符
    for (size_t i = 0; i < ENTRY_SIZE; i++) {
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

    // 初始化外中断处理函数
    for (size_t i = 0x20; i < 0x30; i++) {
        handler_table[i] = (void*)ex_handler;
    }

    // 初始化系统调用
    gate_t *gate = &idt[0x80];
    gate->offset0 = (u32)syscall_handler & 0xffff;
    gate->offset1 = ((u32)syscall_handler >> 16) & 0xffff;
    gate->selector = 1 << 3; // 代码段
    gate->reserved = 0;      // 保留不用
    gate->type = 0b1110;     // 中断门
    gate->segment = 0;       // 系统段
    gate->DPL = 3;           // 用户态
    gate->present = 1;       // 有效
    
    // 设置 idt 指针
    idt_ptr.base = (u32)idt;
    idt_ptr.limit = sizeof(idt) - 1;

    // 加载 idt
    asm volatile("lidt idt_ptr\n");
}

extern "C" void interrupt_init() {
    DEBUGK("init interrupt!!!\n");
    pic_init();
    idt_init();
}