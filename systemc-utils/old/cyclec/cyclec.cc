//
// Lightweight functional digital logic simulator library.
//

// Copyright (c) 2008 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

// Last modified on October 2004.

#include	<iostream>
#include	<sstream>
#include	<typeinfo>
#include	<cstring>
#include	<cxxabi.h>
#include	"cyclec.h"

namespace CycleC {

  void Circuit::BitVector::clear() throw() {
    int n=m_data.size();
    for (int i=0; i<n; ++i)  m_data[i] = 0;
  }

  inline Circuit::ConnectionInfo::ConnectionInfo() throw() : src(NULL) {}

  Circuit::Circuit(int nr_input, int nr_output) throw(std::bad_alloc)
    : m_nr_input(nr_input), m_nr_output(nr_output), m_input(nr_input),
      m_icache(nr_input), m_itag(nr_input),
      m_ocache(nr_output), m_otag(nr_output), m_orecur(nr_output), m_parent(NULL) {
#if 0  // automatically initialized with default constructor uint() ?
    m_itag.clear();
    m_otag.clear();
    m_orecur.clear();
#endif
  }

  std::string Circuit::name() const {
    if (m_parent != NULL) {
      return m_parent->name() + "." + myname();
    } else {
      return myname();
    }
  }

  std::string Circuit::myname() const {
    // default: RTTI class name
    int status = 999;
    char* full = abi::__cxa_demangle(typeid(*this).name(), NULL, 0, &status);
    if (full != NULL) {
      const char* last = std::strrchr(full, ':');
      std::string ans((last == NULL) ? full : last+1);
      free(full);		// may leak when ans.ctor throws exception
      return ans;
    } else {
      return "???";
    }
  }

  void Circuit::connect(int ix, const Circuit& srcref, int srcox, int n)
    throw(connection_error)
  {
    int len;
    const Circuit* src = &srcref;
    if (ix+n < m_nr_input && m_input[ix+n].src == src && m_input[ix+n].srcox == srcox+n) {
      len = n + m_input[ix+n].len;
    } else {
      len = n;
    }
    for (int i=0; i<n; ++i) {
      ConnectionInfo& ent = m_input[ix+i];
      if (ent.src != NULL) {
	std::ostringstream strm;
	strm << "multiple connection for " << name() << ".in(" << ix+i << ") : "
	     << ent.src->name() << ".out(" << ent.srcox << ") and "
	     << src->name() << ".out(" << srcox+i << ")";
	throw connection_error(strm.str());
      }
      ent.src = src;
      ent.srcox = srcox+i;
      ent.len = len - i;
    }
    for (int i=1; i<=ix; ++ix) {
      ConnectionInfo& ent = m_input[ix-i];
      if (! (ent.src == src && ent.srcox == srcox-i))  break;
      ent.len = len + i;
    }
  }

  void Circuit::confirm_connection_sanity() const throw(connection_error) {
    for (int i=0; i<m_nr_input; ++i) {
      const ConnectionInfo& ent = m_input[i];
      if (ent.src == NULL) {
	std::ostringstream strm;
	strm << "no connection for " << name() << ".in(" << i << ")";
	throw connection_error(strm.str());
      }
    }
  }

  uint Circuit::invalue(int ix, int n) const {
    uint value, tag, mask;
    int	lo, len;
    value = m_icache.range(ix, n);
    tag = m_itag.range(ix, n);
    mask = 0xFFFFFFFFu >> (32-n);
    if (tag == mask)  return value;
    if (n == 1) {
      value = m_input[ix].src->outvalue(m_input[ix].srcox);
    } else {
      value &= tag;
      lo = 0;
      do {
	while (((tag >> lo) & 1) != 0)  ++lo;
	if (lo >= n)  break;
	len = m_input[ix+lo].len;
	if (len > n-lo)   len = n-lo;
	value |= m_input[ix+lo].src->outvalue(m_input[ix+lo].srcox, len) << lo;
	lo += len;
      } while (lo < n);
    }
    m_icache.setrange(ix, n, value);
    m_itag.setrange(ix, n, mask);
    return value;
  }

  uint Circuit::calc_outvalue_with_loopcheck(int ox) const {
    if (m_orecur.range(ox, 1) != 0) {
      std::ostringstream strm;
      strm << "infinite recursion in " << name() << ".calc_outvalue(" << ox << ")";
      throw infinite_recursion_error(strm.str());
    }
    m_orecur.setrange(ox, 1, 1);
    uint value = calc_outvalue(ox);
    m_orecur.setrange(ox, 1, 0);
    return value;
  }

