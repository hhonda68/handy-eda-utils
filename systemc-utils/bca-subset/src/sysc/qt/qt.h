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

#ifndef QUICKTHREADS_QT_H
#define QUICKTHREADS_QT_H

#ifdef __cplusplus
extern "C" {
#endif

extern void qt_error(void);

#include "sysc/qt/qtmd.h"


/* A QuickThreads thread is represented by it's current stack pointer.
   To restart a thread, you merely need pass the current sp (qt_t*) to
   a QuickThreads primitive.  `qt_t*' is a location on the stack.  To
   improve type checking, represent it by a particular struct. */

typedef struct qt_t {
  char dummy;
} qt_t;


/* Alignment is guaranteed to be a power of two. */
#ifndef QUICKTHREADS_STKALIGN
  #error "Need to know the machine-dependent stack alignment."
#endif

#define QUICKTHREADS_STKROUNDUP(bytes) \
  (((bytes)+QUICKTHREADS_STKALIGN) & ~(QUICKTHREADS_STKALIGN-1))


/* Find ``top'' of the stack, space on the stack. */
#ifndef QUICKTHREADS_SP
#ifdef QUICKTHREADS_GROW_DOWN
#define QUICKTHREADS_SP(sto, size)	((qt_t *)(&((char *)(sto))[(size)]))
#endif
#ifdef QUICKTHREADS_GROW_UP
#define QUICKTHREADS_SP(sto, size)	((qt_t *)(sto))
#endif
#if !defined(QUICKTHREADS_SP)
  #error "QUICKTHREADS_QT_H: Stack must grow up or down!"
#endif
#endif


/* The type of the user function:
   For non-varargs, takes one void* function.
   For varargs, takes some number of arguments. */
typedef void *(qt_userf_t)(void *pu);

/* For non-varargs, just call a client-supplied function,
   it does all startup and cleanup, and also calls the user's
   function. */
//typedef void (qt_only_t)(void *pu, void *pt, qt_userf_t *userf);

/* Internal helper for putting stuff on stack. */
#ifndef QUICKTHREADS_SPUT
#define QUICKTHREADS_SPUT(top, at, val)	\
    (((qt_word_t *)(top))[(at)] = (qt_word_t)(val))
#endif


/* Push arguments for the non-varargs case. */
#ifndef QUICKTHREADS_ARGS

#ifndef QUICKTHREADS_ARGS_MD
#define QUICKTHREADS_ARGS_MD (0)
#endif

#ifndef QUICKTHREADS_STKBASE
  #error "Need to know the machine-dependent stack allocation."
#endif

/* All things are put on the stack relative to the final value of
   the stack pointer. */
#ifdef QUICKTHREADS_GROW_DOWN
#define QUICKTHREADS_ADJ(sp)	(((char *)sp) - QUICKTHREADS_STKBASE)
#else
#define QUICKTHREADS_ADJ(sp)	(((char *)sp) + QUICKTHREADS_STKBASE)
#endif

#define QUICKTHREADS_ARGS(sp, pu, pt, userf, only) \
    (QUICKTHREADS_ARGS_MD (QUICKTHREADS_ADJ(sp)), \
     QUICKTHREADS_SPUT (QUICKTHREADS_ADJ(sp), QUICKTHREADS_ONLY_INDEX, only), \
     QUICKTHREADS_SPUT (QUICKTHREADS_ADJ(sp), QUICKTHREADS_USER_INDEX, userf), \
     QUICKTHREADS_SPUT (QUICKTHREADS_ADJ(sp), QUICKTHREADS_ARGT_INDEX, pt), \
     QUICKTHREADS_SPUT (QUICKTHREADS_ADJ(sp), QUICKTHREADS_ARGU_INDEX, pu), \
     ((qt_t *)QUICKTHREADS_ADJ(sp)))

#endif


#ifndef QUICKTHREADS_BLOCK
extern void qt_block(qt_t **curctx, qt_t *newsp);
#define QUICKTHREADS_BLOCK(curctx, newsp)  (qt_block(curctx, newsp))
#endif

#ifdef __cplusplus
}		/* Match `extern "C" {' at top. */
#endif

#endif /* ndef QUICKTHREADS_H */
