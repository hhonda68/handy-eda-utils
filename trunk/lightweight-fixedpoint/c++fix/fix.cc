//
// Lightweight fixed-point library for C++.
//

// Copyright (c) 2008 the handy-eda-utils develeper(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

// Last modified on October 2007.

#include <iostream>
#include <iomanip>
#include <cmath>
#include "fix.h"

int fix_ftoi_body(double x, int of)
{
  return static_cast<int>(floor(ldexp(x, of)+(1.0/2.0)));
}

double fix_itof_body(int value, int af)
{
  return ldexp(static_cast<double>(value), -af);
}

const std::string fix_hex_body(int value, int width) {
  char buf[9];
  int ix0 = 8 - ((width + 3) >> 2);
  value &= ((1<<width)-1);
  buf[8] = '\0';
  for (int ix=7; ix>=ix0; --ix) {
    buf[ix] = "0123456789abcdef"[value&15];
    value >>= 4;
  }
  return std::string(&buf[ix0]);
}

#if defined(FIX_CHECK_OVERFLOW) || defined(FIX_CHECK_DIVBYZERO)
namespace {
  void fix_error(const char *message, const char *file, int line)
  {
    if (file == NULL) {
      std::cout << message << " at " << __builtin_return_address(1) << std::endl;
    } else {
      std::cout << message << " at " << file << ':' << line << std::endl;
    }
    fix_overflow_flag = true;
  }
}
#endif

#ifdef FIX_CHECK_OVERFLOW
bool fix_overflow_flag = false;
void fix_overflow_error(const char *file, int line)
{
  fix_error("Overflow", file, line);
  fix_overflow_flag = true;
}
#endif

#ifdef FIX_CHECK_DIVBYZERO
void fix_divbyzero_error(const char *file, int line)
{
  fix_error("Division by zero", file, line);
}
#endif

