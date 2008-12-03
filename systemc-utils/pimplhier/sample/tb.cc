//
// Wrapper for pimpl idiom on hierachical modules
//

// Copyright (c) 2008 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

#include "hiermod.h"
#include <iostream>
#include <stdexcept>

SC_MODULE(TestBench) {
  sc_in<bool>     clk;
  sc_out<bool>    rst;
  sc_out<data_t>  idata_out;
  sc_in<data_t>   odata_in;
  SC_CTOR(TestBench) {
    SC_CTHREAD(source, clk.pos());
    SC_CTHREAD(disp, clk.pos());
  }
 private:
  void source();
  void disp();
};

void TestBench::source() {
  rst = false;
  idata_out = 0;
  ::sc_core::wait(5);
  rst = true;
  data_t random_data = 0x5a;
  for (int i=0; i<256; ++i) {
    idata_out = random_data;
    random_data = random_data * 5 + 3;
    ::sc_core::wait();
  }
  ::sc_core::wait(5);
  ::sc_core::sc_stop();
}

namespace {

const std::string byte_to_hexstr(data_t data)
{
  static char buf[3];
  static const char hextab[] = "0123456789ABCDEF";
  buf[0] = hextab[(data>>4)&0x0f];
  buf[1] = hextab[data&0x0f];
  buf[2] = '\0';
  return std::string(buf);
}

};

void TestBench::disp() {
  int cyc = 0;
  while (true) {
    ::sc_core::wait();
    bool reset = rst.read();
    data_t idata = idata_out.read();
    data_t odata = odata_in.read();
    std::cout << cyc
	      << ": " << static_cast<int>(reset)
	      << " " << byte_to_hexstr(idata)
	      << " " << byte_to_hexstr(odata)
	      << "\n";
    ++ cyc;
  }
}

////////////////////////////////////////////////////////////////

SC_MODULE(System) {
  TestBench            *tb;
  HierMod              *dut;
  ::sc_core::sc_clock  clk;
  sc_signal<bool>      rst_sig;
  sc_signal<data_t>    idata_sig, odata_sig;
  SC_CTOR(System) : clk("clk", 10, ::sc_core::SC_NS) {
    tb = new TestBench("tb");
    dut = new HierMod("dut");
    tb->clk(clk);
    tb->rst(rst_sig);
    tb->idata_out(idata_sig);
    tb->odata_in(odata_sig);
    dut->clk(clk);
    dut->rst(rst_sig);
    dut->idata_in(idata_sig);
    dut->odata_out(odata_sig);
  }
  ~System() {
    delete tb;
    delete dut;
  }
};

////////////////////////////////////////////////////////////////

int sc_main(int argc, char *argv[]) {
  int ret;
  try {
    System sys("sys");
    ::sc_core::sc_start();
    ret = 0;
  } catch (std::exception &e) {
    std::cerr << "Error: " << e.what() << ".\n";
    ret = 2;
  } catch (...) {
    std::cerr << "Error: unexpected exception\n";
    ret = 2;
  }
  return ret;
}

