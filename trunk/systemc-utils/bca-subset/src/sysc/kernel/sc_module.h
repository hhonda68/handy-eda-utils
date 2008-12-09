// -*- c++ -*-

//
// Minimal synthesizable BCA subset of SystemC.
//

// Copyright (c) 2008 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

// sc_module

#ifndef BCASYSC_KERNEL_SCMODULE_H
#define BCASYSC_KERNEL_SCMODULE_H

#include "sc_simcontext.h"
#include "../communication/sc_signal.h"

namespace sc_core {

class sc_module_name {
public:
  sc_module_name(const char *name) : m_str(name), m_pushed(true) { the_simcontext->construct_scmodulename(this); }
  sc_module_name(const sc_module_name& x) : m_str(x.m_str), m_pushed(false) {};
  ~sc_module_name() { if (m_pushed)  the_simcontext->destruct_scmodulename(); }
  operator const char*() const { return m_str; }
private:
  const char * const m_str;
  const bool m_pushed;
  // disabled
  sc_module_name();
  sc_module_name& operator=(const sc_module_name&);
};

class sc_sensitive {
public:
  template <typename T> sc_sensitive& operator<<(T& port_or_sig) {
    port_or_sig.sensitive_add(the_simcontext->current_method_id());
    return *this;
  }
};

class sc_module {
public:
  const char* basename() const;
  const char* name() const;
  sc_module();
  void reset_signal_is(sc_in<bool>& port, bool polarity);
  void reset_signal_is(sc_signal<bool>& sig, bool polarity);
  sc_sensitive sensitive;
  void dont_initialize() {}
  void wait() { ::sc_core::wait(); }
private:
  struct impl_t;
  impl_t& m;
};

}; // namespace sc_core

#define SC_MODULE(name)       struct name : ::sc_core::sc_module
#define SC_HAS_PROCESS(name)  typedef name SC_CURRENT_USER_MODULE
#define SC_CTOR(name)         SC_HAS_PROCESS(name);  name(::sc_core::sc_module_name)

#define SC_CTHREAD(funcname, edge) \
  ::sc_core::the_simcontext->register_cthread(this, static_cast< ::sc_core::sc_entry_func >(&SC_CURRENT_USER_MODULE::funcname))
#define SC_METHOD(funcname) \
  ::sc_core::the_simcontext->register_method(this, static_cast< ::sc_core::sc_entry_func >(&SC_CURRENT_USER_MODULE::funcname))

#endif
