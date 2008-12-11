// -*- c++ -*-

//
// Minimal synthesizable BCA subset of SystemC.
//

// Copyright (c) 2008 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

// sc_int<W> and sc_uint<W>
//   * 1..64 bits (sc_int), 1..63 bits (sc_uint)
//   * implemented as an "int" or a "long long" (value_type)
//        sc_int<1>..sc_int<32>    : int
//        sc_int<33>..sc_int<64>   : long long
//        sc_uint<1>..sc_uint<31>  : int
//        sc_uint<32>..sc_int<63>  : long long
//   * auto conversion to/from value_type
//   * operator[], operator()
//   * operator++, operator--, operator+= etc.
//
// Incompalitibilities from the standard SystemC
// which cannot be detected on compilation:
//   * sc_int<24> a = 0x100000;
//     sc_int<24> b = 0x100000;
//     sc_int<48> c = a * b;       <-- "0x10000000000" in SystemC, "0" in BCA-subset
//   * sc_uint<31> a = 0x7fffffff;
//     sc_uint<31> b = 0x7fffffff;
//     sc_uint<31> c = (a+b)>>2;   <-- "0x3fffffff" in SystemC, "0x7fffffff" in BCA-subset
// These incompatibilities can be fixed by changing
// "sc_int_common<>::operator value_type() const" to
// "sc_int_common<>::operator long long() const",
// which reults in slight performance loss.

#ifndef BCASYSC_DATATYPES_INT_SCINT_H
#define BCASYSC_DATATYPES_INT_SCINT_H

