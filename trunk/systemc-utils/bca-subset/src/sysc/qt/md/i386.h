/*
 * QuickThreads -- Threads-building toolkit.
 * Copyright (c) 1993 by David Keppel
 *
 * Permission to use, copy, modify and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice and this notice
 * appear in all copies.  This software is provided as a
 * proof-of-concept and for demonstration purposes; there is no
 * representation about the suitability of this software for any
 * purpose.
 */

#ifndef QUICKTHREADS_386_H
#define QUICKTHREADS_386_H

typedef unsigned long qt_word_t;

/* Thread's initial stack layout on the i386:

   non-varargs:

   +---
   | arg[2]	=== `userf' on startup
   | arg[1]	=== `pt' on startup
   | arg[0]	=== `pu' on startup
   +---
   | ret pc === qt_error
   +---
   | ret pc	=== `only' on startup
   +---
   | %ebp
   | %esi
   | %edi
   | %ebx				<--- qt_t.sp
   +---

   When a non-varargs thread is started, it ``returns'' directly to
   the client's `only' function. */


/* Hold 4 saved regs plus two return pcs (qt_error, qt_start) plus
   three args. */
#define QUICKTHREADS_STKBASE	(9 * 4)

/* Stack must be 4-byte aligned. */
#define QUICKTHREADS_STKALIGN	(4)


/* Where to place various arguments. */
#define QUICKTHREADS_ONLY_INDEX	(QUICKTHREADS_PC)
#define QUICKTHREADS_USER_INDEX	(QUICKTHREADS_ARG2)
#define QUICKTHREADS_ARGT_INDEX	(QUICKTHREADS_ARG1)
#define QUICKTHREADS_ARGU_INDEX	(QUICKTHREADS_ARG0)

#define QUICKTHREADS_EBX	0
#define QUICKTHREADS_EDI	1
#define QUICKTHREADS_ESI	2
#define QUICKTHREADS_EBP	3
#define QUICKTHREADS_PC	4
/* The following are defined only for non-varargs. */
#define QUICKTHREADS_RPC	5
#define QUICKTHREADS_ARG0	6
#define QUICKTHREADS_ARG1	7
#define QUICKTHREADS_ARG2	8


/* Stack grows down.  The top of the stack is the first thing to
   pop off (preincrement, postdecrement). */
#define QUICKTHREADS_GROW_DOWN

extern void qt_error (void);

/* Push on the error return address. */
#define QUICKTHREADS_ARGS_MD(sto) \
  (QUICKTHREADS_SPUT (sto, QUICKTHREADS_RPC, qt_error))


#endif /* QUICKTHREADS_386_H */
