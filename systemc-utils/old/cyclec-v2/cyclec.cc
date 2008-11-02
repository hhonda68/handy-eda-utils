//
// Lightweight functional digital logic simulator library.
//

// Copyright (c) 2008 the handy-eda-utils develeper(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

// Last modified on October 2005.

#include	<sstream>
#include	<typeinfo>
#include	<cstring>
#include	<cxxabi.h>
#include	"cyclec.h"

namespace CycleC {

ClockedObject::ClockedObject() {
  Circuit* parentcircuit = Circuit::the_circuit_hierarchy.top();
  set_parent(*parentcircuit);
  parentcircuit->m_clockedobjects.push_back(this);
}

std::string Object::name() const {
  if (m_parent != NULL) {
    return m_parent->name() + "." + myname();
  } else {
    return myname();
  }
}

const std::string& Object::myname() const {
  if (m_myname.empty()) {
    // default: RTTI class name
    int status = 999;
    char* full = abi::__cxa_demangle(typeid(*this).name(), NULL, 0, &status);
    if (full != NULL) {
      const char* last = std::strrchr(full, ':');
      m_myname = (last == NULL) ? full : last+1;
      free(full);		// may leak when the above line throws exception
    } else {
      m_myname = "???";
    }
  }
  return m_myname;
}

void Object::set_parent(const Object& parent) {
  if (m_parent != NULL) {
    std::ostringstream strm;
    strm << "CycleC::Object : multiple parent for " << name() << " : "
	 << m_parent->name() << " and " << parent.name();
    throw std::logic_error(strm.str());
  }
  m_parent = &parent;
}

std::stack<Circuit*> Circuit::the_circuit_hierarchy;
std::stack<Circuit::Name*> Circuit::the_name_hierarchy;

Circuit::Name::Name(const char* name) : m_str(name), m_pushed(true) {
  Circuit::the_name_hierarchy.push(this);
}
Circuit::Name::Name(const Name& name) : m_str(name.m_str), m_pushed(false) {}

Circuit::Name::~Name() {
  if (m_pushed) {
    Circuit::the_name_hierarchy.pop();
    Circuit::the_circuit_hierarchy.pop();
  }
}

Circuit::Circuit()
#ifdef CYCLEC_MULTICLOCK
 : m_clockmap(1<<0)
#endif
{
  const Name* namep = the_name_hierarchy.top();
  set_myname(namep->m_str);
  if (! the_circuit_hierarchy.empty()) {
    Circuit* parentcircuit = the_circuit_hierarchy.top();
    set_parent(*parentcircuit);
    parentcircuit->m_subcircuits.push_back(this);
  }
  the_circuit_hierarchy.push(this);
}

void Circuit::evaluate() {
#ifdef CYCLEC_MULTICLOCK
  if ((the_active_clock_map & m_clockmap) == 0)  return;
#endif
  evaluate_self();
  int n = m_subcircuits.size();
  for (int i=0; i<n; ++i)  m_subcircuits[i]->evaluate();
}

void Circuit::update() {
#ifdef CYCLEC_MULTICLOCK
  if ((the_active_clock_map & m_clockmap) == 0)  return;
#endif
  update_self();
  int ns = m_subcircuits.size();
  for (int i=0; i<ns; ++i)  m_subcircuits[i]->update();
#ifdef CYCLEC_MULTICLOCK
  // Tick all ClockedObjects only when the circuit has single clock.
  // When the circuit has multiple clocks,
  // update_self() must tick its ClockedObjects explicitly.
  if ((m_clockmap & (m_clockmap-1)) != 0)  return;
#endif
  int nc = m_clockedobjects.size();
  for (int i=0; i<nc; ++i)  m_clockedobjects[i]->tick();
}

void Circuit::reset() {
  reset_self();
  int ns = m_subcircuits.size();
  for (int i=0; i<ns; ++i)  m_subcircuits[i]->reset();
  int nc = m_clockedobjects.size();
  for (int i=0; i<nc; ++i)  m_clockedobjects[i]->reset();
}

#ifdef CYCLEC_MULTICLOCK
Circuit::clkmap_t Circuit::the_active_clock_map = (1<<0);

void Circuit::assign_clock(int clknr) {
  m_clockmap = 1<<clknr;
  int n = m_subcircuits.size();
  for (int i=0; i<n; ++i)  m_subcircuits[i]->assign_clock(clknr);
}
#endif

#ifdef CYCLEC_MULTICLOCK
TimeKeeper::TimeKeeper() : m_time(0), m_clockmap(0) {}

void TimeKeeper::define_clock(int id, unsigned int period, unsigned int start_time) {
  if (start_time == 0)  start_time = period;
  ClockInfo info = { id, period, start_time, start_time, 0 };
  m_clockinfo.push_back(info);
}

void TimeKeeper::reset() {
  int n = m_clockinfo.size();
  for (int i=0; i<n; ++i) {
    ClockInfo& info = m_clockinfo[i];
    info.rest = info.start_time;
    info.cycle = 0;
  }
  m_time = 0;
  m_clockmap = 0;
}

void TimeKeeper::advance() {
  int n = m_clockinfo.size();
  //assert(sz != 0);
  unsigned int minrest = m_clockinfo[0].rest;
  for (int i=1; i<n; ++i) {
    minrest = std::min(minrest, m_clockinfo[i].rest);
  }
  Circuit::clkmap_t  map = 0;
  for (int i=0; i<n; ++i) {
    ClockInfo& info = m_clockinfo[i];
    info.rest -= minrest;
    if (info.rest == 0) {
      ++ info.cycle;
      info.rest = info.period;
      map |= (1<<info.id);
    }
  }
  //assert(map != 0);
  m_time += minrest;
  m_clockmap = map;
}

unsigned int TimeKeeper::time() const { return m_time; }
unsigned int TimeKeeper::cycle(int clkid) const {
  int n = m_clockinfo.size();
  for (int i=0; i<n; ++i) {
    if (m_clockinfo[i].id == clkid)  return m_clockinfo[i].cycle;
  }
  throw std::logic_error("TimeKeeper::cycle(): invalid clock id");
}
unsigned int TimeKeeper::clockmap() const { return m_clockmap; }

#endif

} // namespace CycleC

