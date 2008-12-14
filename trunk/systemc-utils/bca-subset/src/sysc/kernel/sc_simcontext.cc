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
#include <stack>

namespace sc_core {

struct sc_entry_desc {
  sc_module*    mod;
  sc_entry_func func;
};

struct sc_cthread_desc : sc_entry_desc {
  sc_cor cor;
};
struct sc_method_desc : sc_entry_desc {
  int eval_order;
};

union sensitive_entry {
  int            id;
  sc_method_desc *desc;
};

struct sc_sensitive_methods::data_t {
  ::std::vector<sensitive_entry> vec;
};

void sc_sensitive_methods::add(int method_id)
{
  if (data == 0)  data = new data_t;
  sensitive_entry ent;
  ent.id = method_id;
  data->vec.push_back(ent);
}

void sc_sensitive_methods::merge(sc_sensitive_methods& other)
{
  if (other.data == 0)  return;
  if (data == 0) {
    data = other.data;
    other.data = 0;
  } else {
    ::std::vector<sensitive_entry>& lhs = data->vec;
    const ::std::vector<sensitive_entry>& rhs = other.data->vec;
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
  ::std::stack<const sc_module_name*>  name_hierarchy;
  ::std::stack<const sc_module*>       module_hierarchy;
  ::std::vector<sc_cthread_desc>       cthreads;
  const sc_in<bool>                    *reset_port;
  int                                  nr_cthreads_with_reset;
  int                                  cthread_tick_index;
  bool                                 cthread_aborted;
  bool                                 stopping;
  ::std::vector<sc_method_desc>        methods, cmethods;
  ::std::vector<sc_method_desc*>       method_eval_table;
  int                                  method_eval_index;
  ::std::vector<sc_signal_update_if*>  signals;
  ::std::vector<sc_sensitive_methods*> smeths;
  sc_time                              current_time;
  static void cthread_wrapper(void *arg);
  void setup_simulation();
  void tick_cthreads(int start_index);
  void tick_cmethods();
  void update_signals();
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
  m.cthreads.push_back(desc);
}

void sc_simcontext::register_reset_port(const sc_in<bool>& port)
{
  if (m.reset_port == 0)  m.reset_port = &port;
}

void sc_simcontext::mark_cthread_as_resettable()
{
  int nc = m.cthreads.size();
  int nr = m.nr_cthreads_with_reset;
  int nr1 = nr + 1;
  m.nr_cthreads_with_reset = nr1;
  if (nr1 == nc)  return;
  sc_cthread_desc &last = m.cthreads.back();
  sc_cthread_desc &mid = m.cthreads[nr];
  sc_cthread_desc tmp = mid;  mid = last;  last = tmp;
}

void sc_simcontext::register_method(sc_module *mod, sc_entry_func func)
{
  sc_method_desc desc;
  desc.mod = mod;
  desc.func = func;
  desc.eval_order = m.methods.size();
  m.methods.push_back(desc);
}

int  sc_simcontext::current_method_id() const { return m.methods.size() - 1; }

void sc_simcontext::mark_method_as_clocked(int method_id)
{
  // assert(method_id == m.methods.size()-1);
  m.cmethods.push_back(m.methods[method_id]);
  m.methods.pop_back();
}

void sc_simcontext::register_signal(sc_signal_update_if& sig, sc_sensitive_methods& smeth)
{
  m.signals.push_back(&sig);
  m.smeths.push_back(&smeth);
}

void sc_simcontext::check_sensitive_methods(sc_sensitive_methods *sensitive_methods)
{
  ::std::vector<sensitive_entry>& methods = sensitive_methods->data->vec;
  int n = methods.size();
  int eval_index = m.method_eval_index;
  for (int i=0; i<n; ++i) {
    sc_method_desc *meth = methods[i].desc;
    int order = meth->eval_order;
    if (order >= eval_index)  continue;
    // reorder method_eval_table
    // assert(m.method_eval_table[i] == &m.methods[order]);
    ::std::vector<sc_method_desc*>& table = m.method_eval_table;
    for (int j=order; j<eval_index; ++j) {
      sc_method_desc *meth1 = table[j+1];
      table[j] = meth1;
      // assert(meth1->eval_order == i+1);
      meth1->eval_order = j;
    }
    table[eval_index] = meth;
    meth->eval_order = eval_index;
    -- eval_index;
    m.method_eval_index = eval_index;
  }
}

////////////////////////////////////////////////////////////////

void sc_simcontext::impl_t::cthread_wrapper(void *arg)
{
  int cthread_ix = reinterpret_cast<long long>(arg);  // reinterpret to long long because "void*" may not fit in "int"
  sc_cthread_desc *desc = &the_simcontext->m.cthreads[cthread_ix];
  try {
    sc_module *mod = desc->mod;
    void (sc_module::*func)() = desc->func;
    (mod->*func)();
  } catch (::std::exception& e) {
    ::std::cerr << "Unhandled exception in a cthread: " << e.what() << ".\n";
  } catch (...) {
    ::std::cerr << "Unhandled exception in a cthread.\n";
  }
  desc->mod = 0;
  the_simcontext->m.cthread_aborted = true;
  sc_cor_utils::yield(&desc->cor, &the_simcontext->m.cthreads[cthread_ix+1].cor);
}

void sc_simcontext::impl_t::setup_simulation()
{
  // initialize cthread coroutines
  {
    int n = cthreads.size();
    for (int i=0; i<n; ++i) {
      sc_cor_utils::init_thread(&cthreads[i].cor, 16*1024, cthread_wrapper, reinterpret_cast<void*>(i));
    }
  }
  // register main coroutine
  sc_cthread_desc desc;
  desc.mod = 0;
  desc.func = 0;
  cthreads.push_back(desc);
  // force current value of the reset signal to be "active(false)"
  if (reset_port != 0) {
    the_simcontext->m_signal_write_index = 0;
    reset_port->m_sig->write(false);
  }
  // initialize method_eval_table
  {
    int n = methods.size();
    method_eval_table.reserve(n);
    for (int i=0; i<n; ++i)  method_eval_table.push_back(&methods[i]);
  }
  // change sc_sensitive_methods entry from id to desc
  {
    int n = smeths.size();
    for (int i=0; i<n; ++i) {
      if (smeths[i]->data == 0)  continue;
      ::std::vector<sensitive_entry>& vec = smeths[i]->data->vec;
      int nn = vec.size();
      for (int j=0; j<nn; ++j)  vec[j].desc = &methods[vec[j].id];
    }
  }
}

void sc_simcontext::impl_t::tick_cthreads(int start_index)
{
  cthread_aborted = false;
  cthread_tick_index = start_index;
  sc_cor_utils::yield(&cthreads.back().cor, &cthreads[start_index].cor);
  if (cthread_aborted) {
    int n = cthreads.size();
    int oldix = 0;
    while (cthreads[oldix].mod != 0)  ++oldix;
    int newix = oldix;
    sc_cor_utils::destroy_thread(&cthreads[oldix].cor);
    for (++oldix; oldix<n; ++oldix) {
      if (cthreads[oldix].mod == 0) {
	sc_cor_utils::destroy_thread(&cthreads[oldix].cor);
      } else {
	cthreads[newix++] = cthreads[oldix];
      }
    }
    cthreads.resize(newix);
  }
}

void sc_simcontext::wait()
{
  sc_cor& curr_cor = m.cthreads[m.cthread_tick_index].cor;
  sc_cor& next_cor = m.cthreads[++m.cthread_tick_index].cor;
  sc_cor_utils::yield(&curr_cor, &next_cor);
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

void sc_simcontext::impl_t::update_signals()
{
  int n = signals.size();
  for (int i=0; i<n; ++i)  signals[i]->update();
}

void sc_simcontext::impl_t::eval_methods()
{
  int n = method_eval_table.size();
  method_eval_index = 0;
  while (method_eval_index < n) {
    sc_method_desc *meth = method_eval_table[method_eval_index];
    sc_module *mod = meth->mod;
    sc_entry_func func = meth->func;
    (mod->*func)();
    ++ method_eval_index;
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
  for (int i=0; i<n; ++i)  sc_cor_utils::destroy_thread(&cthreads[i].cor);
}

void sc_simcontext::scstart()
{
  m.setup_simulation();
  m.stopping = false;
  int cthread_start_index = 0;
  do {
    m_signal_write_index = 1;
    m.tick_cthreads(cthread_start_index);
    m.tick_cmethods();
    m.update_signals();
    m_signal_write_index = 0;
    m.eval_methods();
    cthread_start_index = m.check_reset_state();
    m.current_time.m_val += 10;
  } while (! m.stopping);
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

}; // namespace sc_core

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

