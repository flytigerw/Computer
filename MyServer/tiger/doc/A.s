	.file	"A.cpp"
	.intel_syntax noprefix
	.text
	.p2align 4,,15
	.globl	_Z4testv
	.type	_Z4testv, @function
_Z4testv:
.LFB0:
	.cfi_startproc
	mov	eax, DWORD PTR b[rip]
	add	eax, 1
	mov	DWORD PTR b[rip], eax
	mov	eax, DWORD PTR b[rip]
	add	eax, 1
	mov	DWORD PTR a[rip], eax
	ret
	.cfi_endproc
.LFE0:
	.size	_Z4testv, .-_Z4testv
	.globl	b
	.bss
	.align 4
	.type	b, @object
	.size	b, 4
b:
	.zero	4
	.globl	a
	.align 4
	.type	a, @object
	.size	a, 4
a:
	.zero	4
	.ident	"GCC: (GNU) 8.2.0"
	.section	.note.GNU-stack,"",@progbits
