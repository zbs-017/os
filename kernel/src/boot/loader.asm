[org 0x1000]

dw 0x55aa  ; loader 标识符

mov si, loadingMsg
call print

jmp $

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
