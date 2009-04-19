#include "sum8.h"
#include "tb.h"
#include <iostream>

int main() {
  try {
    Sum8<8>  dut;
    Tb       tb;
    dut(tb.d0, tb.d1, tb.d2, tb.d3, tb.d4, tb.d5, tb.d6, tb.d7);
    tb(dut);
    return 0;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << ".\n";
    return 2;
  } catch (...) {
    std::cerr << "Error: unexpected exception.\n";
    return 2;
  }
}
