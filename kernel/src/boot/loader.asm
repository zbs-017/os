[org 0x1000]

dw 0x55aa  ; loader 标识符

mov si, loadingMsg
call print

; 内核加载器重要功能1：内存检测
detect_memory:
    ; 将 ebx 置为 0
    xor ebx, ebx

    ; es:di 结构体的缓存位置
    mov ax, 0
    mov es, ax
    mov edi, ards_buffer

    mov edx, 0x534d4150; 固定签名

.next:
    ; 子功能号
    mov eax, 0xe820
    ; ards 结构的大小 (字节)
    mov ecx, 20
    ; 调用 0x15 系统调用
    int 0x15

    ; 如果 CF 置位，表示出错
    jc error

    ; 将缓存指针指向下一个结构体
    add di, cx

    ; 将结构体数量加一
    inc dword [ards_count]

    cmp ebx, 0
    jnz .next

    mov si, detectingMsg
    call print

    xchg bx, bx

    mov cx, [ards_count]
    mov si, 0
    ; 查看内存信息
    .show:
        mov eax, [si + ards_buffer]       ; 内存基地址
        mov ebx, [si + ards_buffer + 8]   ; 内存长度
        mov edx, [si + ards_buffer + 16]  ; 内存类型
        add si, 20
        xchg bx, bx
        loop .show

jmp $

error:
    mov si, .msg
    call print
    hlt
    jmp $
    .msg db "Loading ERROR!", 13, 10, 0

print:
    mov ah, 0x0e
    .next:
        mov al, [si]
        cmp al, 0
        jz .down
        int 0x10
        inc si
        jmp .next
    .down:
        ret 

loadingMsg:
    db "Loading ...", 13, 10, 0  ; /n/r
detectingMsg:
    db "Detecting Memory Success ...", 13, 10, 0  ; /n/r

ards_count:
    dw 0
ards_buffer:
