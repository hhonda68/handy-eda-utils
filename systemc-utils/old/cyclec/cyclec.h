// -*- C++ -*-

//
// Lightweight functional digital logic simulator library.
//

// Copyright (c) 2008 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

// Last modified on October 2004.

#ifndef CYCLEC_H_INCLUDED
#define CYCLEC_H_INCLUDED

#include	<vector>
#include	<string>
#include	<stdexcept>
#include	<new>

namespace CycleC {

  typedef unsigned int uint;

  class connection_error : public std::logic_error {
  public: connection_error(const std::string& arg);
  };
  class infinite_recursion_error : public std::logic_error {
  public: infinite_recursion_error(const std::string& arg);
  };

  class Circuit {
  public:
    class InRange;
    class OutRange;
    Circuit(int nr_input, int nr_output) throw(std::bad_alloc);
    virtual ~Circuit() throw();
    virtual void connect(int ix, const Circuit& src, int srcox, int n=1)
      throw(connection_error);
    virtual void confirm_connection_sanity() const throw(connection_error);
    virtual uint invalue(int ix, int n=1) const;
    virtual uint outvalue(int ox, int n=1) const;
    virtual void evaluate();
    virtual void update();
    virtual void reset();
    void tick();
    InRange in(int ix, int n=1) throw();
    OutRange out(int ox, int n=1) throw();
    std::string name() const;
    const Circuit& parent() const throw();
    void set_parent(const Circuit& parent) throw(connection_error);

  protected:
    virtual uint calc_outvalue(int ox) const;
    virtual std::string myname() const;
    uint calc_outvalue_with_loopcheck(int ox) const;

    class BitVector {
    public:
      explicit BitVector(int len) throw(std::bad_alloc);
      int length() const throw();
      uint range(int pos, int len) const throw();
      void setrange(int pos, int len, uint val) throw();
      void clear() throw();
    private:
      const int		m_len;
      std::vector<uint>	m_data;
    };
    struct ConnectionInfo {
      const Circuit*	src;
      int		srcox;
      int		len;	/* number of continuous connections */
      ConnectionInfo() throw();
    };

  protected:
    const int			m_nr_input, m_nr_output;
  private:
    std::vector<ConnectionInfo>	m_input;
    mutable BitVector		m_icache, m_itag;
  protected:
    mutable BitVector		m_ocache, m_otag, m_orecur;
  private:
    const Circuit*		m_parent;

  private:
    // disabled
    Circuit(const Circuit&);
    Circuit& operator=(const Circuit&);
  };

  class CompoundCircuit : public Circuit {
  public:
    class InRange;
    class OutRange;
    CompoundCircuit(int nr_input, int nr_output) throw(std::bad_alloc);
    virtual ~CompoundCircuit() throw();
    virtual void connect(int ix, const Circuit& src, int srcox, int n=1)
      throw(connection_error);
    virtual void confirm_connection_sanity() const throw(connection_error);
    virtual uint outvalue(int ox, int n=1) const;
    virtual void evaluate();
    virtual void update();
    virtual void reset();
    InRange in(int ix, int n=1) throw();
    OutRange out(int ox, int n=1) throw();
    void add_component(Circuit& component) throw(std::bad_alloc, connection_error);
    template <typename Iter>
    void add_components(Iter first, Iter last) throw(std::bad_alloc, connection_error);
    CompoundCircuit& operator<<(Circuit& component) throw(std::bad_alloc, connection_error);
    template <typename Iter>
    struct Components : public std::pair<Iter,Iter> {
      Components(Iter first, Iter last) : std::pair<Iter,Iter>(first, last) {}
    };
    template <typename Iter>
    CompoundCircuit& operator<<(const Components<Iter>& range)
      throw(std::bad_alloc, connection_error);
    void forward_input(int ix, Circuit& dst, int dstix, int n=1) throw(std::bad_alloc);
    void forward_output(int ox, const Circuit& src, int srcox, int n=1)
      throw(connection_error);
  private:
    typedef std::pair<Circuit*,int>	SignalLocation;
    typedef std::vector<SignalLocation>	ForwardInputInfo;
    std::vector<ForwardInputInfo>	m_fwin;
    std::vector<ConnectionInfo>		m_fwout;
    std::vector<Circuit*>		m_internalcomponents;
    // disabled
    CompoundCircuit(const CompoundCircuit&);
    CompoundCircuit& operator=(const CompoundCircuit&);
  };

  class PinRange;

  class Circuit::InRange {
    friend class PinRange;
  public:
    InRange(Circuit& obj, int ix, int n) throw();
    void operator=(const Circuit::OutRange& rhs) throw(connection_error);
    void operator=(const CompoundCircuit::OutRange& rhs) throw(connection_error);
    void operator=(const CompoundCircuit::InRange& rhs)
      throw(std::bad_alloc, connection_error);
  private:
    Circuit&	m_obj;
    const int	m_ix, m_n;
    // disabled
    InRange& operator=(const InRange&);
  };

  class Circuit::OutRange {
    friend class PinRange;
    friend class Circuit::InRange;
    friend class CompoundCircuit::InRange;
    friend class CompoundCircuit::OutRange;
  public:
    OutRange(Circuit& obj, int ox, int n) throw();
  private:
    Circuit&	m_obj;
    const int	m_ox, m_n;
    // disabled
    OutRange& operator=(const OutRange&);
  };

