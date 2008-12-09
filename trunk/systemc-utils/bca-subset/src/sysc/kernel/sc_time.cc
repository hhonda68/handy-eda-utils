//
// Minimal synthesizable BCA subset of SystemC.
//

// Copyright (c) 2008 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

#include "sc_time.h"
#include <ostream>

namespace sc_core {

::std::ostream& operator<<(::std::ostream& os, const sc_time& tim)
{
  return (os << tim.m_val << "ns");
}


}; // namespace sc_core

