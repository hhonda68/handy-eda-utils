// -*- C++ -*-

//
// Lightweight functional digital logic simulator library.
//

// Copyright (c) 2008 the handy-eda-utils develeper(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

// Last modified on October 2005.

#include "cyclec.h"

#ifndef CYCLEC_MULTICLOCK
#error "CYCLEC_MULTICLOCK not defined"
#endif

typedef unsigned char uchar;

class GrayCodeCounter : public CycleC::Circuit {
  HAS_COMBOUTPORT(GrayCodeCounter);
public:
  InPort<bool>        countenable;
  CombOutPort<uchar>  out;
  GrayCodeCounter(Name name) : CombOut(out, calc_out) {}
private:
  Register<uchar>     m_count;
  uchar calc_out() const {
    uchar cnt = m_count.cur();
    return cnt ^ (cnt>>1);
  }
protected:
  virtual void evaluate_self() {
    m_count.nxt() = m_count.cur() + 1;
    m_count.cke() = countenable();
  }
};

class GrayToBinConverter : public CycleC::Circuit {
  HAS_COMBOUTPORT(GrayToBinConverter);
public:
  InPort<uchar>       din;
  CombOutPort<uchar>  dout;
  GrayToBinConverter(Name name) : CombOut(dout, calc_dout) {}
private:
  uchar calc_dout() const {
    uchar x = din();
    x ^= (x>>1);
    x ^= (x>>2);
    x ^= (x>>4);
    return x;
  }
};

class SampleDualClockCircuit : public CycleC::Circuit {
public:
  InPort<bool>        countenable;   // domain 1
  FwdOutPort<uchar>   dout;          // domain 2
//private:
  GrayCodeCounter     graycounter;   // domain 1
  GrayToBinConverter  graytobin;     // domain 2
  Register<uchar>     m_ff0;         // domain 1
  Register<uchar>     m_ff1;         // domain 2
  RegOutPort<uchar>   m_ff2;         // domain 2
  clkmap_t            m_clk1map, m_clk2map;
public:
  SampleDualClockCircuit(Name name)
    : graycounter("graycounter"), graytobin("graytobin") {
    graycounter.countenable.src() = countenable;
    graytobin.din.src() = m_ff2;
    dout.src() = graytobin.dout;
  }
  void assign_clocks(int clk1_id, int clk2_id) {
    m_clk1map = 1<<clk1_id;
    m_clk2map = 1<<clk2_id;
    m_clockmap = m_clk1map | m_clk2map;
    graycounter.assign_clock(clk1_id);
    graytobin.assign_clock(clk2_id);
  }
protected:
  virtual void evaluate_self() {
    if ((the_active_clock_map & m_clk1map) != 0) {
      m_ff0.nxt() = graycounter.out();
    }
    if ((the_active_clock_map & m_clk2map) != 0) {
      m_ff1.nxt() = m_ff0.cur();
      m_ff2.nxt() = m_ff1.cur();
    }
  }
  virtual void update_self() {
    if ((the_active_clock_map & m_clk1map) != 0) {
      countenable.tick();
      m_ff0.tick();
    }
    if ((the_active_clock_map & m_clk2map) != 0) {
      m_ff1.tick();
      m_ff2.tick();
      dout.tick();
    }
  }
};

class SampleDualClockBehaviorCircuit : public CycleC::Circuit {
  HAS_COMBOUTPORT(SampleDualClockBehaviorCircuit);
public:
  InPort<bool>         countenable;
  CombOutPort<uchar>   dout;
  SampleDualClockBehaviorCircuit(Name name)
    : CombOut(dout, calc_dout) {
    m_clockmap = 0x3;
    reset_self();
  }
private:
  bool                 m_enable;
  uchar                m_cnt, m_ff[3];
protected:
  virtual void evaluate_self() {
    if ((the_active_clock_map & 1) != 0)  m_enable = countenable();
  }
  virtual void update_self() {
    if ((the_active_clock_map & 2) != 0) {
      m_ff[2] = m_ff[1];
      m_ff[1] = m_ff[0];
      dout.tick();
    }
    if ((the_active_clock_map & 1) != 0) {
      m_ff[0] = m_cnt;
      if (m_enable)  m_cnt ++;
      countenable.tick();
    }
  }
  virtual void reset_self() {
    m_enable = false;
    m_cnt = 0;
    m_ff[0] = 0;
    m_ff[1] = 0;
    m_ff[2] = 0;
  }
private:
  uchar calc_dout() const { return m_ff[2]; }
};

