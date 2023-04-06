[bits 32]

extern kernel_init
extern console_init

global _start
_start:
    push ebx  ; ards 数量
    push eax  ; 内核魔数

    call console_init

    call kernel_init
    
    jmp $