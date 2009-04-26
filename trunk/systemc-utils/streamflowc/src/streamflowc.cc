//
// StreamFlowC : A C++ coding style to describe hardware structure for stream processing
//

// Copyright (c) 2009 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

#include "streamflowc.h"
#include <vector>
#include <string>
#include <cstring>
#include <stdexcept>

namespace streamflowc {

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

namespace {

void throw_binding_error() __attribute__((__noreturn__));

void throw_binding_error()
{
  if (simcontext::the_simcontext()->name_hierarchy.empty()) {
    throw ::std::logic_error("non peer-to-peer port binding");
  } else {
    const char *instance = simcontext::get_current_name();
    if (instance[0] == '\0') {
      throw ::std::logic_error("non peer-to-peer port binding about the top module");
    } else {
      throw ::std::logic_error(::std::string("non peer-to-peer port binding about ") + instance);
    }
  }
  /*NOTREACHED*/
}

} // anonymous namespace

// portref_base.m_ptr management
//
// value            state           description
// -------------------------------------------------------------------------------------
// 00...0000         this           initialized
// ss...ss01   src<--this           points to source portref_base
// 00...0010         this<--dst     pointed to by someone
// ss...ss11   src<--this<--dst     points to source portref_base, and pointed to by someone
//
// pp...pp00         port           points to the established destination port
// pp...pp10    forwarded_port      forwarded the established destination port to someone

void portref_base::chain(portref_base *src)
{
  if (m_ptr == 0 || m_ptr == 2) {
    m_ptr += reinterpret_cast<__SIZE_TYPE__>(src) + 1;
    __SIZE_TYPE__ sptr = src->m_ptr;
    if (((sptr & 2) != 0) || (sptr != 0 && (sptr & 3) == 0))  throw_binding_error();
    src->m_ptr += 2;
  } else if ((m_ptr & 3) == 0) {
    src->assign(reinterpret_cast<void *>(m_ptr));
    m_ptr += 2;
  } else {
    throw_binding_error();
  }
}

void portref_base::assign(void *dst)
{
  portref_base *p = this;
  __SIZE_TYPE__ ptr = p->m_ptr;
  if (ptr == 0) {
    p->m_ptr = reinterpret_cast<__SIZE_TYPE__>(dst);
  } else if ((ptr & 3) == 1) {
    p->m_ptr = reinterpret_cast<__SIZE_TYPE__>(dst) + 2;
    p = reinterpret_cast<portref_base*>(ptr - 1);
    while (true) {
      ptr = p->m_ptr;
      if (ptr == 2) {
        p->m_ptr = reinterpret_cast<__SIZE_TYPE__>(dst);
        break;
      } else if ((ptr & 3) == 3) {
        p->m_ptr = reinterpret_cast<__SIZE_TYPE__>(dst) + 2;
        p = reinterpret_cast<portref_base*>(ptr - 3);
      } else {
        throw ::std::logic_error("internal error in streamflowc::potref_base::assign()");
      }
    }
  } else {
    throw_binding_error();
  }
}

} // namespace streamflowc
