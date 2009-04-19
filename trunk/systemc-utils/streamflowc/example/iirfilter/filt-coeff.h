#ifndef FILT_PARAM_H
#define FILT_PARAM_H

#include "subfilt-coeff.h"

namespace filt_dt {

struct coeff_t {
  subfilt_dt::coeff_t sub[3];
};

} // namespace filt_dt

#endif
