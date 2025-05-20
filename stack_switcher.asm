global switcher           ; экспортируем символ для линковщика
section .text
;в rdi - функцию, котору нужно вызвать
;в rsi - адрес стека
;в rdx - указатель на параметры на функцию
;возврат rax - адрес текущего положения на стеке
switcher:
    jmp save_reg
    save_reg_exit:
    mov rax, rsp
    mov rsp, rsi
    push rax
    mov rax, rdi
    mov rdi, rdx
    call rax
    mov rax, rsp
    pop rsp
    jmp load_reg
    load_reg_exit:
    ret              

save_reg:
    ;push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    pushfq
    jmp save_reg_exit

load_reg:
    popfq
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    ;pop rax
    jmp load_reg_exit