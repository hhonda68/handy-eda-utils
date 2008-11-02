//
// Lightweight single-clock digital logic simulator library.
//

// Copyright (c) 2008 the handy-eda-utils develeper(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

// Last modified on August 2004.

#include "synchroc.h"

void Circuit::BitVector::clear() {
  std::fill(m_data.begin(), m_data.end(), 0);
}

Circuit::Circuit(int nin, int nout)
  : m_nin(nin), m_nout(nout), m_input(nin),
    m_icache(nin), m_itag(nin), m_ocache(nout), m_otag(nout) {
#if 0  // automatically initialized with default constructor uint() ?
  m_itag.clear();
  m_otag.clear();
#endif
}

void Circuit::base_connect(int ix, Circuit& srcref, int srcox, int n) {
  int len;
  Circuit* src = &srcref;
  if (ix+n < m_nin && m_input[ix+n].src == src && m_input[ix+n].srcox == srcox+n) {
    len = n + m_input[ix+n].len;
  } else {
    len = n;
  }
  for (int i=0; i<n; ++i) {
    m_input[ix+i].src = src;
    m_input[ix+i].srcox = srcox+i;
    m_input[ix+i].len = len - i;
  }
  for (int i=1; i<=ix; ++ix) {
    if (! (m_input[ix-i].src == src && m_input[ix-i].srcox == srcox-i))  break;
    m_input[ix-i].len = len + i;
  }
}

unsigned int Circuit::base_invalue(int ix, int n) const {
  unsigned int value, tag, mask;
  int	lo, len;
  value = m_icache.range(ix, n);
  tag = m_itag.range(ix, n);
  mask = 0xFFFFFFFFu >> (32-n);
  if (tag == mask)  return value;
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
  m_icache.setrange(ix, n, value);
  m_itag.setrange(ix, n, mask);
  return value;
}

unsigned int Circuit::base_outvalue(int ox, int n) const {
  unsigned int value, tag, mask;
  value = m_ocache.range(ox, n);
  tag = m_otag.range(ox, n);
  mask = 0xFFFFFFFFu >> (32-n);
  if (tag == mask)  return value;
  value &= tag;
  for (int i=0; i<n; ++i) {
    if (((tag >> i) & 1) == 0)  value |= (calc_outvalue(ox+i) << i);
  }
  m_ocache.setrange(ox, n, value);
  m_otag.setrange(ox, n, mask);
  return value;
}

CompoundCircuit::CompoundCircuit(int nin, int nout)
  : Circuit(nin, nout), m_fwin(nin), m_fwout(nout) {}

void CompoundCircuit::forward_input(int ix, Circuit& dst, int dstix, int n) {
  for (int i=0; i<n; ++i) {
    m_fwin[ix+i].push_back(SignalLocation(&dst, dstix+i));
  }
}

void CompoundCircuit::forward_output(int ox, Circuit& srcref, int srcox, int n) {
  int len;
  Circuit* src = &srcref;
  if (ox+n < m_nout && m_fwout[ox+n].src == src && m_fwout[ox+n].srcox == srcox+n) {
    len = n + m_fwout[ox+n].len;
  } else {
    len = n;
  }
  for (int i=0; i<n; ++i) {
    m_fwout[ox+i].src = src;
    m_fwout[ox+i].srcox = srcox+i;
    m_fwout[ox+i].len = len - i;
  }
  for (int i=1; i<=ox; ++ox) {
    if (! (m_fwout[ox-i].src == src && m_fwout[ox-i].srcox == srcox-i))  break;
    m_fwout[ox-i].len = len + i;
  }
}

void CompoundCircuit::connect(int ix, Circuit& src, int srcox, int n) {
  Circuit::base_connect(ix, src, srcox, n);
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

unsigned int CompoundCircuit::outvalue(int ox, int n) const {
  unsigned int value, tag, mask;
  int	lo, len;
  value = m_ocache.range(ox, n);
  tag = m_otag.range(ox, n);
  mask = 0xFFFFFFFFu >> (32-n);
  if (tag == mask)  return value;
  value &= tag;
  lo = 0;
  do {
    Circuit *src = NULL;
    do {
      if (((tag >> lo) & 1) != 0) {
	// already cached.  do nothing
      } else {
	src = m_fwout[ox+lo].src;
	if (src != NULL)  break;
	value |= (calc_outvalue(ox+lo) << lo);
      }
      ++ lo;
    } while (lo < n);
    if (lo >= n)  break;
    len = m_fwout[ox+lo].len;
    if (len > n-lo)  len = n-lo;
    value |= (src->outvalue(m_fwout[ox+lo].srcox, len) << lo);
    lo += len;
  } while (lo < n);
  m_ocache.setrange(ox, n, value);
  m_otag.setrange(ox, n, mask);
  return value;
}

