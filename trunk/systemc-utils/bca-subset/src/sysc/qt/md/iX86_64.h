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

#ifndef QUICKTHREADS_X86_64_H
#define QUICKTHREADS_X86_64_H

typedef unsigned long qt_word_t;

/* Thread's initial stack layout on the iX86_64:

   non-varargs:

   +---
   | ret pc	=== qt_error
   +---
   | ret pc	=== qt_start
   +---
   | %rbp
   | %rbx
   | %r12	=== `only' on startup
   | %r13	=== `pu' on startup
   | %r14	=== `pt' on startup
   | %r15	=== `userf' on startup      <--- qt_t.sp
   +---

   When a non-varargs thread is started, it ``returns'' directly to
   the client's `only' function. */


/* Hold two return pcs (qt_error, qt_start) plus thirteen args. */
#define QUICKTHREADS_STKBASE	(8 * sizeof(long))

/* Stack must be 16-byte aligned. */
#define QUICKTHREADS_STKALIGN	16


/* Where to place various arguments. */
#define QUICKTHREADS_ONLY_INDEX	(QUICKTHREADS_R12)
#define QUICKTHREADS_USER_INDEX	(QUICKTHREADS_R15)
#define QUICKTHREADS_ARGT_INDEX	(QUICKTHREADS_R14)
#define QUICKTHREADS_ARGU_INDEX	(QUICKTHREADS_R13)

/* Stack layout offsets relative to stack when save stack function called. */

#define QUICKTHREADS_RPC    7
#define QUICKTHREADS_PC	    6
#define QUICKTHREADS_RBP    5
#define QUICKTHREADS_RBX    4
#define QUICKTHREADS_R12    3
#define QUICKTHREADS_R13    2
#define QUICKTHREADS_R14    1
#define QUICKTHREADS_R15    0

/* Stack grows down.  The top of the stack is the first thing to
   pop off (preincrement, postdecrement). */
#define QUICKTHREADS_GROW_DOWN

extern void qt_start(void);

/* Push on the error return address. */
#define QUICKTHREADS_ARGS_MD(sto) \
  (QUICKTHREADS_SPUT (sto, QUICKTHREADS_RPC, qt_error), \
   QUICKTHREADS_SPUT (sto, QUICKTHREADS_PC, qt_start))


#endif /* QUICKTHREADS_X86_64_H */
