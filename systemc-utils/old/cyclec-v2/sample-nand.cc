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

class NandGate : public CycleC::Circuit {
  HAS_COMBOUTPORT(NandGate);
public:
  InPort<bool>          A, B;
  CombOutPort<bool>     X;
  NandGate(Name name) : CombOut(X, calcX) {}
private:
  bool calcX() const { return !(A() && B()); }
};

class TestJig : public CycleC::Circuit {
public:
  InPort<bool>          X;
  VarOutPort<bool>      A, B;
  unsigned int          m_rng;
  TestJig(Name name) : m_rng(0x10130945) {}
protected:
  virtual void evaluate_self() {
    bool expect = !(A() && B());
    bool actual = X();
    if (actual != expect) throw std::runtime_error("verify error");
  }
  virtual void update_self() {
    m_rng = m_rng * 1103515245 + 12345;
    A = (m_rng >> 13) & 1;
    B = (m_rng >> 23) & 1;
  }
};

class TestBench : public CycleC::Circuit {
public:
  TestBench(Name name) : m_nand("nand"), m_jig("jig") {
    m_nand.A.src() = m_jig.A;
    m_nand.B.src() = m_jig.B;
    m_jig.X.src() = m_nand.X;
  }
#if 0
private:
#endif
  NandGate      m_nand;
  TestJig       m_jig;
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
                << ": jig.out " << tb.m_jig.A()
                << " " << tb.m_jig.B()
                << " -> nand.in " << tb.m_nand.A()
                << " " << tb.m_nand.B()
                << " -> nand.out " << tb.m_nand.X()
                << " -> jig.in " << tb.m_jig.X()
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