  uint Circuit::outvalue(int ox, int n) const {
    uint value, tag, mask;
    value = m_ocache.range(ox, n);
    tag = m_otag.range(ox, n);
    mask = 0xFFFFFFFFu >> (32-n);
    if (tag == mask)  return value;
    if (n == 1) {
      value = calc_outvalue_with_loopcheck(ox);
    } else {
      value &= tag;
      for (int i=0; i<n; ++i) {
	if (((tag >> i) & 1) == 0) {
	  value |= (calc_outvalue_with_loopcheck(ox+i) << i);
	}
      }
    }
    m_ocache.setrange(ox, n, value);
    m_otag.setrange(ox, n, mask);
    return value;
  }

  void Circuit::set_parent(const Circuit& parent) throw(connection_error) {
    if (m_parent != NULL) {
      std::ostringstream strm;
      strm << "multiple parent for " << name() << " : "
	   << m_parent->name() << " and " << parent.name();
      throw connection_error(strm.str());
    }
    m_parent = &parent;
  }

  CompoundCircuit::CompoundCircuit(int nr_input, int nr_output) throw(std::bad_alloc)
    : Circuit(nr_input, nr_output), m_fwin(nr_input), m_fwout(nr_output) {}


  void CompoundCircuit::forward_input(int ix, Circuit& dst, int dstix, int n)
    throw(std::bad_alloc)
  {
    for (int i=0; i<n; ++i) {
      m_fwin[ix+i].push_back(SignalLocation(&dst, dstix+i));
    }
  }

  void CompoundCircuit::forward_output(int ox, const Circuit& srcref, int srcox, int n)
    throw(connection_error)
  {
    int len;
    const Circuit* src = &srcref;
    if (ox+n < m_nr_output && m_fwout[ox+n].src == src && m_fwout[ox+n].srcox == srcox+n) {
      len = n + m_fwout[ox+n].len;
    } else {
      len = n;
    }
    for (int i=0; i<n; ++i) {
      ConnectionInfo& ent = m_fwout[ox+i];
      if (ent.src != NULL) {
	std::ostringstream strm;
	strm << "multiple connection for " << name() << ".out(" << ox+i << ") : "
	     << ent.src->name() << ".out(" << ent.srcox << ") and "
	     << src->name() << ".out(" << srcox+i << ")";
	throw connection_error(strm.str());
      }
      ent.src = src;
      ent.srcox = srcox+i;
      ent.len = len - i;
    }
    for (int i=1; i<=ox; ++ox) {
      ConnectionInfo& ent = m_fwout[ox-i];
      if (! (ent.src == src && ent.srcox == srcox-i))  break;
      ent.len = len + i;
    }
  }

  void CompoundCircuit::connect(int ix, const Circuit& src, int srcox, int n)
    throw(connection_error)
  {
    Circuit::connect(ix, src, srcox, n);
    for (int i=n-1; i>=0; --i) {
      ForwardInputInfo& dstlist(m_fwin[ix+i]);
      ForwardInputInfo::iterator p;
      for (p=dstlist.begin(); p!=dstlist.end(); ++p) {
	Circuit *dst = p->first;
	int dstix = p->second;
	dst->connect(dstix, src, srcox+i);
      }
    }
  }

  void CompoundCircuit::confirm_connection_sanity() const throw(connection_error) {
    Circuit::confirm_connection_sanity();
    int n = m_internalcomponents.size();
    for (int i=0; i<n; ++i)  m_internalcomponents[i]->confirm_connection_sanity();
  }

  uint CompoundCircuit::outvalue(int ox, int n) const {
    uint value, tag, mask;
    int	lo, len;
    value = m_ocache.range(ox, n);
    tag = m_otag.range(ox, n);
    mask = 0xFFFFFFFFu >> (32-n);
    if (tag == mask)  return value;
    if (n == 1) {
      const Circuit *src = m_fwout[ox].src;
      if (src == NULL) {
	value = calc_outvalue_with_loopcheck(ox);
      } else {
	value = src->outvalue(m_fwout[ox].srcox);
      }
    } else {
      value &= tag;
      lo = 0;
      do {
	const Circuit *src = NULL;
	do {
	  if (((tag >> lo) & 1) != 0) {
	    // already cached.  do nothing
	  } else {
	    src = m_fwout[ox+lo].src;
	    if (src != NULL)  break;
	    value |= (calc_outvalue_with_loopcheck(ox+lo) << lo);
	  }
	  ++ lo;
	} while (lo < n);
	if (lo >= n)  break;
	len = m_fwout[ox+lo].len;
	if (len > n-lo)  len = n-lo;
	value |= (src->outvalue(m_fwout[ox+lo].srcox, len) << lo);
	lo += len;
      } while (lo < n);
    }
    m_ocache.setrange(ox, n, value);
    m_otag.setrange(ox, n, mask);
    return value;
  }

