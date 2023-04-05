[org 0x7c00]

; 设置屏幕为文本模式，清除屏幕
mov ax, 3
int 0x10

; 初始化段寄存器
mov ax, 0
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7c00

mov ax, 0xb800
mov ds, ax
mov byte [0], 'H'

; 阻塞
jmp $

; 填充到 512 个字节
times 510 - ($ - $$) db 0

; 主引导扇区魔术字
db 0x55, 0xaa