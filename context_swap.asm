global context_swap
%define OFF_GREGS     0
%define OFF_RIP       (8*16)
%define OFF_EFLAGS    (8*17)
%define OFF_STACK_PTR (8*15)
section .text
; ---------------------------
; context_swap:
;   вход: rdi = pointer to scontext
; ---------------------------

context_swap:
    ; — восстанавливаем RSP
    mov rsp, [rdi + OFF_STACK_PTR]

    ; восстанавливаем no-volitile reg
    mov r8,  [rdi + OFF_GREGS + 8*0]   ; R8
    mov r9,  [rdi + OFF_GREGS + 8*1]   ; R9
    mov r10, [rdi + OFF_GREGS + 8*2]    ; R10
    mov r11, [rdi + OFF_GREGS + 8*3]    ; R11
    mov r12, [rdi + OFF_GREGS + 8*4]    ; R12
    mov r13, [rdi + OFF_GREGS + 8*5]    ; R13
    mov r14, [rdi + OFF_GREGS + 8*6]    ; R14
    mov r15, [rdi + OFF_GREGS + 8*7]    ; R15

    ;mov rdi, [rdi + OFF_GREGS + 8*8]    ; RDI
    mov rsi, [rdi + OFF_GREGS + 8*9]    ; RSI
    mov rbp, [rdi + OFF_GREGS + 8*10]    ; RBP
    mov rbx, [rdi + OFF_GREGS + 8*11]    ; RBX 
    mov rdx, [rdi + OFF_GREGS + 8*12]    ; RDX
    mov rax, [rdi + OFF_GREGS + 8*13]    ; RAX
    mov rcx, [rdi + OFF_GREGS + 8*14]    ; RCX

    push qword [rdi + OFF_EFLAGS]
    popfq

    mov rax, [rdi + OFF_RIP]
    mov rdi, [rdi + OFF_GREGS + 8*8]    ; RDI
    jmp rax