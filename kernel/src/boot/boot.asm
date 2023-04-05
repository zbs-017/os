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

mov si, bootingMsg
call print

; 将 loader 读入内存
mov ebx, 2      ; 起始扇区位置
mov edi, 0x1000 ; 内存位置
mov cx, 4       ; 扇区数量
.read_loader:
    call read_disk
    inc ebx
    loop .read_loader

cmp word [0x1000], 0x55aa
jnz error

jmp 0:0x1002

; 阻塞
jmp $

error:
    mov si, .msg
    call print
    hlt
    jmp $
    .msg db "Botting ERROR! Can not found Loader!", 13, 10, 0

read_disk:
    ; 参数：
    ;   - ebx: 硬盘的起始扇区
    ;   - edi: 存放到内存中的位置

    push eax
    push ebx
    push edx
    push ecx

    ; 初始化硬盘
    mov dx, 0x1f2
    mov al, 1  ; 一次只能读取一个扇区
    out dx, al

    inc dx; 0x1f3
    mov al, bl
    out dx, al
    shr ebx, 8

    inc dx; 0x1f4
    mov al, bl
    out dx, al
    shr ebx, 8

    inc dx; 0x1f5
    mov al, bl
    out dx, al
    shr ebx, 8

    inc dx; 0x1f6
    mov al, bl
    and al, 0x0f
    or al, 0b1110_0000  ; 主盘，LBA 模式
    out dx, al

    inc dx; 0x1f7
    mov al, 0x20
    out dx, al  ; 读硬盘

    xor ecx, ecx
    mov cx, 256  ; 每次读取一个字（16bit)，读 256 次正好一个扇区
    .wait:
        mov dx, 0x1f7
        in al, dx
        and al, 0b1000_1000  ; 不关心出错的情况
        cmp al, 0b0000_1000  ; 数据准备完毕且硬盘空闲
        jnz .wait
    .read_word:
        ; 数据准备完毕，可以读取
        mov dx, 0x1f0
        in ax, dx
        mov [di], ax
        add di, 2
        loop .read_word

    pop ecx
    pop edx
    pop ebx
    pop eax

    ret

print:
    mov ah, 0x0e  ; 0x10 的打印子功能号
    .next:
        mov al, [si]
        cmp al, 0
        jz .down
        int 0x10  ; 调用 BOIS 的打印功能
        inc si
        jmp .next
    .down:
        ret 

bootingMsg:
    db "Booting ...", 13, 10, 0  ; /n/r

; 填充到 512 个字节
times 510 - ($ - $$) db 0

; 主引导扇区魔术字
db 0x55, 0xaa