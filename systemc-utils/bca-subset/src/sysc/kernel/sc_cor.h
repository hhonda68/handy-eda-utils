// -*- c++ -*-

//
// Minimal synthesizable BCA subset of SystemC.
//

// Copyright (c) 2008 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

// sc_cor

#ifndef BCASYSC_KERNEL_SCCOR_H
#define BCASYSC_KERNEL_SCCOR_H

extern "C" struct qt_t;

namespace sc_core {

struct sc_cor_desc { char *m_stack; };  // descriptor for initialization/finalization
struct sc_cor_ctx { qt_t *m_sp; };      // context used during simulation

namespace sc_cor_ut {

void init_thread(sc_cor_desc *desc, sc_cor_ctx *ctx, int sz_stack, void (*fn)(void*), void *arg);
void yield(sc_cor_ctx *curr_ctx, sc_cor_ctx *next_ctx);
void destroy_thread(sc_cor_desc *desc);

} // namespace sc_cor_ut

} // namespace sc_core

#endif


