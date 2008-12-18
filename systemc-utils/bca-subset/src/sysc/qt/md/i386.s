/* i386.s -- assembly support. */

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

// NOTE: double-labeled `_name' and `name' for System V compatability.
// NOTE: Comment lines start with '/*' and '//' ONLY.  Sorry!

/* Callee-save: %esi, %edi, %ebx, %ebp
// Caller-save: %eax, %ecx
// Can't tell: %edx (seems to work w/o saving it.)
//
// Assignment:
//
// See ``i386.h'' for the somewhat unconventional stack layout.  */


	.text
	.align 2

	.globl _qt_block
	.globl qt_block

/* These all have the type signature
//
//	void *blocking (helper, arg0, arg1, new)
//
// On procedure entry, the helper is at 4(sp), args at 8(sp) and
// 12(sp) and the new thread's sp at 16(sp).  It *appears* that the
// calling convention for the 8X86 requires the caller to save all
// floating-point registers, this makes our life easy.  */

/* Halt the currently-running thread.  Save it's callee-save regs on
// to the stack, 32 bytes.  Switch to the new stack (next == 16+32(sp))
// and call the user function (f == 4+32(sp) with arguments: old sp
// arg1 (8+32(sp)) and arg2 (12+32(sp)).  When the user function is
// done, restore the new thread's state and return.
//
// The helper function (4(sp)) can return a void* that is returned
// by the call to `qt_block'.  Since we don't touch %eax in
// between, we get that ``for free''.  */

_qt_block:
qt_block:
	pushl %ebp		/* Save callee-save, sp-=4. */
	pushl %esi		/* Save callee-save, sp-=4. */
	pushl %edi		/* Save callee-save, sp-=4. */
	pushl %ebx		/* Save callee-save, sp-=4. */
	movl %esp, %eax		/* Remember old stack pointer. */
	movl 20(%eax), %ebx	/* Get address of the context save area. */
	movl 24(%eax), %esp	/* Move to new thread. */
	movl %eax, (%ebx)	/* Save the old context. */
	popl %ebx		/* Restore callee-save, sp+=4. */
	popl %edi		/* Restore callee-save, sp+=4. */
	popl %esi		/* Restore callee-save, sp+=4. */
	popl %ebp		/* Restore callee-save, sp+=4. */
	ret			/* Resume the stopped function. */
	hlt


