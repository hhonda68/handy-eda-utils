// -*- c++ -*-

//
// Minimal synthesizable BCA subset of SystemC.
//

// Copyright (c) 2008 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

// sc_clock

#ifndef BCASYSC_COMMUNICATION_SCCLOCK_H
#define BCASYSC_COMMUNICATION_SCCLOCK_H

#include "sc_signal.h"
#include <vector>

namespace sc_core {

enum sc_time_unit;

class sc_clock : public sc_signal<bool> {
public:
  sc_clock(const char* name, double period, sc_time_unit unit) {}
};

};

#endif
