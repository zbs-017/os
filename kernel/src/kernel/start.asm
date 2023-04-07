[bits 32]

extern kernel_init
extern console_init
extern gdt_init
extern memory_init

global _start
_start:
    push ebx  ; ards 数量
    push eax  ; 内核魔数

    call console_init
    call gdt_init
    call memory_init
    call kernel_init

    xchg bx, bx

    mov eax, 0
    int 0x80
    
    jmp $