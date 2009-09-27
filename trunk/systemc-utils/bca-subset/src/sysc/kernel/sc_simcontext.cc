//
// Minimal synthesizable BCA subset of SystemC.
//

// Copyright (c) 2008 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

#include "sc_simcontext.h"
#include "sc_cor.h"
#include "sc_module.h"
#include "sc_time.h"
#include "../communication/sc_signal.h"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <stack>
#include <set>

namespace sc_core {

struct sc_entry_desc {
  sc_module*    mod;
  sc_entry_func func;
};

struct sc_cthread_desc : sc_entry_desc {
  union {
    int         sz_stack;
    sc_cor_desc cor;
  };
#ifdef BCASYSC_MULTICLOCK
  union {
    const sc_in<bool> *clkport;
    int clkid;
  };
#endif
};

typedef sc_entry_desc sc_method_desc;

#ifdef BCASYSC_MULTICLOCK
struct sc_lmethod_desc : sc_method_desc {
  unsigned int clkmap;
};
struct sc_cmethod_desc : sc_method_desc {
  union {
    const sc_in<bool> *clkport;
    int clkid;
  };
};
#else
typedef sc_method_desc sc_lmethod_desc;
typedef sc_method_desc sc_cmethod_desc;
#endif

#ifdef BCASYSC_MULTICLOCK
struct sc_clock_desc {
  union {
    const sc_signal<bool> *sig;
    unsigned int rest;
  };
  unsigned int period;
};
#endif

struct sc_sensitive_methods::data_t {
  ::std::vector<int> vec;   // list of method id
};

void sc_sensitive_methods::add(int method_id)
{
  if (data == 0)  data = new data_t;
  data->vec.push_back(method_id);
}

void sc_sensitive_methods::merge(sc_sensitive_methods& other)
{
  if (other.data == 0)  return;
  if (data == 0) {
    data = other.data;
    other.data = 0;
  } else {
    ::std::vector<int>& lhs = data->vec;
    const ::std::vector<int>& rhs = other.data->vec;
    int n = rhs.size();
    lhs.reserve(lhs.size() + n);
    for (int i=0; i<n; ++i)  lhs.push_back(rhs[i]);
  }
}

sc_sensitive_methods::~sc_sensitive_methods()
{
  if (data != 0)  delete data;
}

struct sc_simcontext::impl_t {
  ::std::stack<const sc_module_name*>      name_hierarchy;
  ::std::stack<const sc_module*>           module_hierarchy;
  ::std::vector<sc_cthread_desc>           cthreads;
  typedef ::std::vector<int>               int_vec_t;
  typedef ::std::vector<sc_cor_ctx>        ctx_vec_t;
  typedef ::std::vector<sc_method_desc>    method_vec_t;
  typedef ::std::vector<sc_signal_access_manager*> sigptr_vec_t;
#ifdef BCASYSC_MULTICLOCK
  ::std::vector<sc_clock_desc>             clocks;
  unsigned int                             active_clock_map;
  ::std::vector<ctx_vec_t>                 cthreads_ctx_tab;
  ::std::vector<int_vec_t>                 cthreads_id_tab;
  int_vec_t                                nr_cthreads_with_reset_tab;
  ::std::vector<method_vec_t*>             methods_tab;
  ::std::vector<method_vec_t>              cmethods_tab;
  ::std::vector<sigptr_vec_t>              nonblksigs_tab;
#else
  ctx_vec_t                                cthreads_ctx;
  sigptr_vec_t                             nonblksigs;
#endif
  int                                      nr_cthreads_with_reset;
  ctx_vec_t::iterator                      curr_ctx;
  const sc_in<bool>                        *reset_port;
  int                                      curr_cthread;
  bool                                     cthread_aborted;
  bool                                     stopping;
  ::std::vector<sc_lmethod_desc>           methods;
  ::std::vector<sc_cmethod_desc>           cmethods;
  sigptr_vec_t                             signals;
  ::std::vector<sc_sensitive_methods*>     smeths;
  sc_time                                  current_time;
  static void cthread_wrapper(void *arg);
  int search_signal_id(const sc_signal_access_manager& sig);
  void setup_simulation();
  void tick_cthreads_first();
  void tick_cmethods_first();
  void sort_methods();
  void eval_methods_first();
#ifdef BCASYSC_MULTICLOCK
  void cleanup_aborted_cthreads(int clk);
#else
  void cleanup_aborted_cthreads();
#endif
  void tick_simulation_time();
  int  check_reset_state();  // ret = cthread_start_index for next cycle
  void tick_cthreads(int start_index);
  void tick_cmethods();
  void update_nonblocking_assignments();
  void eval_methods();
  void cleanup_simulation();
};

sc_simcontext::sc_simcontext() : m(*(new impl_t))
{
  m.reset_port = 0;
  m.nr_cthreads_with_reset = 0;
  m.current_time.m_val = 0;
}

sc_simcontext::~sc_simcontext() { delete &m; }

void sc_simcontext::construct_scmodulename(const sc_module_name* name) { m.name_hierarchy.push(name); }
void sc_simcontext::destruct_scmodulename() { m.name_hierarchy.pop(); m.module_hierarchy.pop(); }
const char *sc_simcontext::scmodule_basename() { return *(m.name_hierarchy.top()); }
const sc_module *sc_simcontext::scmodule_parent() { return m.module_hierarchy.empty() ? 0 : m.module_hierarchy.top(); }
void sc_simcontext::construct_scmodule(const sc_module *mod) { m.module_hierarchy.push(mod); }

#ifdef BCASYSC_MULTICLOCK
void sc_simcontext::register_clock(sc_signal<bool>& sig, double period)
{
  sc_clock_desc desc;
  desc.sig = &sig;
  desc.period = static_cast<unsigned int>(period);
  if (static_cast<double>(desc.period) != period)  throw ::std::logic_error("non-integer clock period");
  m.clocks.push_back(desc);
}
#endif

#ifdef BCASYSC_MULTICLOCK
void sc_simcontext::register_cthread(sc_module *mod, sc_entry_func func, const sc_in<bool>& clkport)
#else
void sc_simcontext::register_cthread(sc_module *mod, sc_entry_func func)
#endif
{
  sc_cthread_desc desc;
  desc.mod = mod;
  desc.func = func;
  desc.sz_stack = 16*1024;
#ifdef BCASYSC_MULTICLOCK
  desc.clkport = &clkport;
#endif
  m.curr_cthread = m.cthreads.size();
  m.cthreads.push_back(desc);
}

void sc_simcontext::register_reset_port(const sc_in<bool>& port)
{
  if (m.reset_port == 0)  m.reset_port = &port;
}

void sc_simcontext::mark_cthread_as_resettable()
{
  int nc = m.curr_cthread;
  int nr = m.nr_cthreads_with_reset;
  m.nr_cthreads_with_reset = nr + 1;
  if (nr == nc)  return;
  sc_cthread_desc &cdesc = m.cthreads[nc];
  sc_cthread_desc &rdesc = m.cthreads[nr];
  sc_cthread_desc tmp = rdesc;  rdesc = cdesc;  cdesc = tmp;
  m.curr_cthread = nr;
}

void sc_simcontext::set_cthread_stack_size(int size)
{
  m.cthreads[m.curr_cthread].sz_stack = size;
}

void sc_simcontext::register_method(sc_module *mod, sc_entry_func func)
{
  sc_lmethod_desc desc;
  desc.mod = mod;
  desc.func = func;
#ifdef BCASYSC_MULTICLOCK
  desc.clkmap = 0;
#endif
  m.methods.push_back(desc);
}

int  sc_simcontext::current_method_id() const { return m.methods.size() - 1; }

#ifdef BCASYSC_MULTICLOCK
void sc_simcontext::mark_method_as_clocked(int method_id, const sc_in<bool>& clkport)
{
  // assert(method_id == m.methods.size()-1);
  sc_lmethod_desc& desc = m.methods[method_id];
  sc_cmethod_desc cdesc;
  cdesc.mod = desc.mod;
  cdesc.func = desc.func;
  cdesc.clkport = &clkport;
  m.cmethods.push_back(cdesc);
  m.methods.pop_back();
}
#else
void sc_simcontext::mark_method_as_clocked(int method_id)
{
  // assert(method_id == m.methods.size()-1);
  m.cmethods.push_back(m.methods[method_id]);
  m.methods.pop_back();
}
#endif

void sc_simcontext::register_signal(sc_signal_access_manager& sig)
{
  sig.m_id = m.signals.size() & 0xff;
  m.signals.push_back(&sig);
  m.smeths.push_back(0);
}

int sc_simcontext::impl_t::search_signal_id(const sc_signal_access_manager& sig)
{
  int id = sig.m_id;
  while (signals[id] != &sig)  id += 0x100;
  return id;
}

void sc_simcontext::sensitive_add(sc_signal_access_manager& sig, int method_id)
{
  int id = m.search_signal_id(sig);
  if (m.smeths[id] == 0)  m.smeths[id] = new sc_sensitive_methods;
  m.smeths[id]->add(method_id);
}

void sc_simcontext::sensitive_merge(sc_signal_access_manager& sig, sc_sensitive_methods& other)
{
  if (other.data == 0)  return;
  int id = m.search_signal_id(sig);
  if (m.smeths[id] == 0)  m.smeths[id] = new sc_sensitive_methods;
  m.smeths[id]->merge(other);
}

////////////////////////////////////////////////////////////////

void sc_simcontext::impl_t::cthread_wrapper(void *arg)
{
  try {
    int cthread_ix = reinterpret_cast<long long>(arg);  // reinterpret to long long because "void*" may not fit in "int"
    sc_cthread_desc *desc = &the_simcontext->m.cthreads[cthread_ix];
    sc_module *mod = desc->mod;
    void (sc_module::*func)() = desc->func;
    (mod->*func)();
  } catch (::std::exception& e) {
    ::std::cerr << "Unhandled exception in a cthread: " << e.what() << ".\n";
  } catch (...) {
    ::std::cerr << "Unhandled exception in a cthread.\n";
  }
  {
#ifdef BCASYSC_MULTICLOCK
    int cthread_ix = reinterpret_cast<long long>(arg);
#else
    // cthead_ix may have been changed during execution
    int cthread_ix = &(*the_simcontext->m.curr_ctx) - &(*the_simcontext->m.cthreads_ctx.begin());
#endif
    the_simcontext->m.cthreads[cthread_ix].mod = 0;
    the_simcontext->m.cthread_aborted = true;
    the_simcontext->wait();
  }
}

void sc_simcontext::impl_t::setup_simulation()
{
#ifdef BCASYSC_MULTICLOCK
  {
    int nr_clk = clocks.size();
    // convert clock ports to clock IDs
    int nr_cthread = cthreads.size();
    int_vec_t nr_cthreads_tab;
    nr_cthreads_tab.resize(nr_clk);
    nr_cthreads_with_reset_tab.resize(nr_clk);
    for (int i=0; i<nr_cthread; ++i) {
      const sc_signal<bool> *clksig = cthreads[i].clkport->m_sig;
      for (int clk=0; clk<nr_clk; ++clk) {
        if (clocks[clk].sig == clksig) {
          cthreads[i].clkid = clk;
          ++ nr_cthreads_tab[clk];
          if (i < nr_cthreads_with_reset)  ++ nr_cthreads_with_reset_tab[clk];
          goto found_cthread_clk;
        }
      }
      throw ::std::logic_error("curious clock for an SC_CTHREAD");
    found_cthread_clk:
      ;
    }
    int nr_cmethod = cmethods.size();
    int_vec_t nr_cmethods_tab;
    nr_cmethods_tab.resize(nr_clk);
    for (int i=0; i<nr_cmethod; ++i) {
      const sc_signal<bool> *clksig = cmethods[i].clkport->m_sig;
      for (int clk=0; clk<nr_clk; ++clk) {
        if (clocks[clk].sig == clksig) {
          cmethods[i].clkid = clk;
          ++ nr_cmethods_tab[clk];
          goto found_cmethod_clk;
        }
      }
      throw ::std::logic_error("curious clock for an SC_CMETHOD");
    found_cmethod_clk:
      ;
    }
    for (int clk=0; clk<nr_clk; ++clk)  clocks[clk].rest = clocks[clk].period;
    // group cthreads by associated clocks and initialize cthread coroutines
    cthreads_ctx_tab.resize(nr_clk);
    cthreads_id_tab.resize(nr_clk);
    for (int clk=0; clk<nr_clk; ++clk) {
      int nr = nr_cthreads_tab[clk];
      cthreads_ctx_tab[clk].resize(nr + 1);	// +1 for the main thread
      cthreads_id_tab[clk].reserve(nr);
    }
    for (int i=0; i<nr_cthread; ++i) {
      int clk = cthreads[i].clkid;
      int ix = cthreads_id_tab[clk].size();
      sc_cor_ut::init_thread(&cthreads[i].cor, &cthreads_ctx_tab[clk][ix],
			     cthreads[i].sz_stack, cthread_wrapper, reinterpret_cast<void*>(i));
      cthreads_id_tab[clk].push_back(i);
    }
    // group cmethods by associated clocks
    cmethods_tab.resize(nr_clk);
    for (int clk=0; clk<nr_clk; ++clk) {
      int nr = nr_cmethods_tab[clk];
      cmethods_tab[clk].reserve(nr);
    }
    for (int i=0; i<nr_cmethod; ++i) {
      int clk = cmethods[i].clkid;
      cmethods_tab[clk].push_back(static_cast<sc_method_desc&>(cmethods[i]));
    }
    // initialize methods_tab
    {
      if (nr_clk > 10)  throw ::std::logic_error("Too many (>10) clocks");
      methods_tab.resize(1u<<nr_clk);
    }
  }
#else
  // initialize cthread coroutines
  {
    int n = cthreads.size();
    cthreads_ctx.resize(n+1);  // +1 for the main thread
    for (int i=0; i<n; ++i) {
      sc_cor_ut::init_thread(&cthreads[i].cor, &cthreads_ctx[i],
			     cthreads[i].sz_stack, cthread_wrapper, reinterpret_cast<void*>(i));
    }
  }
#endif
  // initialize sc_signal_access_manager's
  {
    int n = signals.size();
    for (int i=0; i<n; ++i) {
      sc_signal_access_manager *sig = signals[i];
      sig->m_rix = 0;
      sig->m_wix = 0;
      sig->m_written = false;
    }
  }
  // force current value of the reset signal to be "active(false)"
  if (reset_port != 0) {
    reset_port->m_sig->write(false);
    reset_port->m_sig->m_written = false;
  }
}

#ifdef BCASYSC_MULTICLOCK
void sc_simcontext::impl_t::cleanup_aborted_cthreads(int clk)
{
  ctx_vec_t& cthreads_ctx = cthreads_ctx_tab[clk];	
  int_vec_t& cthreads_id = cthreads_id_tab[clk];
  int n = cthreads_id.size();
  int oldix = 0;
  while (cthreads[cthreads_id[oldix]].mod != 0)  ++oldix;
  int newix = oldix;
  for (++oldix; oldix<n; ++oldix) {
    if (cthreads[cthreads_id[oldix]].mod != 0) {
      cthreads_ctx[newix] = cthreads_ctx[oldix];
      cthreads_id[newix] = cthreads_id[oldix];
      ++ newix;
    }
  }
  cthreads_id.resize(newix);
  cthreads_ctx[newix] = cthreads_ctx[n];
  cthreads_ctx.resize(newix+1);  // +1 for the main thread
}
#else
void sc_simcontext::impl_t::cleanup_aborted_cthreads()
{
  int n = cthreads.size();
  int oldix = 0;
  while (cthreads[oldix].mod != 0)  ++oldix;
  int newix = oldix;
  sc_cor_ut::destroy_thread(&cthreads[oldix].cor);
  for (++oldix; oldix<n; ++oldix) {
    if (cthreads[oldix].mod == 0) {
      sc_cor_ut::destroy_thread(&cthreads[oldix].cor);
    } else {
      cthreads[newix] = cthreads[oldix];
      cthreads_ctx[newix] = cthreads_ctx[oldix];
      ++ newix;
    }
  }
  cthreads.resize(newix);
  cthreads_ctx[newix] = cthreads_ctx[n];
  cthreads_ctx.resize(newix+1);  // +1 for the main thread
}
#endif

#ifdef BCASYSC_MULTICLOCK

void sc_simcontext::impl_t::tick_cthreads_first()
{
  int nr_clk = clocks.size();
  int nr_sig = signals.size();
  nonblksigs_tab.resize(nr_clk+1);	// +1 for "signals whose drivers are unknown"
  for (int clk=0; clk<nr_clk; ++clk) {
    ctx_vec_t& cthreads_ctx = cthreads_ctx_tab[clk];
    if (cthreads_ctx.size() != 0) {
      curr_ctx = cthreads_ctx.begin();
      cthread_aborted = false;
      sc_cor_ut::yield(&cthreads_ctx.back(), &(*curr_ctx));
      if (cthread_aborted)  cleanup_aborted_cthreads(clk);
      int n = 0;
      for (int s=0; s<nr_sig; ++s)  n += signals[s]->m_written;
      sigptr_vec_t& nonblksigs = nonblksigs_tab[clk];
      nonblksigs.reserve(n);
      for (int s=0; s<nr_sig; ++s) {
        sc_signal_access_manager *sig = signals[s];
        if (! sig->m_written)  continue;
        sig->m_written = false;
        nonblksigs.push_back(sig);
      }
    }
  }
}

void sc_simcontext::impl_t::tick_cmethods_first()
{
  int nr_clk = clocks.size();
  int nr_sig = signals.size();
  for (int clk=0; clk<nr_clk; ++clk) {
    method_vec_t methvec = cmethods_tab[clk];
    int nr_meth = methvec.size();
    for (int i=0; i<nr_meth; ++i) {
      sc_method_desc& meth = methvec[i];
      sc_module *mod = meth.mod;
      sc_entry_func func = meth.func;
      (mod->*func)();
    }
    int n = 0;	
    for (int s=0; s<nr_sig; ++s)  n += signals[s]->m_written;
    sigptr_vec_t& nonblksigs = nonblksigs_tab[clk];
    nonblksigs.reserve(nonblksigs.size() + n);
    for (int s=0; s<nr_sig; ++s) {
      sc_signal_access_manager *sig = signals[s];
      if (! sig->m_written)  continue;
      sig->m_written = false;
      nonblksigs.push_back(sig);
    }
  }
}

#else
inline void sc_simcontext::impl_t::tick_cthreads_first() { tick_cthreads(0); }
inline void sc_simcontext::impl_t::tick_cmethods_first() { tick_cmethods(); }
#endif

void sc_simcontext::impl_t::tick_cthreads(int start_index)
{
#ifdef BCASYSC_MULTICLOCK
  unsigned int clkmap = active_clock_map;
  do {
    int clk = __builtin_ctz(clkmap);
    clkmap ^= (1u<<clk);
    ctx_vec_t& cthreads_ctx = cthreads_ctx_tab[clk];
    curr_ctx = cthreads_ctx.begin();
    if (start_index)  curr_ctx += nr_cthreads_with_reset_tab[clk];
    sc_cor_ctx *sched_ctx = &cthreads_ctx.back();
    if (&(*curr_ctx) != sched_ctx) {
      cthread_aborted = false;
      sc_cor_ut::yield(sched_ctx, &(*curr_ctx));
      if (cthread_aborted)  cleanup_aborted_cthreads(clk);
    }
  } while (clkmap != 0);
#else
  cthread_aborted = false;
  curr_ctx = cthreads_ctx.begin() + start_index;
  // assert(&cthreads_ctx.back() != &(*curr_ctx));
  // [i.e., We assume at least one "SC_CTHREAD without reset" exists.]
  sc_cor_ut::yield(&cthreads_ctx.back(), &(*curr_ctx));
  if (cthread_aborted)  cleanup_aborted_cthreads();
#endif
}

void sc_simcontext::wait()
{
  sc_cor_ctx& curr = *m.curr_ctx;
  sc_cor_ctx& next = *(++m.curr_ctx);
  sc_cor_ut::yield(&curr, &next);
}

void sc_simcontext::impl_t::tick_cmethods()
{
#ifdef BCASYSC_MULTICLOCK
  unsigned int clkmap = active_clock_map;
  do {
    int clk = __builtin_ctz(clkmap);
    clkmap ^= (1u<<clk);
    method_vec_t& methvec = cmethods_tab[clk];
    int n = methvec.size();
    for (int i=0; i<n; ++i) {
      sc_method_desc& meth = methvec[i];
      sc_module *mod = meth.mod;
      sc_entry_func func = meth.func;
      (mod->*func)();
    }
  } while (clkmap != 0);
#else
  int n = cmethods.size();
  for (int i=0; i<n; ++i) {
    sc_method_desc& meth = cmethods[i];
    sc_module *mod = meth.mod;
    sc_entry_func func = meth.func;
    (mod->*func)();
  }
#endif
}

void sc_simcontext::impl_t::update_nonblocking_assignments()
{
#ifdef BCASYSC_MULTICLOCK
  int nr_clk = clocks.size();
  //assert(nr_clk < 32);
  unsigned int clkmap = active_clock_map | (1u<<nr_clk);
  do {
    int clk = __builtin_ctz(clkmap);
    clkmap ^= (1u<<clk);
    sigptr_vec_t& nonblksigs = nonblksigs_tab[clk];
    int n = nonblksigs.size();
    for (int i=0; i<n; ++i) {
      sc_signal_access_manager *sig = nonblksigs[i];
      bool written = sig->m_written;
      sig->m_rix ^= written;
      sig->m_wix ^= written;
      sig->m_written = false;
    }
  } while (clkmap != 0);
#else
  int n = nonblksigs.size();
  for (int i=0; i<n; ++i) {
    sc_signal_access_manager *sig = nonblksigs[i];
    bool written = sig->m_written;
    sig->m_rix ^= written;
    sig->m_wix ^= written;
    sig->m_written = false;
  }
#endif
}

class PartiallyOrderedSet {
public:
  explicit PartiallyOrderedSet(int nr_elements) : nodes(nr_elements) {}
  void register_order(int predecessor, int successor);
#ifdef BCASYSC_MULTICLOCK
  void register_order_clk(int clk, int element) { register_order(~clk, element); }
  unsigned int clkmap(int element) const { return nodes[element].clkmap; }
#endif
  const ::std::vector<int> topological_sort();
private:
#ifdef BCASYSC_MULTICLOCK
  unsigned int topological_sort_recur(int ix);
#else
  void topological_sort_recur(int ix);
#endif
  struct node_t {
    ::std::set<int> preds;
#ifdef BCASYSC_MULTICLOCK
    unsigned int    clkmap;
#else
    bool            visited;
#endif
  };
  ::std::vector<node_t> nodes;
  ::std::vector<int>    *sort_result;
};

void PartiallyOrderedSet::register_order(int predecessor, int successor)
{
  nodes[successor].preds.insert(predecessor);
}

const ::std::vector<int> PartiallyOrderedSet::topological_sort()
{
  int n = nodes.size();
#ifdef BCASYSC_MULTICLOCK
  for (int i=0; i<n; ++i)  nodes[i].clkmap = 0;
#else
  for (int i=0; i<n; ++i)  nodes[i].visited = false;
#endif
  ::std::vector<int> result;
  result.reserve(n);
  sort_result = &result;
  for (int i=0; i<n; ++i)  topological_sort_recur(i);
  sort_result = 0;
  return result;
}

#ifdef BCASYSC_MULTICLOCK
unsigned int PartiallyOrderedSet::topological_sort_recur(int ix)
{
  if (ix < 0)  return 1u<<(~ix);
  node_t& node = nodes[ix];
  if (node.clkmap != 0)  return node.clkmap;
  for (::std::set<int>::const_iterator iter=node.preds.begin(); iter!=node.preds.end(); ++iter) {
    node.clkmap |= topological_sort_recur(*iter);
  }
  sort_result->push_back(ix);
  return node.clkmap;
}
#else
void PartiallyOrderedSet::topological_sort_recur(int ix)
{
  node_t& node = nodes[ix];
  if (node.visited)  return;
  node.visited = true;
  for (::std::set<int>::const_iterator iter=node.preds.begin(); iter!=node.preds.end(); ++iter) {
    topological_sort_recur(*iter);
  }
  sort_result->push_back(ix);
}
#endif

void sc_simcontext::impl_t::sort_methods()
{
  int nr_meth = methods.size();
  int nr_sig = signals.size();
  PartiallyOrderedSet pos(nr_meth);
#ifdef BCASYSC_MULTICLOCK
  int nr_clk = clocks.size();
  for (int clk=0; clk<nr_clk; ++clk) {
    sigptr_vec_t& nonblksigs = nonblksigs_tab[clk];
    int n = nonblksigs.size();
    for (int i=0; i<n; ++i) {
      int s = search_signal_id(*nonblksigs[i]);
      if (smeths[s] != 0) {
        ::std::vector<int>& vec = smeths[s]->data->vec;
        int vecsize = vec.size();
        for (int j=0; j<vecsize; ++j) {
          // clock "clk" drives signal "s".
          // method "vec[j]" is sensitive to signal "s".
          pos.register_order_clk(clk, vec[j]);
        }
      }
    }
  }
#else
  // clear "written" flag of each signal
  for (int s=0; s<nr_sig; ++s) {
    signals[s]->m_written = false;
  }
#endif
  ::std::set<int> msig;
  for (int m=0; m<nr_meth; ++m) {
    // for each methods, ...
    sc_lmethod_desc& meth = methods[m];
    // experimentally evaluate it.
    sc_module *mod = meth.mod;
    sc_entry_func func = meth.func;
    (mod->*func)();
    // search written signals.
    for (int s=0; s<nr_sig; ++s) {
      sc_signal_access_manager *sig = signals[s];
      if (! sig->m_written)  continue;
      msig.insert(s);
      sig->m_written = false;
      // judge dependency of methods
      if (smeths[s] == 0)  continue;
      ::std::vector<int>& vec = smeths[s]->data->vec;
      int n = vec.size();
      for (int i=0; i<n; ++i) {
	// method "m" writes to signal "s".
	// method "vec[i]" is sensitive to signal "s".
	pos.register_order(m, vec[i]);
      }
    }
  }
#ifdef BCASYSC_MULTICLOCK
  // set "nonblksigs_tab[nr_clk]" (non blocking signals driven by unknown SC_CTHREAD)
  ::std::set<sc_signal_access_manager*> sigset;	
  for (int s=0; s<nr_sig; ++s) {
    if (msig.find(s) == msig.end()) {
      sc_signal_access_manager *sig = signals[s];
      sigset.insert(sig);
      sig->m_wix = 1;
    }
  }
  for (int clk=0; clk<nr_clk; ++clk) {
    sigptr_vec_t& sigvec = nonblksigs_tab[clk];
    int n = sigvec.size();
    for (int i=0; i<n; ++i)  sigset.erase(sigvec[i]);
  }
  sigptr_vec_t& nonblksigs = nonblksigs_tab[nr_clk];
  nonblksigs.reserve(sigset.size());
  for (::std::set<sc_signal_access_manager*>::iterator iter=sigset.begin(); iter!=sigset.end(); ++iter) {
    sc_signal_access_manager *sig = *iter;
    nonblksigs.push_back(sig);
    int s = search_signal_id(*sig);
    if (smeths[s] != 0) {
      ::std::vector<int>& vec = smeths[s]->data->vec;
      int vecsize = vec.size();
      for (int j=0; j<vecsize; ++j) {
        // some (unknown) clock drives signal "s".
        // method "vec[j]" is sensitive to signal "s".
        int m = vec[j];
        for (int clk=0; clk<nr_clk; ++clk)  pos.register_order_clk(clk, m);
      }
    }
  }
#else
  // set "nonblksigs"
  nonblksigs.reserve(nr_sig - msig.size());
  for (int s=0; s<nr_sig; ++s) {
    if (msig.find(s) == msig.end()) {
      sc_signal_access_manager *sig = signals[s];
      nonblksigs.push_back(sig);
      sig->m_wix = 1;
    }
  }
#endif
  // determine correct evaluation order
  ::std::vector<int> order = pos.topological_sort();
  // sort "methods"
  ::std::vector<sc_lmethod_desc> sorted_methods;
  sorted_methods.reserve(nr_meth);
  for (int i=0; i<nr_meth; ++i) {
    int ii = order[i];
#ifdef BCASYSC_MULTICLOCK
    methods[ii].clkmap = pos.clkmap(ii);
#endif
    sorted_methods.push_back(methods[ii]);
  }
  using ::std::swap;
  swap(methods, sorted_methods);
}

void sc_simcontext::impl_t::eval_methods_first()
{
  int n = methods.size();
  for (int i=0; i<n; ++i) {
    sc_lmethod_desc& meth = methods[i];
    sc_module *mod = meth.mod;
    sc_entry_func func = meth.func;
    (mod->*func)();
  }
}

#ifdef BCASYSC_MULTICLOCK
void sc_simcontext::impl_t::eval_methods()
{
  unsigned int clkmap = active_clock_map;
  method_vec_t *methvec = methods_tab[clkmap];
  if (methvec == 0) {
    methvec = new method_vec_t;
    methods_tab[clkmap] = methvec;
    int nr_meth = methods.size();
    int nr_active_meth = 0;
    for (int m=0; m<nr_meth; ++m)  nr_active_meth += ((methods[m].clkmap & clkmap) != 0);
    methvec->reserve(nr_active_meth);
    if (nr_active_meth != 0) {
      for (int m=0; m<nr_meth; ++m) {
        if ((methods[m].clkmap & clkmap) != 0)  methvec->push_back(static_cast<sc_method_desc&>(methods[m]));
      }
    }
  }
  int n = methvec->size();
  for (int i=0; i<n; ++i) {
    sc_method_desc& meth = (*methvec)[i];
    sc_module *mod = meth.mod;
    sc_entry_func func = meth.func;
    (mod->*func)();
  }
}
#else
inline void sc_simcontext::impl_t::eval_methods() { eval_methods_first(); }
#endif

int sc_simcontext::impl_t::check_reset_state()
{
  if (reset_port != 0) {
    if (! reset_port->read()) {
#ifdef BCASYSC_MULTICLOCK
      return 1;
#else
      return nr_cthreads_with_reset;
#endif
    } else {
      reset_port = 0;
    }
  }
  return 0;
}

#ifdef BCASYSC_MULTICLOCK
void sc_simcontext::impl_t::tick_simulation_time()
{
  int nr_clk = clocks.size();
  switch (nr_clk) {
  case 1:
    current_time.m_val += 10;
    break;
  case 2:
    {
      sc_clock_desc& desc0 = clocks[0];
      sc_clock_desc& desc1 = clocks[1];
      unsigned int rest0 = desc0.rest;
      unsigned int rest1 = desc1.rest;
      if (rest0 < rest1) {
        current_time.m_val += rest0;
        active_clock_map = (1u<<0);
        desc0.rest = desc0.period;
        desc1.rest = rest1 - rest0;
      } else if (rest0 > rest1) {
        current_time.m_val += rest1;
        active_clock_map = (1u<<1);
        desc0.rest = rest0 - rest1;
        desc1.rest = desc1.period;
      } else /*if (rest0 == rest1)*/ {
        current_time.m_val += rest0;
        active_clock_map = (1u<<0) | (1u<<1);
        desc0.rest = desc0.period;
        desc1.rest = desc1.period;
      }
    }
    break;
  default:
    {
      unsigned int clkmap = (1u<<0);
      unsigned int minrest = clocks[0].rest;
      for (int clk=1; clk<nr_clk; ++clk) {
        unsigned int rest = clocks[clk].rest;
        if (rest < minrest) {
          clkmap = (1u<<clk);
          minrest = rest;
        } else if (rest == minrest) {
          clkmap |= (1u<<clk);
        }
      }
      active_clock_map = clkmap;
      for (int clk=0; clk<nr_clk; ++clk) {
        if ((clkmap & 1) != 0) {
          //assert(clocks[clk].rest == minrest);
          clocks[clk].rest = clocks[clk].period;
        } else {
          clocks[clk].rest -= minrest;
        }
        clkmap >>= 1;
      }
      current_time.m_val += minrest;
    }
    break;
  }
}
#else
inline void sc_simcontext::impl_t::tick_simulation_time() { current_time.m_val += 10; }
#endif

void sc_simcontext::impl_t::cleanup_simulation()
{
  int n = cthreads.size();
  for (int i=0; i<n; ++i)  sc_cor_ut::destroy_thread(&cthreads[i].cor);
}

void sc_simcontext::scstart()
{
  m.setup_simulation();
  m.stopping = false;
  // the first cycle
  m.tick_cthreads_first();
  m.tick_cmethods_first();
  m.sort_methods();
  m.eval_methods_first();
  // subsequent cycles
  while (! m.stopping) {
    m.tick_simulation_time();
    m.tick_cthreads(m.check_reset_state());
    m.tick_cmethods();
    m.update_nonblocking_assignments();
    m.eval_methods();
  }
  m.cleanup_simulation();
}

void sc_simcontext::scstop()
{
  m.stopping = true;
}

const sc_time& sc_simcontext::sctimestamp()
{
  return m.current_time;
}

sc_simcontext *the_simcontext;

} // namespace sc_core

int main(int argc, char *argv[]) {
  try {
    using ::sc_core::the_simcontext;
    the_simcontext = new ::sc_core::sc_simcontext;
    extern int sc_main(int, char*[]);
    int ret = sc_main(argc, argv);
    delete the_simcontext;
    return ret;
  } catch (::std::exception& e) {
    ::std::cerr << "Error: " << e.what() << ".\n";
    return 1;
  } catch (...) {
    ::std::cerr << "Error: unknown exception.\n";
    return 1;
  }
}

