//
// Lightweight restricted-width integer library for C++.
//

// Copyright (c) 2008 the handy-eda-utils develeper(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

#include "fx.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace fx {

#ifdef FX_CHECK_OVERFLOW

void overflow_error(const char *file, int line)
{
  std::cerr << "Overflow at ";
  if (file == 0) {
    std::cerr << __builtin_return_address(0);
  } else {
    std::cerr << file << ' ' << line;
  }
  std::cerr << ".\n";
}

#endif

std::string convert_to_hexadecimal_string(value_type val, int width)
{
  std::ostringstream strm;
  strm << std::setbase(16) << std::setw((width+3)>>2) << std::setfill('0')
       << (val & ((static_cast<value_type>(1)<<width)-1));
  return strm.str();
}

std::string convert_to_binary_string(value_type val, int width)
{
  char buf[sizeof(value_type)*8+1];
  int ix = sizeof(value_type)*8 - width;
  int i = sizeof(value_type)*8;
  buf[i] = '\0';
  do {
    buf[--i] = '0' + (val&1);
    val >>= 1;
  } while (i != ix);
  return std::string(&buf[ix]);
}

};