namespace sc_dt {

template <bool WIDE> struct sc_int_traits_body;
template <> struct sc_int_traits_body<false> { typedef int       value_type; };
template <> struct sc_int_traits_body<true>  { typedef long long value_type; };

template <typename S, int W> struct sc_int_traits;
template <int W> struct sc_int_traits<signed,W> {
  typedef typename sc_int_traits_body<(W>32)>::value_type value_type;
  static value_type wrap(value_type value) {
    return (value << (sizeof(value_type)*8-W)) >> (sizeof(value_type)*8-W);
  }
  static value_type adjust_setbit(value_type newvalue, int pos) {
    return (__builtin_constant_p(pos) && pos != (W-1)) ? newvalue :  wrap(newvalue);
  }
};
template <int W> struct sc_int_traits<unsigned,W> {
  typedef typename sc_int_traits_body<(W>=32)>::value_type value_type;
  static value_type wrap(value_type value) {
    value_type max_value = (static_cast<value_type>(1)<<W)-1;
    return value & max_value;
  }
  static value_type adjust_setbit(value_type newvalue, int pos) { return newvalue; }
};

template <typename Traits, bool B> struct sc_int_wrap_if;
template <typename Traits> struct sc_int_wrap_if<Traits,false> {
  typedef typename Traits::value_type value_type;
  static value_type wrap(value_type value) { return value; }
};
template <typename Traits> struct sc_int_wrap_if<Traits,true> {
  typedef typename Traits::value_type value_type;
  static value_type wrap(value_type value) { return Traits::wrap(value); }
};

struct sc_int_bitref_r {
  const bool m_val;
  explicit sc_int_bitref_r(bool val) : m_val(val) {}
  operator bool() const { return m_val; }
private:
  void operator=(const sc_int_bitref_r&);  // disabled
};

struct sc_int_subref_r {
  const long long m_value;
  const int m_size;
  sc_int_subref_r(long long value, int size) : m_value(value), m_size(size) {}
  explicit sc_int_subref_r(bool value) : m_value(value), m_size(1) {}
  sc_int_subref_r(const sc_int_bitref_r& x) : m_value(static_cast<bool>(x)), m_size(1) {}
  operator long long() const { return m_value; }
  int size() const { return m_size; }
private:
  void operator=(const sc_int_subref_r&);  // disabled
};

inline const sc_int_subref_r operator,(const sc_int_subref_r& lhs, const sc_int_subref_r& rhs) {
  return sc_int_subref_r((lhs << rhs.size()) | rhs, lhs.size() + rhs.size());
}
inline const sc_int_subref_r operator,(const sc_int_subref_r& lhs, const sc_int_bitref_r& rhs) {
  return (lhs, sc_int_subref_r(rhs));
}
inline const sc_int_subref_r operator,(const sc_int_subref_r& lhs, bool rhs) {
  return (lhs, sc_int_subref_r(rhs));
}
inline const sc_int_subref_r operator,(const sc_int_bitref_r& lhs, const sc_int_subref_r& rhs) {
  return (sc_int_subref_r(lhs), rhs);
}
inline const sc_int_subref_r operator,(const sc_int_bitref_r& lhs, const sc_int_bitref_r& rhs) {
  return (sc_int_subref_r(lhs), sc_int_subref_r(rhs));
}
inline const sc_int_subref_r operator,(const sc_int_bitref_r& lhs, bool rhs) {
  return (sc_int_subref_r(lhs), sc_int_subref_r(rhs));
}
inline const sc_int_subref_r operator,(bool lhs, const sc_int_subref_r& rhs) {
  return (sc_int_subref_r(lhs), rhs);
}
inline const sc_int_subref_r operator,(bool lhs, const sc_int_bitref_r& rhs) {
  return (sc_int_subref_r(lhs), sc_int_subref_r(rhs));
}

template <typename S, int W> struct sc_int_common;

template <typename S, int W> struct sc_int_bitref {
  typedef sc_int_common<S,W> obj_type;
  typedef typename obj_type::value_type value_type;
  obj_type& m_obj;
  const int m_pos;
  sc_int_bitref(obj_type& obj, int pos) : m_obj(obj), m_pos(pos) {}
  const sc_int_bitref& operator=(bool val) const { m_obj.set_bit(m_pos, val);  return *this; }
  const sc_int_bitref& operator=(const sc_int_bitref& rhs) const { return operator=(static_cast<bool>(rhs)); }
  operator bool() const { return (static_cast<const obj_type&>(m_obj))[m_pos]; }
  friend const sc_int_subref_r operator,(const sc_int_bitref& lhs, bool rhs) {
    return (lhs.to_subref_r(), sc_int_subref_r(rhs));
  }
  friend const sc_int_subref_r operator,(const sc_int_bitref& lhs, const sc_int_bitref_r& rhs) {
    return (lhs.to_subref_r(), sc_int_subref_r(rhs));
  }
  friend const sc_int_subref_r operator,(const sc_int_bitref& lhs, const sc_int_subref_r& rhs) {
    return (lhs.to_subref_r(), rhs);
  }
  friend const sc_int_subref_r operator,(bool lhs, const sc_int_bitref& rhs) {
    return (sc_int_subref_r(lhs), rhs.to_subref_r());
  }
  friend const sc_int_subref_r operator,(const sc_int_bitref_r& lhs, const sc_int_bitref& rhs) {
    return (sc_int_subref_r(lhs), rhs.to_subref_r());
  }
  friend const sc_int_subref_r operator,(const sc_int_subref_r& lhs, const sc_int_bitref& rhs) {
    return (lhs, rhs.to_subref_r());
  }
  // API for sc_int_catref
  int size() const { return 1; }
  const sc_int_subref_r to_subref_r() const { return sc_int_subref_r(*this); }
};

template <typename S, int W> struct sc_int_subref {
  typedef sc_int_common<S,W> obj_type;
  typedef typename obj_type::value_type value_type;
  obj_type& m_obj;
  const int m_msb, m_lsb;
  sc_int_subref(obj_type& obj, int msb, int lsb) : m_obj(obj), m_msb(msb), m_lsb(lsb) {}
  const sc_int_subref& operator=(value_type val) const { m_obj.set_sub(m_msb, m_lsb, val);  return *this; }
  const sc_int_subref& operator=(const sc_int_subref& rhs) const { return operator=(static_cast<value_type>(rhs)); }
  const sc_int_subref_r to_subref_r() const { return (static_cast<const obj_type&>(m_obj))(m_msb, m_lsb); }
  operator value_type() const { return to_subref_r(); }
  int size() const { return m_msb - m_lsb + 1; }
  friend const sc_int_subref_r operator,(const sc_int_subref& lhs, bool rhs) {
    return (lhs.to_subref_r(), sc_int_subref_r(rhs));
  }
  friend const sc_int_subref_r operator,(const sc_int_subref& lhs, const sc_int_bitref_r& rhs) {
    return (lhs.to_subref_r(), sc_int_subref_r(rhs));
  }
  friend const sc_int_subref_r operator,(const sc_int_subref& lhs, const sc_int_subref_r& rhs) {
    return (lhs.to_subref_r(), rhs);
  }
  friend const sc_int_subref_r operator,(bool lhs, const sc_int_subref& rhs) {
    return (sc_int_subref_r(lhs), rhs.to_subref_r());
  }
  friend const sc_int_subref_r operator,(const sc_int_bitref_r& lhs, const sc_int_subref& rhs) {
    return (sc_int_subref_r(lhs), rhs.to_subref_r());
  }
  friend const sc_int_subref_r operator,(const sc_int_subref_r& lhs, const sc_int_subref& rhs) {
    return (lhs, rhs.to_subref_r());
  }
};

////////////////////////////////////////////////////////////////

template <typename S, int W>
struct sc_int_common {
  typedef sc_int_traits<S,W>  traits_type;
  typedef typename traits_type::value_type  value_type;
  value_type m_value;
  operator value_type() const { return m_value; }
  sc_int_common() {}
  sc_int_common(value_type x) : m_value(traits_type::wrap(x)) {}
  template <int V> sc_int_common(const sc_int_common<S,V>& x) : m_value(sc_int_wrap_if<traits_type,(W<V)>::wrap(x.m_value)) {}
  template <typename T> sc_int_common& operator+=(T val)  { return *this = *this + val; }
  template <typename T> sc_int_common& operator-=(T val)  { return *this = *this - val; }
  template <typename T> sc_int_common& operator*=(T val)  { return *this = *this * val; }
  template <typename T> sc_int_common& operator/=(T val)  { return *this = *this / val; }
  template <typename T> sc_int_common& operator%=(T val)  { return *this = *this % val; }
  template <typename T> sc_int_common& operator&=(T val)  { return *this = *this & val; }
  template <typename T> sc_int_common& operator|=(T val)  { return *this = *this | val; }
  template <typename T> sc_int_common& operator^=(T val)  { return *this = *this ^ val; }
  template <typename T> sc_int_common& operator<<=(T val) { return *this = *this << val; }
  template <typename T> sc_int_common& operator>>=(T val) { return *this = *this >> val; }
  sc_int_common& operator++() { return *this += 1; }
  sc_int_common& operator--() { return *this -= 1; }
  const sc_int_common operator++(int) { sc_int_common ret = *this;  ++(*this);  return ret; }
  const sc_int_common operator--(int) { sc_int_common ret = *this;  --(*this);  return ret; }
  const sc_int_bitref<S,W> operator[](int pos) { return sc_int_bitref<S,W>(*this, pos); }
  const sc_int_bitref_r operator[](int pos) const { return sc_int_bitref_r((m_value>>pos)&1); }
  const sc_int_subref<S,W> operator()(int msb, int lsb) { return sc_int_subref<S,W>(*this, msb, lsb); }
  const sc_int_subref_r operator()(int msb, int lsb) const {
    int size = msb - lsb + 1;
    value_type mask = (1LL << size) - 1;
    return sc_int_subref_r((m_value>>lsb) & mask, size);
  }
  void set_bit(int pos, bool val) {
    value_type mask = static_cast<value_type>(1) << pos;
    value_type newval;
    if (__builtin_constant_p(val)) {
      newval = val ? (m_value | mask) : (m_value & ~mask);
    } else {
      newval = (m_value & ~mask) | (static_cast<value_type>(val) << pos);
    }
    m_value = traits_type::adjust_setbit(newval, pos);
  }
  void set_sub(int msb, int lsb, value_type value) {
    int size = msb - lsb + 1;
    value_type mask = (static_cast<value_type>(1) << size) - 1;
    value &= mask;
    mask <<= lsb;
    value_type newval = (m_value & ~mask) | (value << lsb);
    m_value = traits_type::adjust_setbit(newval, msb);
  }
  friend const sc_int_subref_r operator,(bool lhs, const sc_int_common& rhs) {
    return (sc_int_subref_r(lhs), rhs.to_subref_r());
  }
  friend const sc_int_subref_r operator,(const sc_int_bitref_r& lhs, const sc_int_common& rhs) {
    return (sc_int_subref_r(lhs), rhs.to_subref_r());
  }
  friend const sc_int_subref_r operator,(const sc_int_subref_r& lhs, const sc_int_common& rhs) {
    return (lhs, rhs.to_subref_r());
  }
  friend const sc_int_subref_r operator,(const sc_int_common& lhs, bool rhs) {
    return (lhs.to_subref_r(), sc_int_subref_r(rhs));
  }
  friend const sc_int_subref_r operator,(const sc_int_common& lhs, const sc_int_bitref_r& rhs) {
    return (lhs.to_subref_r(), sc_int_subref_r(rhs));
  }
  friend const sc_int_subref_r operator,(const sc_int_common& lhs, const sc_int_subref_r& rhs) {
    return (lhs.to_subref_r(), rhs);
  }
  // API for sc_int_catref
  int size() const { return W; }
  const sc_int_subref_r to_subref_r() const { return (*this)(W-1,0); }
};

template <typename S1, int W1, typename S2, int W2>
inline const sc_int_subref_r operator,(const sc_int_common<S1,W1>& lhs, const sc_int_bitref<S2,W2>& rhs) {
  return (lhs.to_subref_r(), rhs.to_subref_r());
}
template <typename S1, int W1, typename S2, int W2>
inline const sc_int_subref_r operator,(const sc_int_bitref<S2,W2>& lhs, const sc_int_common<S1,W1>& rhs) {
  return (lhs.to_subref_r(), rhs.to_subref_r());
}
template <typename S1, int W1, typename S2, int W2>
inline const sc_int_subref_r operator,(const sc_int_common<S1,W1>& lhs, const sc_int_subref<S2,W2>& rhs) {
  return (lhs.to_subref_r(), rhs.to_subref_r());
}
template <typename S1, int W1, typename S2, int W2>
inline const sc_int_subref_r operator,(const sc_int_subref<S1,W1>& lhs, const sc_int_common<S2,W2>& rhs) {
  return (lhs.to_subref_r(), rhs.to_subref_r());
}
template <typename S1, int W1, typename S2, int W2>
inline const sc_int_subref_r operator,(const sc_int_common<S1,W1>& lhs, const sc_int_common<S2,W2>& rhs) {
  return (lhs.to_subref_r(), rhs.to_subref_r());
}

////////////////////////////////////////////////////////////////

template <typename A, typename D> struct sc_int_catref {
  A& m_car;
  D& m_cdr;
  sc_int_catref(A& car, D& cdr) : m_car(car), m_cdr(cdr) {}
  const sc_int_catref& operator=(long long value) const { m_cdr = value;  m_car = (value>>m_cdr.size());  return *this; }
  const sc_int_catref& operator=(const sc_int_catref& rhs) const { return operator=(static_cast<long long>(rhs)); }
  friend const sc_int_subref_r operator,(const sc_int_catref& lhs, bool rhs) {
    return (lhs.to_subref_r(), sc_int_subref_r(rhs));
  }
  friend const sc_int_subref_r operator,(const sc_int_catref& lhs, const sc_int_bitref_r& rhs) {
    return (lhs.to_subref_r(), sc_int_subref_r(rhs));
  }
  friend const sc_int_subref_r operator,(const sc_int_catref& lhs, const sc_int_subref_r& rhs) {
    return (lhs.to_subref_r(), rhs);
  }
  friend const sc_int_subref_r operator,(bool lhs, const sc_int_catref& rhs) {
    return (sc_int_subref_r(lhs), rhs.to_subref_r());
  }
  friend const sc_int_subref_r operator,(const sc_int_bitref_r& lhs, const sc_int_catref& rhs) {
    return (sc_int_subref_r(lhs), rhs.to_subref_r());
  }
  friend const sc_int_subref_r operator,(const sc_int_subref_r& lhs, const sc_int_catref& rhs) {
    return (lhs, rhs.to_subref_r());
  }
  operator long long() const { return to_subref_r(); }
  int size() const { return m_car.size() + m_cdr.size(); }
  const sc_int_subref_r to_subref_r() const {
    return (m_car.to_subref_r(), m_cdr.to_subref_r());
  }
};

template <typename A, typename D, typename S2, int W2>
inline const sc_int_subref_r operator,(const sc_int_catref<A,D>& lhs, const sc_int_common<S2,W2>& rhs) {
  return (lhs.to_subref_r(), rhs.to_subref_r());
}
template <typename S1, int W1, typename A, typename D>
inline const sc_int_subref_r operator,(const sc_int_common<S1,W1>& lhs, const sc_int_catref<A,D>& rhs) {
  return (lhs.to_subref_r(), rhs.to_subref_r());
}

template <typename S1, int W1, typename S2, int W2>
inline const sc_int_catref< const sc_int_bitref<S1,W1>, const sc_int_bitref<S2,W2> >
operator,(const sc_int_bitref<S1,W1>& lhs, const sc_int_bitref<S2,W2>& rhs) {
  return sc_int_catref< const sc_int_bitref<S1,W1>, const sc_int_bitref<S2,W2> > (lhs, rhs);
}
template <typename S1, int W1, typename S2, int W2>
inline const sc_int_catref< const sc_int_bitref<S1,W1>, const sc_int_subref<S2,W2> >
operator,(const sc_int_bitref<S1,W1>& lhs, const sc_int_subref<S2,W2>& rhs) {
  return sc_int_catref< const sc_int_bitref<S1,W1>, const sc_int_subref<S2,W2> > (lhs, rhs);
}
template <typename S1, int W1, typename S2, int W2>
inline const sc_int_catref< const sc_int_bitref<S1,W1>, sc_int_common<S2,W2> >
operator,(const sc_int_bitref<S1,W1>& lhs, sc_int_common<S2,W2>& rhs) {
  return sc_int_catref< const sc_int_bitref<S1,W1>, sc_int_common<S2,W2> > (lhs, rhs);
}
template <typename S1, int W1, typename A, typename D>
inline const sc_int_catref< const sc_int_bitref<S1,W1>, const sc_int_catref<A,D> >
operator,(const sc_int_bitref<S1,W1>& lhs, const sc_int_catref<A,D>& rhs) {
  return sc_int_catref< const sc_int_bitref<S1,W1>, const sc_int_catref<A,D> > (lhs, rhs);
}
template <typename S1, int W1, typename S2, int W2>
inline const sc_int_catref< const sc_int_subref<S1,W1>, const sc_int_bitref<S2,W2> >
operator,(const sc_int_subref<S1,W1>& lhs, const sc_int_bitref<S2,W2>& rhs) {
  return sc_int_catref< const sc_int_subref<S1,W1>, const sc_int_bitref<S2,W2> > (lhs, rhs);
}
template <typename S1, int W1, typename S2, int W2>
inline const sc_int_catref< const sc_int_subref<S1,W1>, const sc_int_subref<S2,W2> >
operator,(const sc_int_subref<S1,W1>& lhs, const sc_int_subref<S2,W2>& rhs) {
  return sc_int_catref< const sc_int_subref<S1,W1>, const sc_int_subref<S2,W2> > (lhs, rhs);
}
template <typename S1, int W1, typename S2, int W2>
inline const sc_int_catref< const sc_int_subref<S1,W1>, sc_int_common<S2,W2> >
operator,(const sc_int_subref<S1,W1>& lhs, sc_int_common<S2,W2>& rhs) {
  return sc_int_catref< const sc_int_subref<S1,W1>, sc_int_common<S2,W2> > (lhs, rhs);
}
template <typename S1, int W1, typename A, typename D>
inline const sc_int_catref< const sc_int_subref<S1,W1>, const sc_int_catref<A,D> >
operator,(const sc_int_subref<S1,W1>& lhs, const sc_int_catref<A,D>& rhs) {
  return sc_int_catref< const sc_int_subref<S1,W1>, const sc_int_catref<A,D> > (lhs, rhs);
}
template <typename S1, int W1, typename S2, int W2>
inline const sc_int_catref< sc_int_common<S1,W1>, const sc_int_bitref<S2,W2> >
operator,(sc_int_common<S1,W1>& lhs, const sc_int_bitref<S2,W2>& rhs) {
  return sc_int_catref< sc_int_common<S1,W1>, const sc_int_bitref<S2,W2> > (lhs, rhs);
}
template <typename S1, int W1, typename S2, int W2>
inline const sc_int_catref< sc_int_common<S1,W1>, const sc_int_subref<S2,W2> >
operator,(sc_int_common<S1,W1>& lhs, const sc_int_subref<S2,W2>& rhs) {
  return sc_int_catref< sc_int_common<S1,W1>, const sc_int_subref<S2,W2> > (lhs, rhs);
}
template <typename S1, int W1, typename S2, int W2>
inline const sc_int_catref< sc_int_common<S1,W1>, sc_int_common<S2,W2> >
operator,(sc_int_common<S1,W1>& lhs, sc_int_common<S2,W2>& rhs) {
  return sc_int_catref< sc_int_common<S1,W1>, sc_int_common<S2,W2> > (lhs, rhs);
}
template <typename S1, int W1, typename A, typename D>
inline const sc_int_catref< sc_int_common<S1,W1>, const sc_int_catref<A,D> >
operator,(sc_int_common<S1,W1>& lhs, const sc_int_catref<A,D>& rhs) {
  return sc_int_catref< sc_int_common<S1,W1>, const sc_int_catref<A,D> > (lhs, rhs);
}
template <typename A, typename D, typename S2, int W2>
inline const sc_int_catref< const sc_int_catref<A,D>, const sc_int_bitref<S2,W2> >
operator,(const sc_int_catref<A,D>& lhs, const sc_int_bitref<S2,W2>& rhs) {
  return sc_int_catref< const sc_int_catref<A,D>, const sc_int_bitref<S2,W2> > (lhs, rhs);
}
template <typename A, typename D, typename S2, int W2>
inline const sc_int_catref< const sc_int_catref<A,D>, const sc_int_subref<S2,W2> >
operator,(const sc_int_catref<A,D>& lhs, const sc_int_subref<S2,W2>& rhs) {
  return sc_int_catref< const sc_int_catref<A,D>, const sc_int_subref<S2,W2> > (lhs, rhs);
}
template <typename A, typename D, typename S2, int W2>
inline const sc_int_catref< const sc_int_catref<A,D>, sc_int_common<S2,W2> >
operator,(const sc_int_catref<A,D>& lhs, sc_int_common<S2,W2>& rhs) {
  return sc_int_catref< const sc_int_catref<A,D>, sc_int_common<S2,W2> > (lhs, rhs);
}
template <typename A1, typename D1, typename A2, typename D2>
inline const sc_int_catref< const sc_int_catref<A1, D1>, const sc_int_catref<A2,D2> >
operator,(const sc_int_catref<A1,D1>& lhs, const sc_int_catref<A2,D2>& rhs) {
  return sc_int_catref< const sc_int_catref<A1,D1>, const sc_int_catref<A2,D2> > (lhs, rhs);
}

////////////////////////////////////////////////////////////////

// Define sc_int<W> is an alias of sc_int_common<signed,W>,
//   and sc_uint<W> as an alias of sc_int_common<unsigned,W>.
// In C++0x, we will need only 2 lines of code using "template typedef" syntax.

template <int W> struct sc_int : sc_int_common<signed,W> {
  typedef sc_int_common<signed,W>  base_type;
  sc_int() : base_type() {}
  template <typename T> sc_int(T x) : base_type(x) {}
};

template <int W> struct sc_uint : sc_int_common<unsigned,W> {
  typedef sc_int_common<unsigned,W>  base_type;
  sc_uint() : base_type() {}
  template <typename T> sc_uint(T x) : base_type(x) {}
};

}; // namespace sc_dt

#endif
