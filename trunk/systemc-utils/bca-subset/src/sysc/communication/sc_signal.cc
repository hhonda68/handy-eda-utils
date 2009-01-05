// -*- c++ -*-

//
// Minimal synthesizable BCA subset of SystemC.
//

// Copyright (c) 2008 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

#include "sc_signal.h"
#include <vector>

namespace sc_core {

struct sc_in_ports_generic::impl_t {
  ::std::vector<void *> vec;
};

sc_in_ports_generic::sc_in_ports_generic() : m(*(new impl_t)) {}
sc_in_ports_generic::~sc_in_ports_generic() { delete &m; }

int sc_in_ports_generic::size()                           { return m.vec.size(); }
void sc_in_ports_generic::push_back(void *x)              { m.vec.push_back(x); }
void * const sc_in_ports_generic::operator[](int n) const { return m.vec[n]; }

} // namespace sc_core
