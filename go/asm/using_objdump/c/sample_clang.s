	.text
	.intel_syntax noprefix
	.file	"sample.c"
	.globl	AddInt8                         # -- Begin function AddInt8
	.p2align	4, 0x90
	.type	AddInt8,@function
AddInt8:                                # @AddInt8
# %bb.0:
	push	rbp
	mov	rbp, rsp
	and	rsp, -8
                                        # kill: def $esi killed $esi def $rsi
                                        # kill: def $edi killed $edi def $rdi
	lea	eax, [rsi + rdi]
                                        # kill: def $al killed $al killed $eax
	mov	rsp, rbp
	pop	rbp
	ret
.Lfunc_end0:
	.size	AddInt8, .Lfunc_end0-AddInt8
                                        # -- End function
	.globl	AddConstInt8                    # -- Begin function AddConstInt8
	.p2align	4, 0x90
	.type	AddConstInt8,@function
AddConstInt8:                           # @AddConstInt8
# %bb.0:
	push	rbp
	mov	rbp, rsp
	and	rsp, -8
                                        # kill: def $edi killed $edi def $rdi
	lea	eax, [rdi + 42]
                                        # kill: def $al killed $al killed $eax
	mov	rsp, rbp
	pop	rbp
	ret
.Lfunc_end1:
	.size	AddConstInt8, .Lfunc_end1-AddConstInt8
                                        # -- End function
	.globl	AddUint8                        # -- Begin function AddUint8
	.p2align	4, 0x90
	.type	AddUint8,@function
AddUint8:                               # @AddUint8
# %bb.0:
	push	rbp
	mov	rbp, rsp
	and	rsp, -8
                                        # kill: def $esi killed $esi def $rsi
                                        # kill: def $edi killed $edi def $rdi
	lea	eax, [rsi + rdi]
                                        # kill: def $al killed $al killed $eax
	mov	rsp, rbp
	pop	rbp
	ret
.Lfunc_end2:
	.size	AddUint8, .Lfunc_end2-AddUint8
                                        # -- End function
	.globl	AddConstUint8                   # -- Begin function AddConstUint8
	.p2align	4, 0x90
	.type	AddConstUint8,@function
AddConstUint8:                          # @AddConstUint8
# %bb.0:
	push	rbp
	mov	rbp, rsp
	and	rsp, -8
                                        # kill: def $edi killed $edi def $rdi
	lea	eax, [rdi + 42]
                                        # kill: def $al killed $al killed $eax
	mov	rsp, rbp
	pop	rbp
	ret
.Lfunc_end3:
	.size	AddConstUint8, .Lfunc_end3-AddConstUint8
                                        # -- End function
	.globl	AddInt16                        # -- Begin function AddInt16
	.p2align	4, 0x90
	.type	AddInt16,@function
AddInt16:                               # @AddInt16
# %bb.0:
	push	rbp
	mov	rbp, rsp
	and	rsp, -8
                                        # kill: def $esi killed $esi def $rsi
                                        # kill: def $edi killed $edi def $rdi
	lea	eax, [rdi + rsi]
                                        # kill: def $ax killed $ax killed $eax
	mov	rsp, rbp
	pop	rbp
	ret
.Lfunc_end4:
	.size	AddInt16, .Lfunc_end4-AddInt16
                                        # -- End function
	.globl	AddConstInt16                   # -- Begin function AddConstInt16
	.p2align	4, 0x90
	.type	AddConstInt16,@function
AddConstInt16:                          # @AddConstInt16
# %bb.0:
	push	rbp
	mov	rbp, rsp
	and	rsp, -8
                                        # kill: def $edi killed $edi def $rdi
	lea	eax, [rdi + 42]
                                        # kill: def $ax killed $ax killed $eax
	mov	rsp, rbp
	pop	rbp
	ret
.Lfunc_end5:
	.size	AddConstInt16, .Lfunc_end5-AddConstInt16
                                        # -- End function
	.globl	AddUint16                       # -- Begin function AddUint16
	.p2align	4, 0x90
	.type	AddUint16,@function
AddUint16:                              # @AddUint16
# %bb.0:
	push	rbp
	mov	rbp, rsp
	and	rsp, -8
                                        # kill: def $esi killed $esi def $rsi
                                        # kill: def $edi killed $edi def $rdi
	lea	eax, [rdi + rsi]
                                        # kill: def $ax killed $ax killed $eax
	mov	rsp, rbp
	pop	rbp
	ret
