#include "sort4.h"
#include "tb.h"
#include <iostream>

int main() {
  try {
    Sort4    dut;
    Tb       tb;
    dut(tb.a, tb.b, tb.c, tb.d);
    tb(dut.p, dut.q, dut.r, dut.s);
    return 0;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << ".\n";
    return 2;
  } catch (...) {
    std::cerr << "Error: unexpected exception.\n";
    return 2;
  }
}
