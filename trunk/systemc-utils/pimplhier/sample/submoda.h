//
// Wrapper for pimpl idiom on hierachical modules
//

// Copyright (c) 2008 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

#ifndef SUBMODA_H
#define SUBMODA_H

#include "common.h"

SC_MODULE(SubModA) {
  sc_in<bool>     clk, rst;
  sc_in<data_t>   idata_in;
  sc_out<data_t>  odata_out;
  SC_CTOR(SubModA) {
    SC_CTHREAD(entry, clk.pos());
    reset_signal_is(rst, false);
  }
 private:
  void entry() {
    odata_out = 0;
    ::sc_core::wait();
    while (true) {
      odata_out = idata_in.read() + 1;
      ::sc_core::wait();
    }
  }
};

#endif
