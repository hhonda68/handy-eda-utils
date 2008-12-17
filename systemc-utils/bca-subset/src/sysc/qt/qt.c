#include "copyright.h"
#include "qt.h"

#ifdef __cplusplus
extern "C"
#endif
  void
qt_error (void)
{
  extern void abort(void);

  abort();
}
