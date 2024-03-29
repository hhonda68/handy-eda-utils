// -*- c++ -*-

//
// Minimal synthesizable BCA subset of SystemC.
//

// Copyright (c) 2008 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

// sc_simcontext

#ifndef BCASYSC_KERNEL_SCSIMCONTEXT_H
#define BCASYSC_KERNEL_SCSIMCONTEXT_H

namespace sc_core {

class sc_module_name;
class sc_module;
template <typename T> class sc_in;
template <typename T> class sc_signal;
struct sc_time;

struct sc_signal_access_manager {
  unsigned char m_rix, m_wix;
  bool          m_written;
  unsigned char m_id;
};

class sc_sensitive_methods {
public:
  sc_sensitive_methods() : data(0) {}
  ~sc_sensitive_methods();
  void add(int method_id);
  void merge(sc_sensitive_methods& other);
private:
  friend class sc_simcontext;
  struct data_t;
  data_t *data;
};

typedef void (sc_module::*sc_entry_func)();

class sc_simcontext {
public:
  sc_simcontext();
  ~sc_simcontext();
  void construct_scmodulename(const sc_module_name*);
  void destruct_scmodulename();
  const char *scmodule_basename();
  const sc_module *scmodule_parent();
  void construct_scmodule(const sc_module *);
#ifdef BCASYSC_MULTICLOCK
  void register_clock(sc_signal<bool>&, double);
  void register_cthread(sc_module *, sc_entry_func, const sc_in<bool>&);
  void mark_method_as_clocked(int, const sc_in<bool>&);
#else
  void register_cthread(sc_module *, sc_entry_func);
  void mark_method_as_clocked(int);
#endif
  void register_reset_port(const sc_in<bool>&);
  void mark_cthread_as_resettable();
  void set_cthread_stack_size(int);
  void register_method(sc_module *, sc_entry_func);
  int  current_method_id() const;
  void register_signal(sc_signal_access_manager&);
  void sensitive_add(sc_signal_access_manager&, int);
  void sensitive_merge(sc_signal_access_manager&, sc_sensitive_methods&);
  void scstart();
  void scstop();
  const sc_time& sctimestamp();
  void wait();
private:
  struct impl_t;
  impl_t& m;
};

extern sc_simcontext *the_simcontext;

inline void sc_start() { the_simcontext->scstart(); }
inline void sc_stop() { the_simcontext->scstop(); }
inline const sc_time& sc_time_stamp() { return the_simcontext->sctimestamp(); }
inline void wait() { the_simcontext->wait(); }
inline void wait(int n) { for (int i=0; i<n; ++i)  wait(); }

class sc_trace_file;  // allow user to define sc_trace() [which is unused in this minimal BCA-subset library]

} // namespace sc_core

#endif

