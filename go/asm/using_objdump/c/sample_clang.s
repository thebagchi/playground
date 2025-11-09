	.text
	.intel_syntax noprefix
	.file	"sample.c"
	.globl	AddInt64                        # -- Begin function AddInt64
	.p2align	4, 0x90
	.type	AddInt64,@function
AddInt64:                               # @AddInt64
# %bb.0:
	push	rbp
	mov	rbp, rsp
	and	rsp, -8
	lea	rax, [rdi + rsi]
	mov	rsp, rbp
	pop	rbp
	ret
.Lfunc_end0:
	.size	AddInt64, .Lfunc_end0-AddInt64
                                        # -- End function
	.globl	AddConstInt64                   # -- Begin function AddConstInt64
	.p2align	4, 0x90
	.type	AddConstInt64,@function
AddConstInt64:                          # @AddConstInt64
# %bb.0:
	push	rbp
	mov	rbp, rsp
	and	rsp, -8
	lea	rax, [rdi + 42]
	mov	rsp, rbp
	pop	rbp
	ret
.Lfunc_end1:
	.size	AddConstInt64, .Lfunc_end1-AddConstInt64
                                        # -- End function
	.section	".note.GNU-stack","",@progbits
	.addrsig
