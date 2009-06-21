#include "system.h"
#include <iostream>

int main(int argc, char *argv[]) {
  try {
    System   sys;
    sys(argc, argv);
    return 0;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << ".\n";
    return 2;
  } catch (...) {
    std::cerr << "Error: unexpected exception.\n";
    return 2;
  }
}
