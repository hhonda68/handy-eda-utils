//
// StreamFlowC : A C++ coding style to describe hardware structure for stream processing
//

// Copyright (c) 2009 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

#include "streamflowc.h"
#include <vector>
#include <cstring>
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

namespace simcontext {

namespace {

struct simcontext_t {
  ::std::vector<const char *> name_hierarchy;
};

simcontext_t *the_simcontext() {
  static simcontext_t *x = new simcontext_t;  // Never deleted.  Do not mind.
  return x;
}

} // anonymous namespace

void push_modulename(const char *name) { the_simcontext()->name_hierarchy.push_back(name); }
void pop_modulename()                  { the_simcontext()->name_hierarchy.pop_back(); }
const char *get_current_basename()     { return the_simcontext()->name_hierarchy.back(); }

const char *get_current_name()
{
  ::std::vector<const char *>& vec = the_simcontext()->name_hierarchy;
  int s = vec.size();
  if (s == 0)  return "";
  int ix = (vec[0] == 0) ? 1 : 0;  // main() may directly instanciate DUT and TestBench unnamed.
  if (ix == 1 && s == 1)  return "";
  int len = 0;
  for (int i=ix; i<s; ++i) {
    len += ((vec[i] != 0) ? ::std::strlen(vec[i]) : 0) + 1;
  }
  char *ans = new char[len];	// Never deleted.  Do not mind.
  int cx = 0;
  for (int i=ix; i<s; ++i) {
    if (vec[i] != 0) {
      len = ::std::strlen(vec[i]);
      ::std::strncpy(&ans[cx], vec[i], len);
      cx += len;
    }
    ans[cx++] = '.';
  }
  // assert(cx == sizeof(ans[]));
  ans[cx-1] = '\0';  // change the last '.' to '\0'
  return ans;
}

} // namespace simcontext

} // namespace streamflowc