.Lfunc_end6:
	.size	AddUint16, .Lfunc_end6-AddUint16
                                        # -- End function
	.globl	AddConstUint16                  # -- Begin function AddConstUint16
	.p2align	4, 0x90
	.type	AddConstUint16,@function
AddConstUint16:                         # @AddConstUint16
# %bb.0:
	push	rbp
	mov	rbp, rsp
	and	rsp, -8
                                        # kill: def $edi killed $edi def $rdi
	lea	eax, [rdi + 42]
                                        # kill: def $ax killed $ax killed $eax
	mov	rsp, rbp
	pop	rbp
	ret
.Lfunc_end7:
	.size	AddConstUint16, .Lfunc_end7-AddConstUint16
                                        # -- End function
	.globl	AddInt32                        # -- Begin function AddInt32
	.p2align	4, 0x90
	.type	AddInt32,@function
AddInt32:                               # @AddInt32
# %bb.0:
	push	rbp
	mov	rbp, rsp
	and	rsp, -8
                                        # kill: def $esi killed $esi def $rsi
                                        # kill: def $edi killed $edi def $rdi
	lea	eax, [rdi + rsi]
	mov	rsp, rbp
	pop	rbp
	ret
.Lfunc_end8:
	.size	AddInt32, .Lfunc_end8-AddInt32
                                        # -- End function
	.globl	AddConstInt32                   # -- Begin function AddConstInt32
	.p2align	4, 0x90
	.type	AddConstInt32,@function
AddConstInt32:                          # @AddConstInt32
# %bb.0:
	push	rbp
	mov	rbp, rsp
	and	rsp, -8
                                        # kill: def $edi killed $edi def $rdi
	lea	eax, [rdi + 42]
	mov	rsp, rbp
	pop	rbp
	ret
.Lfunc_end9:
	.size	AddConstInt32, .Lfunc_end9-AddConstInt32
                                        # -- End function
	.globl	AddUint32                       # -- Begin function AddUint32
	.p2align	4, 0x90
	.type	AddUint32,@function
AddUint32:                              # @AddUint32
# %bb.0:
	push	rbp
	mov	rbp, rsp
	and	rsp, -8
                                        # kill: def $esi killed $esi def $rsi
                                        # kill: def $edi killed $edi def $rdi
	lea	eax, [rdi + rsi]
	mov	rsp, rbp
	pop	rbp
	ret
.Lfunc_end10:
	.size	AddUint32, .Lfunc_end10-AddUint32
                                        # -- End function
	.globl	AddConstUint32                  # -- Begin function AddConstUint32
	.p2align	4, 0x90
	.type	AddConstUint32,@function
AddConstUint32:                         # @AddConstUint32
# %bb.0:
	push	rbp
	mov	rbp, rsp
	and	rsp, -8
                                        # kill: def $edi killed $edi def $rdi
	lea	eax, [rdi + 42]
	mov	rsp, rbp
	pop	rbp
	ret
.Lfunc_end11:
	.size	AddConstUint32, .Lfunc_end11-AddConstUint32
                                        # -- End function
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
.Lfunc_end12:
	.size	AddInt64, .Lfunc_end12-AddInt64
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
.Lfunc_end13:
	.size	AddConstInt64, .Lfunc_end13-AddConstInt64
                                        # -- End function
	.globl	AddUint64                       # -- Begin function AddUint64
	.p2align	4, 0x90
	.type	AddUint64,@function
AddUint64:                              # @AddUint64
# %bb.0:
	push	rbp
	mov	rbp, rsp
	and	rsp, -8
	lea	rax, [rdi + rsi]
	mov	rsp, rbp
	pop	rbp
	ret
.Lfunc_end14:
	.size	AddUint64, .Lfunc_end14-AddUint64
                                        # -- End function
	.globl	AddConstUint64                  # -- Begin function AddConstUint64
	.p2align	4, 0x90
	.type	AddConstUint64,@function
AddConstUint64:                         # @AddConstUint64
# %bb.0:
	push	rbp
	mov	rbp, rsp
	and	rsp, -8
	lea	rax, [rdi + 42]
	mov	rsp, rbp
	pop	rbp
	ret
.Lfunc_end15:
	.size	AddConstUint64, .Lfunc_end15-AddConstUint64
                                        # -- End function
	.section	".note.GNU-stack","",@progbits
	.addrsig
