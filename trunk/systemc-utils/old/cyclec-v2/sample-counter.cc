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

class AndGate : public CycleC::Circuit {
  HAS_COMBOUTPORT(AndGate);
public:
  InPort<bool>          A, B;
  CombOutPort<bool>     X;
  AndGate(Name name) : CombOut(X, calcX) {}
private:
  bool calcX() const { return A() && B(); }
};

class XorGate : public CycleC::Circuit {
  HAS_COMBOUTPORT(XorGate);
public:
  InPort<bool>          A, B;
  CombOutPort<bool>     X;
  XorGate(Name name) : CombOut(X, calcX) {}
private:
  bool calcX() const { return A() ^ B(); }
};

class Incrementer4 : public CycleC::Circuit {
  HAS_COMBOUTPORT(Incrementer4);
public:
  InPort<bool>       A0,A1,A2,A3;
  CombOutPort<bool>  Y0;
  FwdOutPort<bool>   Y1,Y2,Y3;
  Incrementer4(Name name)
    : CombOut(Y0,calc_Y0), m_and1("and1"), m_and2("and2"),
      m_xor1("xor1"), m_xor2("xor1"), m_xor3("xor1") {
    // Y1 = A0^A1
    // Y2 = (A0&A1)^A2
    // Y3 = (A0&A1&A2)^A3
	Y1.src() = m_xor1.X;
	m_xor1.A.src() = A0;
	m_xor1.B.src() = A1;
	Y2.src() = m_xor2.X;
	m_xor2.A.src() = m_and1.X;
	m_xor2.B.src() = A2;
	m_and1.A.src() = A0;
	m_and1.B.src() = A1;
	Y3.src() = m_xor3.X;
	m_xor3.A.src() = m_and2.X;
	m_xor3.B.src() = A3;
	m_and2.A.src() = m_and1.X;
	m_and2.B.src() = A2;
  }
private:
  AndGate   m_and1, m_and2;
  XorGate   m_xor1, m_xor2, m_xor3;
  bool calc_Y0() const { return !A0(); }
};

#if 1

class FlipFlop : public CycleC::Circuit {
public:
  InPort<bool>       D;
  VarOutPort<bool>   Q;
  FlipFlop(Name name) {}
protected:
  virtual void evaluate_self() { m_nextQ = D(); }
  virtual void update_self() { Q = m_nextQ; }
private:
  bool m_nextQ;
};

#else

class FlipFlop : public CycleC::Circuit {
public:
  InPort<bool>       D;
  RegOutPort<bool>   Q;
  FlipFlop(Name name) {}
protected:
  virtual void evaluate_self() { Q.nxt() = D(); }
};

#endif

class Counter4Structure : public CycleC::Circuit {
public:
  FwdOutPort<bool>  Y0,Y1,Y2,Y3;
  Counter4Structure(Name name)
    : m_ff0("ff0"), m_ff1("ff1"), m_ff2("ff2"), m_ff3("ff3"),
      m_inc("inc") {
    Y0.src() = m_ff0.Q;
    Y1.src() = m_ff1.Q;
    Y2.src() = m_ff2.Q;
    Y3.src() = m_ff3.Q;
    m_ff0.D.src() = m_inc.Y0;
    m_ff1.D.src() = m_inc.Y1;
    m_ff2.D.src() = m_inc.Y2;
    m_ff3.D.src() = m_inc.Y3;
    m_inc.A0.src() = m_ff0.Q;
    m_inc.A1.src() = m_ff1.Q;
    m_inc.A2.src() = m_ff2.Q;
    m_inc.A3.src() = m_ff3.Q;
  }
private:
  FlipFlop      m_ff0, m_ff1, m_ff2, m_ff3;
  Incrementer4  m_inc;
};

class Counter4RTL : public CycleC::Circuit {
public:
  RegOutPort<bool>   Y0,Y1,Y2,Y3;
  Counter4RTL(Name name) {}
protected:
  virtual void evaluate_self() {
    unsigned int n = (Y3()<<3) | (Y2()<<2) | (Y1()<<1) | Y0();
    unsigned int n1 = (n + 1) & 15;
    Y0.nxt() = n1 & 1;
    Y1.nxt() = (n1 >> 1) & 1;
    Y2.nxt() = (n1 >> 2) & 1;
    Y3.nxt() = n1 >> 3;
  }
};

class Counter4Behavior : public CycleC::Circuit {
public:
  VarOutPort<bool>  Y0,Y1,Y2,Y3;
  Counter4Behavior(Name name) : m_cnt(0) {}
protected:
  virtual void update_self() {
    m_cnt = (m_cnt + 1) & 15;
    Y0 = m_cnt & 1;
    Y1 = (m_cnt >> 1) & 1;
    Y2 = (m_cnt >> 2) & 1;
    Y3 = m_cnt >> 3;
  }
private:
  unsigned int  m_cnt;
};

class TestBench : public CycleC::Circuit {
public:
  TestBench(Name name)
    : m_cnts("cnts"), m_cntr("cntr"), m_cntb("cntb") {}
protected:
  virtual void evaluate_self() {
    bool s0 = m_cnts.Y0();
    bool s1 = m_cnts.Y1();
    bool s2 = m_cnts.Y2();
    bool s3 = m_cnts.Y3();
    bool r0 = m_cntr.Y0();
    bool r1 = m_cntr.Y1();
    bool r2 = m_cntr.Y2();
    bool r3 = m_cntr.Y3();
    bool b0 = m_cntb.Y0();
    bool b1 = m_cntb.Y1();
    bool b2 = m_cntb.Y2();
    bool b3 = m_cntb.Y3();
    if (! (s0 == b0 && r0 == b0
           && s1 == b1 && r1 == b1
           && s2 == b2 && r2 == b2
           && s3 == b3 && r3 == b3)) {
      throw std::runtime_error("verify error");
    }
  }
#if 0
private:
#else
public:
#endif
  Counter4Structure m_cnts;
  Counter4RTL       m_cntr;
  Counter4Behavior  m_cntb;
};

#include <iostream>

int main() {
  int the_cycle = -1;
  try {
    static TestBench tb("tb");
    std::cout << "Simulation start" << std::endl;
    for (the_cycle=0; the_cycle<100; ++the_cycle) {
#if 1
      std::cout << "cycle " << the_cycle
                << ": cnts="
                << tb.m_cnts.Y3()
                << tb.m_cnts.Y2()
                << tb.m_cnts.Y1()
                << tb.m_cnts.Y0()
                << " cntr="
                << tb.m_cntr.Y3()
                << tb.m_cntr.Y2()
                << tb.m_cntr.Y1()
                << tb.m_cntr.Y0()
                << " cntb="
                << tb.m_cntb.Y3()
                << tb.m_cntb.Y2()
                << tb.m_cntb.Y1()
                << tb.m_cntb.Y0()
                << "\n";
#endif
      tb.tick();
    }
    std::cout << "Simulation end" << std::endl;
    return 0;
  } catch (std::exception& e) {
    std::cout << std::flush;
    std::cerr << "\nFatal Error at cycle " << the_cycle << ": "
              << e.what() << "." << std::endl;
    return 2;
  } catch (...) {
    std::cout << std::flush;
    std::cerr << "\nCurious exception occurred." << std::endl;
    return 2;
  }
}

