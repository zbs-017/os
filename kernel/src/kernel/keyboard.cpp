#include <os/assert.h>
#include <os/debug.h>
#include <os/interrupt.h>
#include <os/io.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_CTRL_PORT 0x64

void keyboard_handler(int vector) {
  assert(vector == 0x21);
  send_eoi(vector);                       // 发送中断处理完成信号
  u16 scancode = inb(KEYBOARD_DATA_PORT); // 从键盘读取按键信息扫描码
  LOGK("keyboard input 0x%x\n", scancode);
}

extern "C" void keyboard_init() {
  set_interrupt_handler(IRQ_KEYBOARD, (void *)keyboard_handler);
  set_interrupt_mask(IRQ_KEYBOARD, true);
}