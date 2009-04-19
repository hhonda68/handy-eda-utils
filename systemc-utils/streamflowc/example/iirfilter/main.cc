#include "filt.h"
#include "tb.h"
#include <iostream>

int main() {
  try {
    static filt_dt::coeff_t the_coeff = {
      { { 2.23801748954850749e-01,
          1.04793004180597144e-01,
          0.00000000000000000e+00,
          2.46824032729850495e+00,
          2.46824032729850495e+00,
          0.00000000000000000e+00 },
        { 5.53109465901169481e-02,
          2.31444265788747300e-01,
          1.16419411230618330e-01,
          6.09221031330263063e+00,
          1.21844206266052613e+01,
          6.09221031330263063e+00 },
        { 7.60854166202676457e-02,
          3.18373386689702720e-01,
          5.35740052613985052e-01,
          6.09221031330263063e+00,
          1.21844206266052613e+01,
          6.09221031330263063e+00 } }
    };
    Filt  dut;
    Tb    tb;
    dut(the_coeff, tb);
    tb(the_coeff, dut);
    return 0;
  } catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << ".\n";
    return 2;
  } catch (...) {
    std::cerr << "Error: unexpected exception.\n";
    return 2;
  }
}

