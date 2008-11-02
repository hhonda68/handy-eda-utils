/*
 * Lightweight fixed-point library for C.
 *
 * Copyright (c) 2008 the handy-eda-utils develeper(s).
 * Distributed under the MIT License.
 * (See accompanying file COPYING or copy at
 *  http://www.opensource.org/licenses/mit-license.php.)
 *
 * Last modified on November 2007.
 */

#include	<stdio.h>
#include	<math.h>
#include	"common.h"
#include	"fix.h"

#ifdef FIX_CHECK_OVERFLOW

bool fix_overflow_flag = FALSE;

void fix_overflow_error(const char *file, int line)
{
  printf("Overflow at %s:%d\n", file, line);
  fix_overflow_flag = TRUE;
}

#endif

#ifdef FIX_CHECK_DIVBYZERO

void fix_divbyzero_error_body(const char *file, int line)
{
  printf("Division by zero at %s:%d\n", file, line);
}

#endif

#ifdef FIX_CHECK_UNUSED_LSB

void fix_checklsb_warning(int type, const char *file, int line)
{
  if (type == 0) {
    printf("Unused LSB bits at %s:%d\n", file, line);
  } else {
    printf("Excess subtractor LSB bits at %s:%d\n", file, line);
  }
}

#endif

void fix_sft_invalid_error(const char *file, int line)
{
  printf("Format mismatch in fix_sft() at %s:%d\n", file, line);
}

int fix_ftoi_body(double val, int of)
{
  return (int)floor(ldexp(val, of)+(1.0/2.0));
}

double fix_itof_body(int val, int af)
{
  return ldexp((double)val, -af);
}

