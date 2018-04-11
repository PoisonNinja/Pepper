.global syscall
syscall:
    push %rbp
    mov %rsp, %rbp
    push %rcx
    push %r11
    mov %rdi, %rax
    mov %rsi, %rdi
    mov %rdx, %rsi
    mov %rcx, %rdx
    mov %r8, %rcx
    mov %r9, %r8
    syscall
    pop %r11
    pop %rcx
    pop %rbp
    ret

.global legacy_syscall
legacy_syscall:
    push %rbp
    mov %rsp, %rbp
    mov %rdi, %rax
    mov %rsi, %rdi
    mov %rdx, %rsi
    mov %rcx, %rdx
    mov %r8, %rcx
    mov %r9, %r8
    int $0x80
    pop %rbp
    ret