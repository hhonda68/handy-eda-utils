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


/* Hold two return pcs (qt_error, qt_start) plus thirteen args. */
#define QUICKTHREADS_STKBASE	(15 * sizeof(long))

/* Stack must be long-word aligned. */
#define QUICKTHREADS_STKALIGN	(sizeof(long))


/* Where to place various arguments. */
#define QUICKTHREADS_ONLY_INDEX	(QUICKTHREADS_PC)
#define QUICKTHREADS_USER_INDEX	(QUICKTHREADS_ARG2)
#define QUICKTHREADS_ARGT_INDEX	(QUICKTHREADS_ARG1)
#define QUICKTHREADS_ARGU_INDEX	(QUICKTHREADS_ARG0)

/* Stack layout offsets relative to stack when save stack function called. */

#define QUICKTHREADS_PC	   14
#define QUICKTHREADS_RPC   13

#define QUICKTHREADS_R8    12
#define QUICKTHREADS_R9    11
#define QUICKTHREADS_R10   10
#define QUICKTHREADS_R11    9
#define QUICKTHREADS_R12    8
#define QUICKTHREADS_R13    7
#define QUICKTHREADS_R14    6
#define QUICKTHREADS_R15    5
#define QUICKTHREADS_RBX    4
#define QUICKTHREADS_RCX    3
#define QUICKTHREADS_RDX    2
#define QUICKTHREADS_RDI    1
#define QUICKTHREADS_RSI    0


/* Arguments to save stack function. */

#define QUICKTHREADS_ARG0 QUICKTHREADS_RDI
#define QUICKTHREADS_ARG1 QUICKTHREADS_RSI
#define QUICKTHREADS_ARG2 QUICKTHREADS_RDX


/* Stack grows down.  The top of the stack is the first thing to
   pop off (preincrement, postdecrement). */
#define QUICKTHREADS_GROW_DOWN

extern void qt_error (void);

/* Push on the error return address. */
#define QUICKTHREADS_ARGS_MD(sto) \
  (QUICKTHREADS_SPUT (sto, QUICKTHREADS_RPC, qt_error))


#endif /* QUICKTHREADS_X86_64_H */