class TestBench : public CycleC::Circuit {
public:
  SampleDualClockCircuit          dut;  /* device under test */
  SampleDualClockBehaviorCircuit  ref;  /* reference circuit */
  VarOutPort<bool>                countenable;
  unsigned int                    m_mseq;
  TestBench(Name name) : dut("dut"), ref("ref"),
                         countenable(false), m_mseq(0x10191836) {
    dut.countenable.src() = countenable;
    ref.countenable.src() = countenable;
    m_clockmap = 0x3;
    dut.assign_clocks(0, 1);
  }
protected:
  virtual void evaluate_self() {
    if ((the_active_clock_map & 2) != 0) {
      uchar actual = dut.dout();
      uchar expect = ref.dout();
      if (actual != expect)  throw std::runtime_error("verify error");
    }
  }
  virtual void update_self() {
    if ((the_active_clock_map & 1) != 0) {
      // M-sequence polynomial = x^29 + x^2 + 1
      bool rnd = ((m_mseq >> 28) ^ (m_mseq >> 1)) & 1;
      m_mseq = (m_mseq << 1) | rnd;
      countenable = rnd;
      countenable.tick();
    }
  }
};

#include <iostream>

#if 0
int main() {
  int the_time = -1;
  try {
    static TestBench tb("tb");
    std::cout << "Simulation start" << std::endl;
    for (the_time=1; the_time<3000; ++the_time) {
      CycleC::Circuit::clkmap_t map = 0;
      if ((the_time % 5) == 0)  map |= (1<<0);
      if ((the_time % 8) == 0)  map |= (1<<1);
      if (map == 0)  continue;
      CycleC::Circuit::the_active_clock_map = map;
      tb.tick();
      std::cout << "time " << the_time
                << ": " << tb.countenable()
                << " " << (int)tb.dut.graycounter.out()
                << ":" << (int)tb.dut.m_ff0.cur()
                << ":" << (int)tb.dut.m_ff1.cur()
                << ":" << (int)tb.dut.m_ff2.cur()
                << " " << (int)tb.dut.dout()
                << " " << (int)tb.ref.dout()
                << "\n";
    }
    std::cout << "Simulation end" << std::endl;
    return 0;
  } catch (std::exception& e) {
    std::cout << std::flush;
    std::cerr << "\nFatal Error at time " << the_time << ": "
              << e.what() << "." << std::endl;
    return 2;
  } catch (...) {
    std::cout << std::flush;
    std::cerr << "\nCurious exception occurred." << std::endl;
    return 2;
  }
}
#endif

#if 1
int main() {
  int the_time = -1;
  try {
    static TestBench tb("tb");
    static CycleC::TimeKeeper timekeeper;
    timekeeper.define_clock(0, 5);
    timekeeper.define_clock(1, 8);
    std::cout << "Simulation start" << std::endl;
    the_time = 0;
    do {
      std::cout << "time " << the_time
                << ": " << timekeeper.cycle(0)
                << " " << timekeeper.cycle(1)
                << " " << tb.countenable()
                << " " << (int)tb.dut.graycounter.out()
                << ":" << (int)tb.dut.m_ff0.cur()
                << ":" << (int)tb.dut.m_ff1.cur()
                << ":" << (int)tb.dut.m_ff2.cur()
                << " " << (int)tb.dut.dout()
                << " " << (int)tb.ref.dout()
                << "\n";
      timekeeper.advance();
      the_time = timekeeper.time();
      CycleC::Circuit::the_active_clock_map = timekeeper.clockmap();
      tb.tick();
    } while (the_time < 3000);
    std::cout << "Simulation end" << std::endl;
    return 0;
  } catch (std::exception& e) {
    std::cout << std::flush;
    std::cerr << "\nFatal Error at time " << the_time << ": "
              << e.what() << "." << std::endl;
    return 2;
  } catch (...) {
    std::cout << std::flush;
    std::cerr << "\nCurious exception occurred." << std::endl;
    return 2;
  }
}
#endif
