[bits 32]

extern kernel_main

global _start
_start:
    mov byte [0xb8000], 'K'
    xchg bx, bx
    call kernel_main
    jmp $