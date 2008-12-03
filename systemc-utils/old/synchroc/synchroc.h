// -*- C++ -*-

//
// Lightweight single-clock digital logic simulator library.
//

// Copyright (c) 2008 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

// Last modified on August 2004.

#ifndef SYNCHROC_H_INCLUDED
#define SYNCHROC_H_INCLUDED

#include	<vector>
#include	<algorithm>
#include	<functional>
#include	<stdexcept>
#include	<cassert>

class Circuit {
public:
  class InRange;
  class OutRange;
  Circuit(int nin, int nout);
  virtual ~Circuit();
  virtual void connect(int ix, Circuit& src, int srcox, int n=1);
  virtual unsigned int invalue(int ix, int n=1) const;
  virtual unsigned int outvalue(int ox, int n=1) const;
  virtual void evaluate();
  virtual void update();
  void tick();
  InRange in(int ix, int n=1);
  OutRange out(int ox, int n=1);

protected:
  virtual unsigned int calc_outvalue(int ox) const;
  void base_connect(int ix, Circuit& src, int srcox, int n);
  unsigned int base_invalue(int ix, int n) const;
  unsigned int base_outvalue(int ox, int n) const;
  void base_evaluate();
  void base_update();

  class BitVector {
  public:
    explicit BitVector(int len);
    int length() const;
    unsigned int range(int pos, int len) const;
    void setrange(int pos, int len, unsigned int val);
    void clear();
  private:
    const int			m_len;
    std::vector<unsigned int>	m_data;
  };
  struct ConnectionInfo {
    Circuit*	src;
    int		srcox;
    int		len;	/* number of continuous connections */
    ConnectionInfo();
  };

protected:
  const int			m_nin, m_nout;
private:
  std::vector<ConnectionInfo>	m_input;
  mutable BitVector		m_icache, m_itag;
protected:
  mutable BitVector		m_ocache, m_otag;
};

class CompoundCircuit : public Circuit {
public:
  class InRange;
  class OutRange;
  CompoundCircuit(int nin, int nout);
  virtual ~CompoundCircuit();
  virtual void connect(int ix, Circuit& src, int srcox, int n=1);
  virtual unsigned int outvalue(int ox, int n=1) const;
  virtual void evaluate();
  virtual void update();
  InRange in(int ix, int n=1);
  OutRange out(int ox, int n=1);
  void add_component(Circuit& component);
  template <typename InputIterator>
  void add_components(InputIterator first, InputIterator last);
  CompoundCircuit& operator<< (Circuit& component);
  template <typename InputIterator>
  CompoundCircuit& operator<< (std::pair<InputIterator,InputIterator> range);
  void forward_input(int ix, Circuit& dst, int dstix, int n=1);
  void forward_output(int ox, Circuit& src, int srcox, int n=1);
protected:
  void base_evaluate();
  void base_update();
private:
  typedef std::pair<Circuit*,int>	SignalLocation;
  typedef std::vector<SignalLocation>	ForwardInputInfo;
  std::vector<ForwardInputInfo>		m_fwin;
  std::vector<ConnectionInfo>		m_fwout;
  std::vector<Circuit*>			m_internalcomponents;
};

class Circuit::InRange {
  friend class Circuit::OutRange;
  friend class CompoundCircuit::InRange;
  friend class CompoundCircuit::OutRange;
public:
  InRange(Circuit& obj, int ix, int n);
  void operator= (const Circuit::OutRange& rhs);
  void operator= (const CompoundCircuit::OutRange& rhs);
  void operator= (const CompoundCircuit::InRange& rhs);
private:
  Circuit&	m_obj;
  const int	m_ix, m_n;
};

class Circuit::OutRange {
  friend class Circuit::InRange;
  friend class CompoundCircuit::InRange;
  friend class CompoundCircuit::OutRange;
public:
  OutRange(Circuit& obj, int ox, int n);
private:
  Circuit&	m_obj;
  const int	m_ox, m_n;
};

class CompoundCircuit::InRange {
  friend class Circuit::InRange;
  friend class Circuit::OutRange;
  friend class CompoundCircuit::OutRange;
public:
  InRange(CompoundCircuit& obj, int ix, int n);
  void operator= (const Circuit::OutRange& rhs);
  void operator= (const CompoundCircuit::OutRange& rhs);
  void operator= (const CompoundCircuit::InRange& rhs);
private:
  CompoundCircuit&	m_obj;
  const int		m_ix, m_n;
};

class CompoundCircuit::OutRange {
  friend class Circuit::InRange;
  friend class Circuit::OutRange;
  friend class CompoundCircuit::InRange;
public:
  OutRange(CompoundCircuit& obj, int ox, int n);
  void operator= (const Circuit::OutRange& rhs);
  void operator= (const CompoundCircuit::OutRange& rhs);
private:
  CompoundCircuit&	m_obj;
  const int		m_ox, m_n;
};


////////////////////////////////////////////////////////////////

inline Circuit::BitVector::BitVector(int len) : m_len(len), m_data((len+31)>>5) {}

inline int Circuit::BitVector::length() const { return m_len; }

inline unsigned int Circuit::BitVector::range(int pos, int len) const {
  int ix = pos >> 5;
  int sft = pos & 0x1F;
  unsigned int val;
  val = m_data[ix] >> sft;
  if (((pos+len-1) >> 5) != ix)  val |= (m_data[ix+1] << (32-sft));
  return val & (0xFFFFFFFFu >> (32-len));
}

