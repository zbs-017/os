[bits 32]

section .text
global task_switch
task_switch:
    push ebp
    mov ebp, esp

    push ebx
    push esi
    push edi

    mov eax, esp;
    and eax, 0xfffff000; current

    mov [eax], esp

    mov eax, [ebp + 8]; next
    mov esp, [eax]

    ; 恢复 next 中的寄存器的值
    pop edi
    pop esi
    pop ebx
    pop ebp

    ; 将 next 中的 eip pop 了出来，所以，返回以后执行的是 next 任务中的程序
    ret


[bits 32]

section .text
global switch_task
switch_task:
    push ebp
    mov ebp, esp

    push ebx
    push esi
    push edi

    mov eax, esp;
    and eax, 0xfffff000; current

    mov [eax], esp

    mov eax, [ebp + 8]; next
    mov esp, [eax]

    ; 恢复 next 中的寄存器的值
    pop edi
    pop esi
    pop ebx
    pop ebp

    ; 将 next 中的 eip pop 了出来，所以，返回以后执行的是 next 任务中的程序
    ret