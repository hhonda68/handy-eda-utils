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

struct sc_cor {
  char *m_stack;
  qt_t *m_sp;
};

namespace sc_cor_utils {

void init_thread(sc_cor *cor, int sz_stack, void (*fn)(void*), void *arg);
void yield(sc_cor *curr_cor, sc_cor *next_cor);
void destroy_thread(sc_cor *cor);

}; // namespace sc_cor_utils

}; // namespace sc_core

#endif


