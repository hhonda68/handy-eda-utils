#include <systemc>
#include <iostream>
using ::sc_core::sc_in;
using ::sc_core::sc_out;
using ::sc_core::sc_signal;
using ::sc_core::sc_clock;

SC_MODULE(Dut0) {
  sc_in<bool>   clk, rst;
  sc_in<int>    idata_in;
  sc_out<int>   odata_out;
  SC_CTOR(Dut0) {
    SC_CTHREAD(entry, clk.pos());
    reset_signal_is(rst, false);
  }
 private:
  void entry();
};

void Dut0::entry()
{
  int odata = 1000;
  odata_out = odata;
  ::sc_core::wait();
  while (true) {
    int idata = idata_in.read();
    odata += 1;
    ::std::cout << ::sc_core::sc_time_stamp() << " Dut0 IN " << idata << " OUT " << odata << '\n';
    odata_out = odata;
    ::sc_core::wait();
  }
}

SC_MODULE(Dut1) {
  sc_in<bool>   clk, rst;
  sc_in<int>    idata_in;
  sc_out<int>   odata_out;
  SC_CTOR(Dut1) {
    SC_CTHREAD(entry, clk.pos());
    reset_signal_is(rst, false);
  }
 private:
  void entry();
};

void Dut1::entry()
{
  int odata = 5000;
  odata_out = odata;
  ::sc_core::wait();
  while (true) {
    int idata = idata_in.read();
    odata += 10;
    ::std::cout << ::sc_core::sc_time_stamp() << "                         Dut1 IN " << idata << " OUT " << odata << '\n';
    odata_out = odata;
    ::sc_core::wait();
  }
}

SC_MODULE(Dut) {
  sc_in<bool>      clk0, clk1, rst;
  SC_CTOR(Dut) : sub0("sub0"), sub1("sub1") {
    sub0.clk(clk0);
    sub0.rst(rst);
    sub0.idata_in(sig1to0);
    sub0.odata_out(sig0to1);
    sub1.clk(clk1);
    sub1.rst(rst);
    sub1.idata_in(sig0to1);
    sub1.odata_out(sig1to0);
  }
 private:
  Dut0  sub0;
  Dut1  sub1;
  sc_signal<int>   sig0to1, sig1to0;
};

SC_MODULE(Tb) {
  sc_in<bool>  clk;
  sc_out<bool> rst_out;
  SC_CTOR(Tb) {
    SC_CTHREAD(entry, clk.pos());
  }
 private:
  void entry();
};

void Tb::entry()
{
  rst_out = false;
  ::sc_core::wait(10);
  rst_out = true;
  ::sc_core::wait(100);
  ::sc_core::sc_stop();
  ::sc_core::wait();
}

SC_MODULE(System) {
  sc_clock clk0, clk1;
  Dut      dut;
  Tb       tb;
  sc_signal<bool>  rst_sig;
  SC_CTOR(System) : clk0("clk0", 10, ::sc_core::SC_NS), clk1("clk1", 35, ::sc_core::SC_NS), dut("dut"), tb("tb") {
    dut.clk0(clk0);
    dut.clk1(clk1);
    dut.rst(rst_sig);
    tb.clk(clk0);
    tb.rst_out(rst_sig);
  }
};

int sc_main(int argc, char *argv[])
{
  System sys("sys");
  ::sc_core::sc_start();
  return 0;
}

