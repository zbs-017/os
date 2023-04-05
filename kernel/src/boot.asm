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

xchg bx, bx

mov si, bootingMsg
call print

; 阻塞
jmp $

print:
    mov ah, 0x0e  ; 0x10 的打印子功能号
    .next:
        mov al, [si]
        cmp al, 0
        jz .down
        int 0x10  ; 调用 BOIS 的打印功能
        inc si
        jmp .next
    .down
        ret 

bootingMsg:
    db "Booting ...", 13, 10, 0  ; /n/r

; 填充到 512 个字节
times 510 - ($ - $$) db 0

; 主引导扇区魔术字
db 0x55, 0xaa