  class PinRange {
  public:
    const Circuit&	m_obj;
    int			m_pin;
    int			m_n;
    bool		m_output;
    PinRange(const Circuit::InRange& ref)
      : m_obj(ref.m_obj), m_pin(ref.m_ix), m_n(ref.m_n), m_output(false) {}
    PinRange(const Circuit::OutRange& ref)
      : m_obj(ref.m_obj), m_pin(ref.m_ox), m_n(ref.m_n), m_output(true) {}
    PinRange(const CompoundCircuit::InRange& ref)
      : m_obj(ref.m_obj), m_pin(ref.m_ix), m_n(ref.m_n), m_output(false) {}
    PinRange(const CompoundCircuit::OutRange& ref)
      : m_obj(ref.m_obj), m_pin(ref.m_ox), m_n(ref.m_n), m_output(true) {}
    static void mismatch(const PinRange& dst, const PinRange& src) throw (connection_error);
  };

  void PinRange::mismatch(const PinRange& dst, const PinRange& src) throw (connection_error) {
    std::ostringstream strm;
    strm << "attempt to connect signals with different width : "
	 << dst.m_obj.name() << "." << (dst.m_output ? "out(" : "in(")
	 << dst.m_pin << "," << dst.m_n << ") = "
	 << src.m_obj.name() << "." << (src.m_output ? "out(" : "in(")
	 << src.m_pin << "," << src.m_n << ")";
    throw connection_error(strm.str());
  }

  void Circuit::InRange::operator=(const Circuit::OutRange& rhs)
    throw(connection_error)
  {
    if (m_n != rhs.m_n)  PinRange::mismatch(*this, rhs);
    m_obj.connect(m_ix, rhs.m_obj, rhs.m_ox, m_n);
  }

  void Circuit::InRange::operator=(const CompoundCircuit::OutRange& rhs)
    throw(connection_error)
  {
    if (m_n != rhs.m_n)  PinRange::mismatch(*this, rhs);
    m_obj.connect(m_ix, rhs.m_obj, rhs.m_ox, m_n);
  }

  void Circuit::InRange::operator=(const CompoundCircuit::InRange& rhs)
    throw(std::bad_alloc, connection_error)
  {
    if (m_n != rhs.m_n)  PinRange::mismatch(*this, rhs);
    rhs.m_obj.forward_input(rhs.m_ix, m_obj, m_ix, m_n);
  }

  void CompoundCircuit::InRange::operator=(const Circuit::OutRange& rhs)
    throw(connection_error)
  {
    if (m_n != rhs.m_n)  PinRange::mismatch(*this, rhs);
    m_obj.connect(m_ix, rhs.m_obj, rhs.m_ox, m_n);
  }

  void CompoundCircuit::InRange::operator=(const CompoundCircuit::OutRange& rhs)
    throw(connection_error)
  {
    if (m_n != rhs.m_n)  PinRange::mismatch(*this, rhs);
    m_obj.connect(m_ix, rhs.m_obj, rhs.m_ox, m_n);
  }

  void CompoundCircuit::InRange::operator=(const CompoundCircuit::InRange& rhs)
    throw(std::bad_alloc, connection_error)
  {
    if (m_n != rhs.m_n)  PinRange::mismatch(*this, rhs);
    rhs.m_obj.forward_input(rhs.m_ix, m_obj, m_ix, m_n);
  }

  void CompoundCircuit::OutRange::operator=(const Circuit::OutRange& rhs)
    throw(connection_error)
  {
    if (m_n != rhs.m_n)  PinRange::mismatch(*this, rhs);
    m_obj.forward_output(m_ox, rhs.m_obj, rhs.m_ox, m_n);
  }

  void CompoundCircuit::OutRange::operator=(const CompoundCircuit::OutRange& rhs)
    throw(connection_error)
  {
    if (m_n != rhs.m_n)  PinRange::mismatch(*this, rhs);
    m_obj.forward_output(m_ox, rhs.m_obj, rhs.m_ox, m_n);
  }

  connection_error::connection_error(const std::string& arg)
    : std::logic_error(arg) {}

  infinite_recursion_error::infinite_recursion_error(const std::string& arg)
    : std::logic_error(arg) {}

}

