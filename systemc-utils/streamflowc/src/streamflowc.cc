//
// StreamFlowC : A C++ coding style to describe hardware structure for stream processing
//

// Copyright (c) 2009 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

#include "streamflowc.h"
#include <stdexcept>

namespace streamflowc {

void portref_base::chain(portref_base *src)
{
  if (m_ptr == 0) {
    m_ptr = reinterpret_cast<__SIZE_TYPE__>(src) + 1;
  } else if ((m_ptr & 1) == 0) {
    src->assign(reinterpret_cast<void *>(m_ptr));
  } else {
    throw std::logic_error("non peer-to-peer port connection");
  }
}

void portref_base::assign(void *dst)
{
  portref_base *p = this;
  while (true) {
    __SIZE_TYPE__ ptr = p->m_ptr;
    p->m_ptr = reinterpret_cast<__SIZE_TYPE__>(dst);
    if (ptr == 0)  break;
    if ((ptr & 1) == 0)  throw std::logic_error("non peer-to-peer port connection");
    p = reinterpret_cast<portref_base*>(ptr - 1);
  }
}

} // namespace streamflowc
