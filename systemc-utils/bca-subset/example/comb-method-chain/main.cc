#include <iostream>
#include <systemc>
using sc_core::sc_in;
using sc_core::sc_out;
using sc_core::sc_signal;
using sc_dt::sc_uint;
using sc_dt::sc_int;

////////////////////////////////////////////////////////////////

SC_MODULE(Dut) {
  sc_in<bool>      clk;
  sc_in<int>       data_in[8];
  sc_out<int>      data_out[8];
  sc_signal<int>   mid_sig[6];
  SC_CTOR(Dut) {
    SC_CTHREAD(entry, clk.pos());
    SC_METHOD(logic0);
    sensitive << mid_sig[0] << data_in[4] << mid_sig[1];
    SC_METHOD(logic1);
    sensitive << mid_sig[5] << data_out[5];
    SC_METHOD(logic2);
    sensitive << mid_sig[1] << data_in[7];
    SC_METHOD(logic3);
    sensitive << data_in[2] << data_in[3];
    SC_METHOD(logic4);
    sensitive << data_in[0] << mid_sig[2];
    SC_METHOD(logic5);
    sensitive << data_out[1] << mid_sig[3] << mid_sig[4];
    SC_METHOD(logic6);
    sensitive << data_in[1] << mid_sig[0];
    SC_METHOD(logic7);
    sensitive << data_in[5] << data_in[6];
  }
 private:
  void entry();
  void logic0();
  void logic1();
  void logic2();
  void logic3();
  void logic4();
  void logic5();
  void logic6();
  void logic7();
};

//                                  +--+
// i0 ----------------------------->|L4|----------------------> o0
//                              +-->|  |
//                     +--+     |   +--+
// i1 ---------------->|L6|--m2-+
//                 +-->|  |--------+--------------------------> o1
//        +--+     |   +--+        |
// i2 --->|L3|--m0-+               |      +--+
// i3 --->|  |     |  +--+         +----->|L5|----------------> o2
//        +--+     +->|  |---m3---------->|  |
// i4 --------------->|L0|             +->|  |--m5-+
//        +--+     +->|  |------> o3   |  +--+     |
// i5 --->|L7|--m1-+  +--+    +--+     |           |   +--+
// i6 --->|  |     +--------->|L2|--m4-+           +-->|L1|--> o4
//        +--+            +-->|  |----------+--------->|  |
// i7 --------------------+   +--+          |          +--+
//                                          |
//                                          +----------------> o5

void Dut::entry() {
  data_out[6] = 123;
  data_out[7] = 456;
  while (true) {
    wait();
    data_out[6] = data_out[6].read() + 1;
    data_out[7] = data_out[7].read() + 2;
  }
}

void Dut::logic0() {
  int a = mid_sig[0].read();
  int b = data_in[4].read();
  int c = mid_sig[1].read();
  mid_sig[3] = a + b + c;
  data_out[3] = a ^ b ^ c;
}

void Dut::logic1() {
  int a = mid_sig[5].read();
  int b = data_out[5].read();
  data_out[4] = a ^ b;
}

void Dut::logic2() {
  int a = mid_sig[1].read();
  int b = data_in[7].read();
  mid_sig[4] = a - b;
  data_out[5] = b - a;
}

void Dut::logic3() {
  int a = data_in[2].read();
  int b = data_in[3].read();
  mid_sig[0] = a * b;
}

void Dut::logic4() {
  int a = data_in[0].read();
  int b = mid_sig[2].read();
  data_out[0] = a + b;
}

void Dut::logic5() {
  int a = data_out[1].read();
  int b = mid_sig[3].read();
  int c = mid_sig[4].read();
  data_out[2] = a + b - c;
  mid_sig[5] = a - b + c;
}

void Dut::logic6() {
  int a = data_in[1].read();
  int b = mid_sig[0].read();
  mid_sig[2] = a ^ b;
  data_out[1] = ~(a ^ b);
}

void Dut::logic7() {
  int a = data_in[5].read();
  int b = data_in[6].read();
  mid_sig[1] = a ^ b;
}

////////////////////////////////////////////////////////////////

class RNG {
public:
  RNG(unsigned int seed) : m_seed(seed) {}
  int operator()() {
    int ans;
    m_seed = m_seed * 1103515245u + 12345u;
    ans = (m_seed >> 15) & 0xffff;
    m_seed = m_seed * 1103515245u + 12345u;
    return (ans << 16) | ((m_seed >> 15) & 0xffff);
  }
private:
  unsigned int m_seed;
};

SC_MODULE(Testbench) {
  sc_in<bool>  clk;
  sc_out<int>  idata_out[8];
  sc_in<int>   odata_in[8];
  SC_CTOR(Testbench) {
    SC_CTHREAD(source, clk.pos());
    SC_CTHREAD(sink, clk.pos());
  }
 private:
  void source();
  void sink();
};

void Testbench::source() {
  RNG rng(0x01234567);
  while (true) {
    for (int i=0; i<8; ++i) {
      idata_out[i] = rng();
    }
    wait();
  }
}

void Testbench::sink() {
  for (int clk=0; clk<100; ++clk) {
    wait();
    std::cout << sc_core::sc_time_stamp();
    for (int i=0; i<8; ++i) {
      std::cout << ' ' << odata_in[i].read();
    }
    std::cout << '\n';
  }
  sc_core::sc_stop();
  wait();
}

////////////////////////////////////////////////////////////////

SC_MODULE(System) {
  sc_core::sc_clock  clk;
  sc_signal<int>     idata[8], odata[8];
  Dut                dut;
  Testbench          tb;
  SC_CTOR(System) : clk("clk", 10, sc_core::SC_NS), dut("dut"), tb("tb") {
    dut.clk(clk);
    tb.clk(clk);
    for (int i=0; i<8; ++i) {
      dut.data_in[i](idata[i]);
      dut.data_out[i](odata[i]);
      tb.idata_out[i](idata[i]);
      tb.odata_in[i](odata[i]);
    }
  }
};

////////////////////////////////////////////////////////////////

int sc_main(int argc, char *argv[])
{
  try {
    System sys("sys");
    sc_core::sc_start();
    return 0;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << ".\n";
    return 2;
  } catch (...) {
    std::cerr << "Error: Curious exception.\n";
    return 2;
  }
}
