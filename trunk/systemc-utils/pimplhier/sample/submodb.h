//
// Wrapper for pimpl idiom on hierachical modules
//

// Copyright (c) 2008 the handy-eda-utils develeper(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

#ifndef SUBMODB_H
#define SUBMODB_H

#include "common.h"

SC_MODULE(SubModB) {
  sc_in<bool>     clk, rst;
  sc_in<data_t>   idata_in;
  sc_out<data_t>  odata_out;
  SC_CTOR(SubModB) {
    SC_CTHREAD(entry, clk.pos());
    reset_signal_is(rst, false);
  }
 private:
  void entry() {
    odata_out = 0;
    ::sc_core::wait();
    while (true) {
      data_t data = idata_in.read();
      odata_out = (data.range(3,0), data.range(7,4));
      ::sc_core::wait();
    }
  }
};

#endif
