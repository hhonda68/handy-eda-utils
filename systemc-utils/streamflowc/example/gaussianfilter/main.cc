#include "filt2d.h"
#include "tb.h"
#include <iostream>

int main() {
  try {
    Filt2D   dut;
    Tb       tb;
    const int WIDTH = 12;
    const int HEIGHT = 8;
    dut(WIDTH, HEIGHT, tb);
    tb(WIDTH, HEIGHT, dut);
    return 0;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << ".\n";
    return 2;
  } catch (...) {
    std::cerr << "Error: unexpected exception.\n";
    return 2;
  }
}

