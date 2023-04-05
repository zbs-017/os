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

    jmp prepare_protected_mode

prepare_protected_mode:
    ; 关闭中断
    cli
    ; 打开 A20 线
    in al,  0x92
    or al, 0b10
    out 0x92, al

; 内核加载器重要功能2：加载 GDT
    xchg bx, bx
    lgdt [gdt_ptr]; 加载 gdt

; 内核加载器重要功能2：启动保护模式
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; 用跳转来刷新缓存，启用保护模式
    jmp dword code_selector:protect_mode

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

[bits 32]
protect_mode:
    xchg bx, bx
    
    ; 初始化段寄存器
    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x10000  ; 修改栈顶，内核系统在上，栈在下，互不干扰

    mov byte [0xb8000], 'P'

    mov byte [0x200000], 'P'
    xchg bx, bx

    jmp $

; 段选择子
code_selector equ (1 << 3)
data_selector equ (2 << 3)

; GDT
; 内存开始的位置：基地址
memory_base equ 0
; 内存界限 4G / 4K - 1
memory_limit equ ((1024 * 1024 * 1024 * 4) / (1024 * 4)) - 1
gdt_ptr:
    dw (gdt_end - gdt_base) - 1
    dd gdt_base
gdt_base:
    dd 0, 0; NULL 描述符
gdt_code:  ; 4GB 代码段
    dw memory_limit & 0xffff; 段界限 0 ~ 15 位
    dw memory_base & 0xffff; 基地址 0 ~ 15 位
    db (memory_base >> 16) & 0xff; 基地址 16 ~ 23 位
    ; 存在 - dlp 0 - S _ 代码 - 非依从 - 可读 - 没有被访问过
    db 0b_1_00_1_1_0_1_0;
    ; 4k - 32 位 - 不是 64 位 - 段界限 16 ~ 19
    db 0b1_1_0_0_0000 | (memory_limit >> 16) & 0xf;
    db (memory_base >> 24) & 0xff; 基地址 24 ~ 31 位
gdt_data:  ; 4GB 数据段
    dw memory_limit & 0xffff; 段界限 0 ~ 15 位
    dw memory_base & 0xffff; 基地址 0 ~ 15 位
    db (memory_base >> 16) & 0xff; 基地址 16 ~ 23 位
    ; 存在 - dlp 0 - S _ 数据 - 向上 - 可写 - 没有被访问过
    db 0b_1_00_1_0_0_1_0;
    ; 4k - 32 位 - 不是 64 位 - 段界限 16 ~ 19
    db 0b1_1_0_0_0000 | (memory_limit >> 16) & 0xf;
    db (memory_base >> 24) & 0xff; 基地址 24 ~ 31 位
gdt_end:

ards_count:
    dw 0
ards_buffer:
