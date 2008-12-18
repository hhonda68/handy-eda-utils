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

namespace sc_cor_ut {

extern "C" void sc_cor_qt_wrapper(void* arg, void*, qt_userf_t* fn) {
  (*reinterpret_cast<void (*)(void*)>(fn))(arg);
}

extern "C" void* sc_cor_qt_yieldhelp(qt_t* sp, void* old_ctx, void*) {
  static_cast<sc_cor_ctx*>(old_ctx)->m_sp = sp;
  return 0;
}

void init_thread(sc_cor_desc *desc, sc_cor_ctx *ctx, int sz_stack, void (*fn)(void*), void *arg)
{
  sz_stack += QUICKTHREADS_STKALIGN*2;
  desc->m_stack = new char[sz_stack];
  qt_word_t addr_low = reinterpret_cast<qt_word_t>(desc->m_stack);
  qt_word_t addr_high = addr_low + sz_stack;
  qt_word_t align_lsbmask = QUICKTHREADS_STKALIGN - 1;
  qt_word_t align_msbmask = ~align_lsbmask;
  addr_low = (addr_low + align_lsbmask) & align_msbmask;   // ceil
  addr_high = addr_high & align_msbmask;                   // floor
  qt_word_t sz_actual = addr_high - addr_low;
  ctx->m_sp = QUICKTHREADS_SP(desc->m_stack, sz_actual);
  ctx->m_sp = QUICKTHREADS_ARGS(ctx->m_sp, arg, 0, reinterpret_cast<qt_userf_t*>(fn), sc_cor_qt_wrapper);
}

void yield(sc_cor_ctx* curr_ctx, sc_cor_ctx* next_ctx)
{
  QUICKTHREADS_BLOCK(sc_cor_qt_yieldhelp, curr_ctx, 0, next_ctx->m_sp);
}

void destroy_thread(sc_cor_desc *desc)
{
  delete[] desc->m_stack;
}

} // namespace sc_cor_ut

} // namespace sc_core

