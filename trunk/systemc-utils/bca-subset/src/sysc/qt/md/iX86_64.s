/* iX386_64.s -- assembly support. */

/*
// QuickThreads -- Threads-building toolkit.
// Copyright (c) 1993 by David Keppel
//
// Permission to use, copy, modify and distribute this software and
// its documentation for any purpose and without fee is hereby
// granted, provided that the above copyright notice and this notice
// appear in all copies.  This software is provided as a
// proof-of-concept and for demonstration purposes; there is no
// representation about the suitability of this software for any
// purpose. */

/* 64-bit Intel Architecture Support
// written by Andy Goodrich, Forte Design Systms, Inc.  */

// NOTE: double-labeled `_name' and `name' for System V compatability.
// NOTE: Comment lines start with '/*' and '//' ONLY.  Sorry!

    .text
    .align 2

    .globl _qt_block
    .globl qt_block

_qt_block:
qt_block:
	                   /*  6 (return address) */
	pushq %rbp         /*  5 ... */
	pushq %rbx         /*  4 ... */
	pushq %r12         /*  3 ... */
	pushq %r13         /*  2 ... */
	pushq %r14         /*  1 ... */
	pushq %r15         /*  0 ... */
	movq %rsp, (%rdi)  /* Save old stack pointer. */
	movq %rsi, %rsp    /* Move to new thread. */
	popq %r15          /*  0 ... */
	popq %r14          /*  1 ... */
	popq %r13          /*  2 ... */
	popq %r12          /*  3 ... */
	popq %rbx          /*  4 ... */
	popq %rbp          /*  5 ... */
	ret

    .globl _qt_start
    .globl qt_start
_qt_start:
qt_start:
	movq %r15, %rdx    /* userf (third argument) */
	movq %r14, %rsi    /* pt    (second argument) */
	movq %r13, %rdi    /* pu    (first argument) */
	jmp  *%r12         /* jump to the "only" startup procedure*/
