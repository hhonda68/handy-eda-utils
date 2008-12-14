//
// Minimal synthesizable BCA subset of SystemC.
//

// Copyright (c) 2008 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

#include "sc_module.h"
#include <string>

namespace sc_core {

struct sc_module::impl_t {
  const ::std::string basename;
  const sc_module* parent;
  mutable ::std::string name;
  impl_t() : basename(the_simcontext->scmodule_basename()), parent(the_simcontext->scmodule_parent()) {}
};

const char* sc_module::basename() const { return m.basename.c_str(); }

const char* sc_module::name() const
{
  if (m.name.empty()) {
    m.name = (m.parent != 0) ? (m.parent->m.name + "." + m.basename) : m.basename;
  }
  return m.name.c_str();
}

sc_module::sc_module() : m(*(new impl_t))
{
  the_simcontext->construct_scmodule(this);
}

void sc_module::reset_signal_is(sc_in<bool>& port, bool polarity)
{
  the_simcontext->register_reset_port(port);
  the_simcontext->mark_cthread_as_resettable();
}

void sc_module::reset_signal_is(sc_signal<bool>& sig, bool polarity)
{
  the_simcontext->mark_cthread_as_resettable();
}

void sc_module::set_stack_size(int size)
{
  the_simcontext->set_cthread_stack_size(size);
}

}; // namespace sc_core

