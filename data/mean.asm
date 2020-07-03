	.text
	.file	"PCL program"
	.globl	main                    # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:                                # %entry
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%rbx
	.cfi_def_cfa_offset 40
	pushq	%rax
	.cfi_def_cfa_offset 48
	.cfi_offset %rbx, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movl	$.L__unnamed_1, %edi
	callq	writeString
	callq	readInteger
	movl	%eax, %r14d
	movl	$.L__unnamed_2, %edi
	callq	writeString
	movl	$65, %ebx
	movl	$221, %ebp
	callq	readInteger
	xorpd	%xmm1, %xmm1
	movl	%eax, %r15d
	.p2align	4, 0x90
.LBB0_1:                                # %loop
                                        # =>This Inner Loop Header: Depth=1
	leal	-221(%rbp), %eax
	cmpl	%r15d, %eax
	jge	.LBB0_3
# %bb.2:                                # %body
                                        #   in Loop: Header=BB0_1 Depth=1
	imull	$137, %ebx, %eax
	addl	%ebp, %eax
	cltd
	idivl	%r14d
	movl	%edx, %ebx
	xorps	%xmm0, %xmm0
	cvtsi2sd	%edx, %xmm0
	addsd	%xmm0, %xmm1
	incl	%ebp
	jmp	.LBB0_1
.LBB0_3:                                # %after
	testl	%r15d, %r15d
	jle	.LBB0_5
# %bb.4:                                # %then
	movl	$.L__unnamed_3, %edi
	movsd	%xmm1, (%rsp)           # 8-byte Spill
	callq	writeString
	xorps	%xmm1, %xmm1
	cvtsi2sd	%r15d, %xmm1
	movsd	(%rsp), %xmm0           # 8-byte Reload
                                        # xmm0 = mem[0],zero
	divsd	%xmm1, %xmm0
	callq	writeReal
	movl	$.L__unnamed_4, %edi
	callq	writeString
.LBB0_5:                                # %after4
	xorl	%eax, %eax
	addq	$8, %rsp
	.cfi_def_cfa_offset 40
	popq	%rbx
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc
                                        # -- End function
	.type	.L__unnamed_1,@object   # @0
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__unnamed_1:
	.asciz	"Give n: "
	.size	.L__unnamed_1, 9

	.type	.L__unnamed_2,@object   # @1
.L__unnamed_2:
	.asciz	"Give k: "
	.size	.L__unnamed_2, 9

	.type	.L__unnamed_3,@object   # @2
.L__unnamed_3:
	.asciz	"Mean: "
	.size	.L__unnamed_3, 7

	.type	.L__unnamed_4,@object   # @3
.L__unnamed_4:
	.asciz	"\n"
	.size	.L__unnamed_4, 2

	.section	".note.GNU-stack","",@progbits
