//
// Minimal synthesizable BCA subset of SystemC.
//

// Copyright (c) 2008 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

#include "sc_cor.h"
#include "../qt/qt.h"

namespace sc_core {

namespace sc_cor_utils {

extern "C" void sc_cor_qt_wrapper(void* arg, void*, qt_userf_t* fn) {
  (*reinterpret_cast<void (*)(void*)>(fn))(arg);
}

extern "C" void* sc_cor_qt_yieldhelp(qt_t* sp, void* old_cor, void*) {
  reinterpret_cast<sc_cor*>(old_cor)->m_sp = sp;
  return 0;
}

void init_thread(sc_cor *cor, int sz_stack, void (*fn)(void*), void *arg)
{
  sz_stack += QUICKTHREADS_STKALIGN*2;
  cor->m_stack = new char[sz_stack];
  qt_word_t addr_low = reinterpret_cast<qt_word_t>(cor->m_stack);
  qt_word_t addr_high = addr_low + sz_stack;
  qt_word_t align_lsbmask = QUICKTHREADS_STKALIGN - 1;
  qt_word_t align_msbmask = ~align_lsbmask;
  addr_low = (addr_low + align_lsbmask) & align_msbmask;   // ceil
  addr_high = addr_high & align_msbmask;                   // floor
  qt_word_t sz_actual = addr_high - addr_low;
  cor->m_sp = QUICKTHREADS_SP(cor->m_stack, sz_actual);
  cor->m_sp = QUICKTHREADS_ARGS(cor->m_sp, arg, 0, reinterpret_cast<qt_userf_t*>(fn), sc_cor_qt_wrapper);
}

void yield(sc_cor* curr_cor, sc_cor* next_cor)
{
  QUICKTHREADS_BLOCK(sc_cor_qt_yieldhelp, curr_cor, 0, next_cor->m_sp);
}

void destroy_thread(sc_cor *cor)
{
  delete[] cor->m_stack;
}

} // namespace sc_cor_utils

} // namespace sc_core