inline void Circuit::BitVector::setrange(int pos, int len, unsigned int val) {
  int ix = pos >> 5;
  int sft = pos & 0x1F;
  if (((pos+len-1)>>5) == ix) {
    m_data[ix] = (m_data[ix] & ~((0xFFFFFFFFu>>(32-len))<<sft)) | (val << sft);
  } else {
    m_data[ix] = (m_data[ix] & (0x7FFFFFFFu>>(31-sft))) | (val << sft);
    m_data[ix+1] = (m_data[ix+1] & (0xFFFFFFFEu<<((pos+len-1)&0x1F))) | (val >> (32-sft));
  }
}

////////////////////////////////////////////////////////////////

inline Circuit::~Circuit() {}
inline void Circuit::connect(int ix, Circuit& src, int srcox, int n) {
  base_connect(ix, src, srcox, n);
}
inline unsigned int Circuit::invalue(int ix, int n) const { return base_invalue(ix, n); }
inline unsigned int Circuit::outvalue(int ox, int n) const { return base_outvalue(ox, n); }
inline void Circuit::evaluate() { base_evaluate(); }
inline void Circuit::update() { base_update(); }
inline void Circuit::tick() { evaluate(); update(); }

inline unsigned int Circuit::calc_outvalue(int ox) const { return 0; }
inline void Circuit::base_evaluate() {}
inline void Circuit::base_update() { m_itag.clear(); m_otag.clear(); }

inline Circuit::InRange Circuit::in(int ix, int n) {
  return Circuit::InRange(*this, ix, n);
}
inline Circuit::OutRange Circuit::out(int ox, int n) {
  return Circuit::OutRange(*this, ox, n);
}

inline Circuit::ConnectionInfo::ConnectionInfo() : src(NULL) {}

////////////////////////////////////////////////////////////////

inline CompoundCircuit::~CompoundCircuit() {}

template <typename InputIterator>
inline void CompoundCircuit::add_components(InputIterator first, InputIterator last) {
  for ( ; first!=last; ++first)  add_component(*first);
}

inline void CompoundCircuit::add_component(Circuit& component) {
  m_internalcomponents.push_back(&component);
}

inline CompoundCircuit& CompoundCircuit::operator<< (Circuit& component) {
  add_component(component);
  return *this;
}

inline void CompoundCircuit::evaluate() { base_evaluate(); }
inline void CompoundCircuit::update() { base_update(); }

inline void CompoundCircuit::base_evaluate() {
  Circuit::base_evaluate();
  std::for_each(m_internalcomponents.begin(), m_internalcomponents.end(),
		std::mem_fun(&Circuit::evaluate));
}

inline void CompoundCircuit::base_update() {
  Circuit::base_update();
  std::for_each(m_internalcomponents.begin(), m_internalcomponents.end(),
		std::mem_fun(&Circuit::update));
}

inline CompoundCircuit::InRange CompoundCircuit::in(int ix, int n) {
  return CompoundCircuit::InRange(*this, ix, n);
}
inline CompoundCircuit::OutRange CompoundCircuit::out(int ox, int n) {
  return CompoundCircuit::OutRange(*this, ox, n);
}

////////////////////////////////////////////////////////////////

inline Circuit::InRange::InRange(Circuit& obj, int ix, int n)
  : m_obj(obj), m_ix(ix), m_n(n) {}
inline Circuit::OutRange::OutRange(Circuit& obj, int ox, int n)
  : m_obj(obj), m_ox(ox), m_n(n) {}

inline CompoundCircuit::InRange::InRange(CompoundCircuit& obj, int ix, int n)
  : m_obj(obj), m_ix(ix), m_n(n) {}
inline CompoundCircuit::OutRange::OutRange(CompoundCircuit& obj, int ox, int n)
  : m_obj(obj), m_ox(ox), m_n(n) {}

inline void Circuit::InRange::operator= (const Circuit::OutRange& rhs) {
  assert(m_n == rhs.m_n);
  m_obj.connect(m_ix, rhs.m_obj, rhs.m_ox, rhs.m_n);
}
inline void Circuit::InRange::operator= (const CompoundCircuit::OutRange& rhs) {
  assert(m_n == rhs.m_n);
  m_obj.connect(m_ix, rhs.m_obj, rhs.m_ox, rhs.m_n);
}
inline void Circuit::InRange::operator= (const CompoundCircuit::InRange& rhs) {
  assert(m_n == rhs.m_n);
  rhs.m_obj.forward_input(rhs.m_ix, m_obj, m_ix, m_n);
}

inline void CompoundCircuit::InRange::operator= (const Circuit::OutRange& rhs) {
  assert(m_n == rhs.m_n);
  m_obj.connect(m_ix, rhs.m_obj, rhs.m_ox, rhs.m_n);
}
inline void CompoundCircuit::InRange::operator= (const CompoundCircuit::OutRange& rhs) {
  assert(m_n == rhs.m_n);
  m_obj.connect(m_ix, rhs.m_obj, rhs.m_ox, rhs.m_n);
}
inline void CompoundCircuit::InRange::operator= (const CompoundCircuit::InRange& rhs) {
  assert(m_n == rhs.m_n);
  rhs.m_obj.forward_input(rhs.m_ix, m_obj, m_ix, m_n);
}

inline void CompoundCircuit::OutRange::operator= (const Circuit::OutRange& rhs) {
  assert(m_n == rhs.m_n);
  m_obj.forward_output(m_ox, rhs.m_obj, rhs.m_ox, m_n);
}
inline void CompoundCircuit::OutRange::operator= (const CompoundCircuit::OutRange& rhs) {
  assert(m_n == rhs.m_n);
  m_obj.forward_output(m_ox, rhs.m_obj, rhs.m_ox, m_n);
}

#endif

