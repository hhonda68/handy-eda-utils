//
// Lightweight restricted-width integer library for C++.
//

// Copyright (c) 2008 the handy-eda-utils develeper(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

#include "fx.h"
#include <iostream>

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

};

