/*
 * Lightweight restricted-width integer library for C.
 *
 * Copyright (c) 2008 the handy-eda-utils develeper(s).
 * Distributed under the MIT License.
 * (See accompanying file COPYING or copy at
 *  http://www.opensource.org/licenses/mit-license.php.)
 *
 * Last modified on November 2007.
 */

#include	<stdio.h>
#include	"common.h"
#include	"fx.h"

#ifdef FX_CHECK_OVERFLOW

bool fx_overflow_flag = FALSE;

void fx_overflow_error(const char *file, int line)
{
  printf("Overflow at %s:%d\n", file, line);
  fx_overflow_flag = TRUE;
}

#endif

