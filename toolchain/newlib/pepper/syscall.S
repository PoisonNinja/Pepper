.global syscall
syscall:
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
