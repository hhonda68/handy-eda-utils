// -*- c++ -*-

//
// Minimal synthesizable BCA subset of SystemC.
//

// Copyright (c) 2008 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

// sc_time

#ifndef BCASYSC_KERNEL_SCTIME_H
#define BCASYSC_KERNEL_SCTIME_H

#include <iosfwd>

namespace sc_core {

enum sc_time_unit { SC_NS };

struct sc_time {
  typedef unsigned long long value_type;
  value_type m_val;
};

inline void sc_set_time_resolution(double, sc_time_unit) {}

extern ::std::ostream& operator<<(::std::ostream& os, const sc_time& tim);

} // namespace sc_core

#endif
