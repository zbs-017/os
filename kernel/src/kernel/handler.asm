[bits 32]

section .text
; 中断处理函数
global interrupt_handler
interrupt_handler:
    ; 处理中断
    xchg bx, bx
    iret
