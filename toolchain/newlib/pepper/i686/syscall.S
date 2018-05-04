.global syscall
syscall:
    push %ebp
	mov %esp, %ebp
	push %ebx
	push %edi
	push %esi
    mov 8 (%ebp), %eax
	mov 12 (%ebp), %ebx
	mov 16(%ebp), %ecx
	mov 20(%ebp), %edx
	mov 24(%ebp), %edi
	mov 28(%ebp), %esi
	int $0x80
    pop %esi
	pop %edi
	pop %ebx
	pop %ebp
	ret
