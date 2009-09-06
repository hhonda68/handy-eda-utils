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
};

typedef sc_entry_desc sc_method_desc;

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
  ::std::vector<sc_cor_ctx>                cthreads_ctx;
  ::std::vector<sc_cor_ctx>::iterator      curr_ctx;
  const sc_in<bool>                        *reset_port;
  int                                      curr_cthread;
  int                                      nr_cthreads_with_reset;
  bool                                     cthread_aborted;
  bool                                     stopping;
  ::std::vector<sc_method_desc>            methods, cmethods;
  ::std::vector<sc_signal_access_manager*> signals;
  ::std::vector<sc_sensitive_methods*>     smeths;
  ::std::vector<sc_signal_access_manager*> nonblksigs;
  sc_time                                  current_time;
  static void cthread_wrapper(void *arg);
  int search_signal_id(const sc_signal_access_manager& sig);
  void setup_simulation();
  void tick_cthreads(int start_index);
  void tick_cmethods();
  void update_nonblocking_assignments();
  void sort_methods();
  void eval_methods();
  int  check_reset_state();  // ret = cthread_start_index for next cycle
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

void sc_simcontext::register_cthread(sc_module *mod, sc_entry_func func)
{
  sc_cthread_desc desc;
  desc.mod = mod;
  desc.func = func;
  desc.sz_stack = 16*1024;
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
  sc_method_desc desc;
  desc.mod = mod;
  desc.func = func;
  m.methods.push_back(desc);
}

int  sc_simcontext::current_method_id() const { return m.methods.size() - 1; }

void sc_simcontext::mark_method_as_clocked(int method_id)
{
  // assert(method_id == m.methods.size()-1);
  m.cmethods.push_back(m.methods[method_id]);
  m.methods.pop_back();
}

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
    // cthead_ix may have been changed during execution
    int cthread_ix = &(*the_simcontext->m.curr_ctx) - &(*the_simcontext->m.cthreads_ctx.begin());	// (**1)
    the_simcontext->m.cthreads[cthread_ix].mod = 0;
    the_simcontext->m.cthread_aborted = true;
    the_simcontext->wait();
    // (**1) assumes that ::std::vector<T> is implemented straightforwardly as a contiguous array of T.
  }
}

void sc_simcontext::impl_t::setup_simulation()
{
  // initialize cthread coroutines
  {
    int n = cthreads.size();
    cthreads_ctx.resize(n+1);  // +1 for the main thread
    for (int i=0; i<n; ++i) {
      sc_cor_ut::init_thread(&cthreads[i].cor, &cthreads_ctx[i],
			     cthreads[i].sz_stack, cthread_wrapper, reinterpret_cast<void*>(i));
    }
  }
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

void sc_simcontext::impl_t::tick_cthreads(int start_index)
{
  cthread_aborted = false;
  curr_ctx = cthreads_ctx.begin() + start_index;
  sc_cor_ut::yield(&cthreads_ctx.back(), &(*curr_ctx));
  if (cthread_aborted) {
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
}

void sc_simcontext::wait()
{
  sc_cor_ctx& curr = *m.curr_ctx;
  sc_cor_ctx& next = *(++m.curr_ctx);
  sc_cor_ut::yield(&curr, &next);
}

void sc_simcontext::impl_t::tick_cmethods()
{
  int n = cmethods.size();
  for (int i=0; i<n; ++i) {
    sc_method_desc& meth = cmethods[i];
    sc_module *mod = meth.mod;
    sc_entry_func func = meth.func;
    (mod->*func)();
  }
}

void sc_simcontext::impl_t::update_nonblocking_assignments()
{
  int n = nonblksigs.size();
  for (int i=0; i<n; ++i) {
    sc_signal_access_manager *sig = nonblksigs[i];
    bool written = sig->m_written;
    sig->m_rix ^= written;
    sig->m_wix ^= written;
    sig->m_written = false;
  }
}

class PartiallyOrderedSet {
public:
  PartiallyOrderedSet(int nr_elements) : nodes(nr_elements) {}
  void register_order(int predecessor, int successor);
  const ::std::vector<int> topological_sort();
private:
  void topological_sort_recur(int ix);
  struct node_t {
    ::std::set<int> preds;
    bool            visited;
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
  for (int i=0; i<n; ++i)  nodes[i].visited = false;
  ::std::vector<int> result;
  sort_result = &result;
  for (int i=0; i<n; ++i)  topological_sort_recur(i);
  sort_result = 0;
  return result;
}

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

void sc_simcontext::impl_t::sort_methods()
{
  int nr_meth = methods.size();
  int nr_sig = signals.size();
  // clear "written" flag of each signal
  for (int s=0; s<nr_sig; ++s) {
    signals[s]->m_written = false;
  }
  PartiallyOrderedSet pos(nr_meth);
  ::std::set<int> msig;
  for (int m=0; m<nr_meth; ++m) {
    // for each methods, ...
    sc_method_desc& meth = methods[m];
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
  // determine correct evaluation order
  ::std::vector<int> order = pos.topological_sort();
  // sort "methods"
  ::std::vector<sc_method_desc> sorted_methods(nr_meth);
  for (int i=0; i<nr_meth; ++i)  sorted_methods[i] = methods[order[i]];
  using ::std::swap;
  swap(methods, sorted_methods);
  // set "nonblksigs"
  nonblksigs.reserve(nr_sig - msig.size());
  for (int s=0; s<nr_sig; ++s) {
    if (msig.find(s) == msig.end()) {
      sc_signal_access_manager *sig = signals[s];
      nonblksigs.push_back(sig);
      sig->m_wix = 1;
    }
  }
}

void sc_simcontext::impl_t::eval_methods()
{
  int n = methods.size();
  for (int i=0; i<n; ++i) {
    sc_method_desc& meth = methods[i];
    sc_module *mod = meth.mod;
    sc_entry_func func = meth.func;
    (mod->*func)();
  }
}

int sc_simcontext::impl_t::check_reset_state()
{
  if (reset_port != 0) {
    if (! reset_port->read()) {
      return nr_cthreads_with_reset;
    } else {
      reset_port = 0;
    }
  }
  return 0;
}

void sc_simcontext::impl_t::cleanup_simulation()
{
  int n = cthreads.size();
  for (int i=0; i<n; ++i)  sc_cor_ut::destroy_thread(&cthreads[i].cor);
}

void sc_simcontext::scstart()
{
  m.setup_simulation();
  m.stopping = false;
  int cthread_start_index = 0;
  // the first cycle
  m.tick_cthreads(cthread_start_index);
  m.tick_cmethods();
  m.sort_methods();
  m.eval_methods();
  cthread_start_index = m.check_reset_state();
  m.current_time.m_val += 10;
  // subsequent cycles
  while (! m.stopping) {
    m.tick_cthreads(cthread_start_index);
    m.tick_cmethods();
    m.update_nonblocking_assignments();
    m.eval_methods();
    cthread_start_index = m.check_reset_state();
    m.current_time.m_val += 10;
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