  class CompoundCircuit::InRange {
    friend class PinRange;
    friend class Circuit::InRange;
  public:
    InRange(CompoundCircuit& obj, int ix, int n) throw();
    void operator=(const Circuit::OutRange& rhs) throw(connection_error);
    void operator=(const CompoundCircuit::OutRange& rhs) throw(connection_error);
    void operator=(const CompoundCircuit::InRange& rhs)
      throw(std::bad_alloc, connection_error);
  private:
    CompoundCircuit&	m_obj;
    const int		m_ix, m_n;
  };

  class CompoundCircuit::OutRange {
    friend class PinRange;
    friend class Circuit::InRange;
    friend class CompoundCircuit::InRange;
  public:
    OutRange(CompoundCircuit& obj, int ox, int n) throw();
    void operator=(const Circuit::OutRange& rhs) throw(connection_error);
    void operator=(const CompoundCircuit::OutRange& rhs) throw(connection_error);
  private:
    CompoundCircuit&	m_obj;
    const int		m_ox, m_n;
  };

  ////////////////////////////////////////////////////////////////

  inline Circuit::BitVector::BitVector(int len) throw(std::bad_alloc)
    : m_len(len), m_data((len+31)>>5) {}

  inline int Circuit::BitVector::length() const throw() { return m_len; }

  inline uint Circuit::BitVector::range(int pos, int len) const throw() {
    int ix = pos >> 5;
    int sft = pos & 0x1F;
    uint val;
    val = m_data[ix] >> sft;
    if (((pos+len-1) >> 5) != ix)  val |= (m_data[ix+1] << (32-sft));
    return val & (0xFFFFFFFFu >> (32-len));
  }

  inline void Circuit::BitVector::setrange(int pos, int len, uint val) throw() {
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

  inline Circuit::~Circuit() throw() {}

  inline uint Circuit::calc_outvalue(int ox) const { return 0; }
  inline void Circuit::evaluate() {}
  inline void Circuit::update() { m_itag.clear(); m_otag.clear(); }
  inline void Circuit::tick() { evaluate(); update(); }
  inline void Circuit::reset() { m_itag.clear(); m_otag.clear(); m_orecur.clear(); }

  inline const Circuit& Circuit::parent() const throw() { return *m_parent; }

  inline Circuit::InRange Circuit::in(int ix, int n) throw() {
    return Circuit::InRange(*this, ix, n);
  }
  inline Circuit::OutRange Circuit::out(int ox, int n) throw() {
    return Circuit::OutRange(*this, ox, n);
  }

  ////////////////////////////////////////////////////////////////

  inline CompoundCircuit::~CompoundCircuit() throw() {}

  inline void CompoundCircuit::add_component(Circuit& component)
    throw(std::bad_alloc, connection_error) {
    component.set_parent(*this);
    m_internalcomponents.push_back(&component);
  }

  template <typename Iter>
  inline void CompoundCircuit::add_components(Iter first, Iter last)	
    throw(std::bad_alloc, connection_error)
  {
    for ( ; first!=last; ++first)  add_component(*first);
  }

  inline CompoundCircuit& CompoundCircuit::operator<<(Circuit& component)
    throw(std::bad_alloc, connection_error)
  {
    add_component(component);
    return *this;
  }

  template <typename Iter>
  inline CompoundCircuit& CompoundCircuit::operator<<(const Components<Iter>& range)
    throw(std::bad_alloc, connection_error)
  {
    add_components(range.first, range.second);
    return *this;
  }

  inline void CompoundCircuit::evaluate() {
    Circuit::evaluate();
    int n = m_internalcomponents.size();
    for (int i=0; i<n; ++i)  m_internalcomponents[i]->evaluate();
  }

  inline void CompoundCircuit::update() {
    Circuit::update();
    int n = m_internalcomponents.size();
    for (int i=0; i<n; ++i)  m_internalcomponents[i]->update();
  }

  inline void CompoundCircuit::reset() {
    Circuit::reset();
    int n = m_internalcomponents.size();
    for (int i=0; i<n; ++i)  m_internalcomponents[i]->reset();
  }

  inline CompoundCircuit::InRange CompoundCircuit::in(int ix, int n) throw() {
    return CompoundCircuit::InRange(*this, ix, n);
  }
  inline CompoundCircuit::OutRange CompoundCircuit::out(int ox, int n) throw() {
    return CompoundCircuit::OutRange(*this, ox, n);
  }

  ////////////////////////////////////////////////////////////////

  inline Circuit::InRange::InRange(Circuit& obj, int ix, int n) throw()
    : m_obj(obj), m_ix(ix), m_n(n) {}
  inline Circuit::OutRange::OutRange(Circuit& obj, int ox, int n) throw()
    : m_obj(obj), m_ox(ox), m_n(n) {}

  inline CompoundCircuit::InRange::InRange(CompoundCircuit& obj, int ix, int n) throw()
    : m_obj(obj), m_ix(ix), m_n(n) {}
  inline CompoundCircuit::OutRange::OutRange(CompoundCircuit& obj, int ox, int n) throw()
    : m_obj(obj), m_ox(ox), m_n(n) {}

}

#endif

