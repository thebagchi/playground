	.file	"sample.c"
	.intel_syntax noprefix
	.text
	.p2align 4
	.globl	AddInt8
	.type	AddInt8, @function
AddInt8:
	endbr64
	lea	eax, [rsi+rdi]
	ret
	.size	AddInt8, .-AddInt8
	.p2align 4
	.globl	AddConstInt8
	.type	AddConstInt8, @function
AddConstInt8:
	endbr64
	lea	eax, [rdi+42]
	ret
	.size	AddConstInt8, .-AddConstInt8
	.p2align 4
	.globl	AddUint8
	.type	AddUint8, @function
AddUint8:
	endbr64
	lea	eax, [rsi+rdi]
	ret
	.size	AddUint8, .-AddUint8
	.p2align 4
	.globl	AddConstUint8
	.type	AddConstUint8, @function
AddConstUint8:
	endbr64
	lea	eax, [rdi+42]
	ret
	.size	AddConstUint8, .-AddConstUint8
	.p2align 4
	.globl	AddInt16
	.type	AddInt16, @function
AddInt16:
	endbr64
	lea	eax, [rsi+rdi]
	ret
	.size	AddInt16, .-AddInt16
	.p2align 4
	.globl	AddConstInt16
	.type	AddConstInt16, @function
AddConstInt16:
	endbr64
	lea	eax, [rdi+42]
	ret
	.size	AddConstInt16, .-AddConstInt16
	.p2align 4
	.globl	AddUint16
	.type	AddUint16, @function
AddUint16:
	endbr64
	lea	eax, [rsi+rdi]
	ret
	.size	AddUint16, .-AddUint16
	.p2align 4
	.globl	AddConstUint16
	.type	AddConstUint16, @function
AddConstUint16:
	endbr64
	lea	eax, [rdi+42]
	ret
	.size	AddConstUint16, .-AddConstUint16
	.p2align 4
	.globl	AddInt32
	.type	AddInt32, @function
AddInt32:
	endbr64
	lea	eax, [rdi+rsi]
	ret
	.size	AddInt32, .-AddInt32
	.p2align 4
	.globl	AddConstInt32
	.type	AddConstInt32, @function
AddConstInt32:
	endbr64
	lea	eax, [rdi+42]
	ret
	.size	AddConstInt32, .-AddConstInt32
	.p2align 4
	.globl	AddUint32
	.type	AddUint32, @function
AddUint32:
	endbr64
	lea	eax, [rdi+rsi]
	ret
	.size	AddUint32, .-AddUint32
	.p2align 4
	.globl	AddConstUint32
	.type	AddConstUint32, @function
AddConstUint32:
	endbr64
	lea	eax, [rdi+42]
	ret
	.size	AddConstUint32, .-AddConstUint32
	.p2align 4
	.globl	AddInt64
	.type	AddInt64, @function
AddInt64:
	endbr64
	lea	rax, [rdi+rsi]
	ret
	.size	AddInt64, .-AddInt64
	.p2align 4
	.globl	AddConstInt64
	.type	AddConstInt64, @function
AddConstInt64:
	endbr64
	lea	rax, [rdi+42]
	ret
	.size	AddConstInt64, .-AddConstInt64
	.p2align 4
	.globl	AddUint64
	.type	AddUint64, @function
AddUint64:
	endbr64
	lea	rax, [rdi+rsi]
	ret
	.size	AddUint64, .-AddUint64
	.p2align 4
	.globl	AddConstUint64
	.type	AddConstUint64, @function
AddConstUint64:
	endbr64
	lea	rax, [rdi+42]
	ret
	.size	AddConstUint64, .-AddConstUint64
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
