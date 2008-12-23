// -*- c++ -*-

//
// Minimal synthesizable BCA subset of SystemC.
//

// Copyright (c) 2008 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

// sc_int<W> and sc_uint<W>
//   * 1..64 bits
//   * implemented as an "int" or a "long long" (value_type)
//        sc_int<1>..sc_int<32>    : int
//        sc_int<33>..sc_int<64>   : long long
//        sc_uint<1>..sc_uint<32>  : unsigned int
//        sc_uint<33>..sc_int<64>  : unsigned long long
//   * auto conversion to/from value_type
//   * operator[], operator()
//   * operator++, operator--, operator+= etc.
//
// Incompalitibilities from the standard SystemC
// which cannot be detected on compilation:
//   * sc_int<24> a = 0x100000;
//     sc_int<24> b = 0x100000;
//     sc_int<48> c = a * b;       <-- "0x10000000000" in SystemC, "0" in BCA-subset
//   * sc_uint<32> a = 0xffffffff;
//     sc_uint<32> b = 0xffffffff;
//     sc_uint<32> c = (a+b)>>1;   <-- "0xffffffff" in SystemC, "0x7fffffff" in BCA-subset
// These incompatibilities can be fixed by changing
// "sc_int_common<>::operator value_type() const" to
// "sc_int_common<>::operator sc_traits<S,64>::value_type() const",
// which reults in slight performance loss.

#ifndef BCASYSC_DATATYPES_INT_SCINT_H
#define BCASYSC_DATATYPES_INT_SCINT_H

namespace sc_dt {

template <typename S, bool WIDE> struct sc_int_traits_body;
template <> struct sc_int_traits_body<signed,   false> { typedef signed int         value_type; };
template <> struct sc_int_traits_body<signed,   true>  { typedef signed long long   value_type; };
template <> struct sc_int_traits_body<unsigned, false> { typedef unsigned int       value_type; };
template <> struct sc_int_traits_body<unsigned, true>  { typedef unsigned long long value_type; };

template <typename S, int W> struct sc_int_traits;
template <int W> struct sc_int_traits<signed,W> {
  typedef typename sc_int_traits_body<signed,(W>32)>::value_type value_type;
  static const value_type MINVAL = static_cast<value_type>(-1) << (W-1);
  static const value_type MAXVAL = -MINVAL-1;
  static value_type wrap(value_type value) {
    return (value << (sizeof(value_type)*8-W)) >> (sizeof(value_type)*8-W);
  }
  static value_type wrap(value_type value, int width) {
    return (width <= W) ? value : wrap(value);
  }
  static value_type adjust_setbit(value_type newvalue, int pos) {
    return (__builtin_constant_p(pos) && pos != (W-1)) ? newvalue :  wrap(newvalue);
  }
};
template <int W> struct sc_int_traits<unsigned,W> {
  typedef typename sc_int_traits_body<unsigned,(W>32)>::value_type value_type;
  static const value_type MINVAL = 0;
  static const value_type MAXVAL = static_cast<value_type>(-1) >> (sizeof(value_type)*8-W);
  static value_type wrap(value_type value) {
    value_type max_value = static_cast<value_type>(-1) >> (sizeof(value_type)*8-W);
    return value & max_value;
  }
  static value_type wrap(value_type value, int width) {
    return (width <= W) ? value : wrap(value);
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
  const unsigned long long m_value;
  const int m_size;
  sc_int_subref_r(unsigned long long value, int size) : m_value(value), m_size(size) {}
  explicit sc_int_subref_r(bool value) : m_value(value), m_size(1) {}
  sc_int_subref_r(const sc_int_bitref_r& x) : m_value(static_cast<bool>(x)), m_size(1) {}
  operator unsigned long long() const { return m_value; }
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
  template <typename T> const sc_int_bitref& operator&=(T val) const { return *this = *this & val; }
  template <typename T> const sc_int_bitref& operator|=(T val) const { return *this = *this | val; }
  template <typename T> const sc_int_bitref& operator^=(T val) const { return *this = *this ^ val; }
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
  template <typename T> const sc_int_subref& operator+=(T val) const  { return *this = *this + val; }
  template <typename T> const sc_int_subref& operator-=(T val) const  { return *this = *this - val; }
  template <typename T> const sc_int_subref& operator*=(T val) const  { return *this = *this * val; }
  template <typename T> const sc_int_subref& operator/=(T val) const  { return *this = *this / val; }
  template <typename T> const sc_int_subref& operator%=(T val) const  { return *this = *this % val; }
  template <typename T> const sc_int_subref& operator&=(T val) const  { return *this = *this & val; }
  template <typename T> const sc_int_subref& operator|=(T val) const  { return *this = *this | val; }
  template <typename T> const sc_int_subref& operator^=(T val) const  { return *this = *this ^ val; }
  template <typename T> const sc_int_subref& operator<<=(T val) const { return *this = *this << val; }
  template <typename T> const sc_int_subref& operator>>=(T val) const { return *this = *this >> val; }
  const sc_int_subref& operator++() const { return *this += 1; }
  const sc_int_subref& operator--() const { return *this -= 1; }
  const sc_int_subref operator++(int) const { sc_int_subref ret = *this;  ++(*this);  return ret; }
  const sc_int_subref operator--(int) const { sc_int_subref ret = *this;  --(*this);  return ret; }
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

template <bool WIDE> struct sc_int_ranged_aux;
template <> struct sc_int_ranged_aux<false> {
  static int swidth(unsigned int x) {
    int sx = static_cast<int>(x);
    int sign = sx >> 31;
    int ans = 1;
    if ((sx >> 16) != sign) { sx >>= 16;  ans += 16; }
    if ((sx >>  8) != sign) { sx >>=  8;  ans +=  8; }
    if ((sx >>  4) != sign) { sx >>=  4;  ans +=  4; }
    if ((sx >>  2) != sign) { sx >>=  2;  ans +=  2; }
    if ((sx >>  1) != sign) { sx >>=  1;  ans +=  1; }
    if (sx != sign)  ++ans;
    return ans;
  }
  static int uwidth(unsigned int x) {
    int ans = 1;
    if ((x >> 16) != 0) { x >>= 16;  ans += 16; }
    if ((x >>  8) != 0) { x >>=  8;  ans +=  8; }
    if ((x >>  4) != 0) { x >>=  4;  ans +=  4; }
    if ((x >>  2) != 0) { x >>=  2;  ans +=  2; }
    if ((x >>  1) != 0) { x >>=  1;  ans +=  1; }
    return ans;
  }
};
template <> struct sc_int_ranged_aux<true> {
  static int swidth(unsigned long long x) {
    signed long long sx = static_cast<signed long long>(x);
    signed long long sign = sx >> 63;
    int ans = 1;
    if ((sx >> 32) != sign) { sx >>= 32;  ans += 32; }
    if ((sx >> 16) != sign) { sx >>= 16;  ans += 16; }
    if ((sx >>  8) != sign) { sx >>=  8;  ans +=  8; }
    if ((sx >>  4) != sign) { sx >>=  4;  ans +=  4; }
    if ((sx >>  2) != sign) { sx >>=  2;  ans +=  2; }
    if ((sx >>  1) != sign) { sx >>=  1;  ans +=  1; }
    if (sx != sign)  ++ans;
    return ans;
  }
  static int uwidth(unsigned long long x) {
    int ans = 1;
    if ((x >> 32) != 0) { x >>= 32;  ans += 32; }
    if ((x >> 16) != 0) { x >>= 16;  ans += 16; }
    if ((x >>  8) != 0) { x >>=  8;  ans +=  8; }
    if ((x >>  4) != 0) { x >>=  4;  ans +=  4; }
    if ((x >>  2) != 0) { x >>=  2;  ans +=  2; }
    if ((x >>  1) != 0) { x >>=  1;  ans +=  1; }
    return ans;
  }
};

template <typename S, bool WIDE> struct sc_int_ranged_body : public sc_int_ranged_aux<WIDE> {
  typedef typename sc_int_traits_body<S,       WIDE>::value_type  value_type;
  typedef typename sc_int_traits_body<signed,  WIDE>::value_type svalue_type;
  typedef typename sc_int_traits_body<unsigned,WIDE>::value_type uvalue_type;
  const uvalue_type m_uval, m_minval, m_range;
  sc_int_ranged_body(uvalue_type val, uvalue_type minval, uvalue_type range)
    : m_uval(val), m_minval(__builtin_constant_p(val) ? val : minval), m_range(__builtin_constant_p(val) ? 0 : range) {}
  operator value_type() const { return m_uval; }
  bool signed_range() const { return m_minval+m_range < m_minval; }
  bool unsigned_range() const { return static_cast<svalue_type>(m_minval+m_range) < static_cast<svalue_type>(m_minval); }
  int width(signed) const {
    if (unsigned_range())  return sizeof(value_type)*8;
    int minwidth = swidth(m_minval);
    int maxwidth = swidth(m_minval+m_range);
    return (minwidth > maxwidth) ? minwidth : maxwidth;
  }
  int width(unsigned) const {
    return signed_range() ? sizeof(value_type)*8 : uwidth(m_minval+m_range);
  }
};

template <typename S, bool WIDE> struct sc_int_ranged;
typedef sc_int_ranged<  signed,false>  sc_int_ranged_si;
typedef sc_int_ranged<unsigned,false>  sc_int_ranged_ui;
typedef sc_int_ranged<  signed,true >  sc_int_ranged_sl;
typedef sc_int_ranged<unsigned,true >  sc_int_ranged_ul;

template <> struct sc_int_ranged<signed,false> : public sc_int_ranged_body<signed,false> {
  typedef sc_int_ranged_body<signed,false> base_type;
  typedef base_type::uvalue_type uvalue_type;
  sc_int_ranged(uvalue_type val, uvalue_type minval, uvalue_type range) : base_type(val, minval, range) {}
  explicit sc_int_ranged(bool x)           : base_type(x, 0, 1) {}
  explicit sc_int_ranged(signed char x)    : base_type(x, -128, 255) {}
  explicit sc_int_ranged(unsigned char x)  : base_type(x, 0, 255) {}
  explicit sc_int_ranged(signed short x)   : base_type(x, -32768, 65535) {}
  explicit sc_int_ranged(unsigned short x) : base_type(x, 0, 65535) {}
  explicit sc_int_ranged(signed int x)     : base_type(x, 0x80000000u, 0xffffffffu) {}
};

template <> struct sc_int_ranged<unsigned,false> : public sc_int_ranged_body<unsigned,false> {
  typedef sc_int_ranged_body<unsigned,false> base_type;
  typedef base_type::uvalue_type uvalue_type;
  sc_int_ranged(uvalue_type val, uvalue_type minval, uvalue_type range) : base_type(val, minval, range) {}
  explicit sc_int_ranged(unsigned int x)   : base_type(x, 0u, 0xffffffffu) {}
  explicit sc_int_ranged(const sc_int_ranged_si& x) : base_type(x.m_uval, x.m_minval, x.m_range) {}
};

template <> struct sc_int_ranged<signed,true> : public sc_int_ranged_body<signed,true> {
  typedef sc_int_ranged_body<signed,true> base_type;
  typedef base_type::uvalue_type uvalue_type;
  sc_int_ranged(uvalue_type val, uvalue_type minval, uvalue_type range) : base_type(val, minval, range) {}
  explicit sc_int_ranged(signed long long x) : base_type(x, 0x8000000000000000ull, 0xffffffffffffffffull) {}
  explicit sc_int_ranged(const sc_int_ranged_si& x)
    : base_type(static_cast<int>(x.m_uval),
		x.unsigned_range() ? static_cast<int>(0x80000000u) : static_cast<int>(x.m_minval),
		x.unsigned_range() ? 0xffffffffu : x.m_range) {}
  explicit sc_int_ranged(const sc_int_ranged_ui& x)
    : base_type(x.m_uval, x.signed_range() ? 0 : x.m_minval, x.signed_range() ? 0xffffffffu : x.m_range) {}
};

template <> struct sc_int_ranged<unsigned,true> : public sc_int_ranged_body<unsigned,true> {
  typedef sc_int_ranged_body<unsigned,true> base_type;
  typedef base_type::uvalue_type uvalue_type;
  sc_int_ranged(uvalue_type val, uvalue_type minval, uvalue_type range) : base_type(val, minval, range) {}
  explicit sc_int_ranged(unsigned long long x) : base_type(x, 0, 0xfffffffffffffffful) {}
  explicit sc_int_ranged(const sc_int_ranged_si& x)
    : base_type(static_cast<int>(x.m_uval),
		x.unsigned_range() ? static_cast<int>(0x80000000u) : static_cast<int>(x.m_minval),
		x.unsigned_range() ? 0xffffffffu : x.m_range) {}
  explicit sc_int_ranged(const sc_int_ranged_ui& x)
    : base_type(x.m_uval, x.signed_range() ? 0 : x.m_minval, x.signed_range() ? 0xffffffffu : x.m_range) {}
  explicit sc_int_ranged(const sc_int_ranged_sl& x) : base_type(x.m_uval, x.m_minval, x.m_range) {}
};

template <typename S, bool WIDE>
inline const sc_int_ranged<S,WIDE> operator+(const sc_int_ranged<S,WIDE>& a, const sc_int_ranged<S,WIDE>& b) {
  typedef typename sc_int_traits_body<unsigned,WIDE>::value_type uvalue_type;
  uvalue_type val = a.m_uval + b.m_uval;
  uvalue_type minval = a.m_minval + b.m_minval;
  uvalue_type range  = a.m_range + b.m_range;
  if (range < a.m_range) {
    // overflow
    minval = sc_int_traits<S,(WIDE?64:32)>::MINVAL;
    range = static_cast<uvalue_type>(-1);
  }
  return sc_int_ranged<S,WIDE>(val, minval, range);
}

inline const sc_int_ranged_ui operator+(const sc_int_ranged_si& a, const sc_int_ranged_ui& b)
  { return static_cast<sc_int_ranged_ui>(a) + b; }
inline const sc_int_ranged_sl operator+(const sc_int_ranged_si& a, const sc_int_ranged_sl& b)
  { return static_cast<sc_int_ranged_sl>(a) + b; }
inline const sc_int_ranged_sl operator+(const sc_int_ranged_ui& a, const sc_int_ranged_sl& b)
  { return static_cast<sc_int_ranged_sl>(a) + b; }
inline const sc_int_ranged_ul operator+(const sc_int_ranged_si& a, const sc_int_ranged_ul& b)
  { return static_cast<sc_int_ranged_ul>(a) + b; }
inline const sc_int_ranged_ul operator+(const sc_int_ranged_ui& a, const sc_int_ranged_ul& b)
  { return static_cast<sc_int_ranged_ul>(a) + b; }
inline const sc_int_ranged_ul operator+(const sc_int_ranged_sl& a, const sc_int_ranged_ul& b)
  { return static_cast<sc_int_ranged_ul>(a) + b; }
inline const sc_int_ranged_ui operator+(const sc_int_ranged_ui& a, const sc_int_ranged_si& b)
  { return a + static_cast<sc_int_ranged_ui>(b); }
inline const sc_int_ranged_sl operator+(const sc_int_ranged_sl& a, const sc_int_ranged_si& b)
  { return a + static_cast<sc_int_ranged_sl>(b); }
inline const sc_int_ranged_sl operator+(const sc_int_ranged_sl& a, const sc_int_ranged_ui& b)
  { return a + static_cast<sc_int_ranged_sl>(b); }
inline const sc_int_ranged_ul operator+(const sc_int_ranged_ul& a, const sc_int_ranged_si& b)
  { return a + static_cast<sc_int_ranged_ul>(b); }
inline const sc_int_ranged_ul operator+(const sc_int_ranged_ul& a, const sc_int_ranged_ui& b)
  { return a + static_cast<sc_int_ranged_ul>(b); }
inline const sc_int_ranged_ul operator+(const sc_int_ranged_ul& a, const sc_int_ranged_sl& b)
  { return a + static_cast<sc_int_ranged_ul>(b); }

template <typename L, typename R> struct sc_int_ranged_binop_traits;
template <> struct sc_int_ranged_binop_traits<sc_int_ranged_si,sc_int_ranged_si> { typedef sc_int_ranged_si result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_ranged_si,sc_int_ranged_ui> { typedef sc_int_ranged_ui result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_ranged_si,sc_int_ranged_sl> { typedef sc_int_ranged_sl result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_ranged_si,sc_int_ranged_ul> { typedef sc_int_ranged_ul result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_ranged_ui,sc_int_ranged_si> { typedef sc_int_ranged_ui result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_ranged_ui,sc_int_ranged_ui> { typedef sc_int_ranged_ui result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_ranged_ui,sc_int_ranged_sl> { typedef sc_int_ranged_sl result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_ranged_ui,sc_int_ranged_ul> { typedef sc_int_ranged_ul result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_ranged_sl,sc_int_ranged_si> { typedef sc_int_ranged_sl result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_ranged_sl,sc_int_ranged_ui> { typedef sc_int_ranged_sl result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_ranged_sl,sc_int_ranged_sl> { typedef sc_int_ranged_sl result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_ranged_sl,sc_int_ranged_ul> { typedef sc_int_ranged_ul result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_ranged_ul,sc_int_ranged_si> { typedef sc_int_ranged_ul result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_ranged_ul,sc_int_ranged_ui> { typedef sc_int_ranged_ul result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_ranged_ul,sc_int_ranged_sl> { typedef sc_int_ranged_ul result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_ranged_ul,sc_int_ranged_ul> { typedef sc_int_ranged_ul result_type; };

inline const sc_int_ranged_si operator+(const sc_int_ranged_si& a, bool               b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_si operator+(const sc_int_ranged_si& a, signed char        b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_si operator+(const sc_int_ranged_si& a, unsigned char      b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_si operator+(const sc_int_ranged_si& a, signed short       b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_si operator+(const sc_int_ranged_si& a, unsigned short     b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_si operator+(const sc_int_ranged_si& a, signed int         b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_ui operator+(const sc_int_ranged_si& a, unsigned int       b) { return a + sc_int_ranged_ui(b); }
inline const sc_int_ranged_sl operator+(const sc_int_ranged_si& a, signed long long   b) { return a + sc_int_ranged_sl(b); }
inline const sc_int_ranged_ul operator+(const sc_int_ranged_si& a, unsigned long long b) { return a + sc_int_ranged_ul(b); }

inline const sc_int_ranged_ui operator+(const sc_int_ranged_ui& a, bool               b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_ui operator+(const sc_int_ranged_ui& a, signed char        b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_ui operator+(const sc_int_ranged_ui& a, unsigned char      b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_ui operator+(const sc_int_ranged_ui& a, signed short       b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_ui operator+(const sc_int_ranged_ui& a, unsigned short     b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_ui operator+(const sc_int_ranged_ui& a, signed int         b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_ui operator+(const sc_int_ranged_ui& a, unsigned int       b) { return a + sc_int_ranged_ui(b); }
inline const sc_int_ranged_sl operator+(const sc_int_ranged_ui& a, signed long long   b) { return a + sc_int_ranged_sl(b); }
inline const sc_int_ranged_ul operator+(const sc_int_ranged_ui& a, unsigned long long b) { return a + sc_int_ranged_ul(b); }

inline const sc_int_ranged_sl operator+(const sc_int_ranged_sl& a, bool               b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_sl operator+(const sc_int_ranged_sl& a, signed char        b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_sl operator+(const sc_int_ranged_sl& a, unsigned char      b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_sl operator+(const sc_int_ranged_sl& a, signed short       b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_sl operator+(const sc_int_ranged_sl& a, unsigned short     b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_sl operator+(const sc_int_ranged_sl& a, signed int         b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_sl operator+(const sc_int_ranged_sl& a, unsigned int       b) { return a + sc_int_ranged_ui(b); }
inline const sc_int_ranged_sl operator+(const sc_int_ranged_sl& a, signed long long   b) { return a + sc_int_ranged_sl(b); }
inline const sc_int_ranged_ul operator+(const sc_int_ranged_sl& a, unsigned long long b) { return a + sc_int_ranged_ul(b); }

inline const sc_int_ranged_ul operator+(const sc_int_ranged_ul& a, bool               b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_ul operator+(const sc_int_ranged_ul& a, signed char        b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_ul operator+(const sc_int_ranged_ul& a, unsigned char      b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_ul operator+(const sc_int_ranged_ul& a, signed short       b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_ul operator+(const sc_int_ranged_ul& a, unsigned short     b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_ul operator+(const sc_int_ranged_ul& a, signed int         b) { return a + sc_int_ranged_si(b); }
inline const sc_int_ranged_ul operator+(const sc_int_ranged_ul& a, unsigned int       b) { return a + sc_int_ranged_ui(b); }
inline const sc_int_ranged_ul operator+(const sc_int_ranged_ul& a, signed long long   b) { return a + sc_int_ranged_sl(b); }
inline const sc_int_ranged_ul operator+(const sc_int_ranged_ul& a, unsigned long long b) { return a + sc_int_ranged_ul(b); }

inline const sc_int_ranged_si operator+(bool               a, const sc_int_ranged_si& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_si operator+(signed char        a, const sc_int_ranged_si& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_si operator+(unsigned char      a, const sc_int_ranged_si& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_si operator+(signed short       a, const sc_int_ranged_si& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_si operator+(unsigned short     a, const sc_int_ranged_si& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_si operator+(signed int         a, const sc_int_ranged_si& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_ui operator+(unsigned int       a, const sc_int_ranged_si& b) { return sc_int_ranged_ui(a) + b; }
inline const sc_int_ranged_sl operator+(signed long long   a, const sc_int_ranged_si& b) { return sc_int_ranged_sl(a) + b; }
inline const sc_int_ranged_ul operator+(unsigned long long a, const sc_int_ranged_si& b) { return sc_int_ranged_ul(a) + b; }

inline const sc_int_ranged_ui operator+(bool               a, const sc_int_ranged_ui& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_ui operator+(signed char        a, const sc_int_ranged_ui& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_ui operator+(unsigned char      a, const sc_int_ranged_ui& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_ui operator+(signed short       a, const sc_int_ranged_ui& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_ui operator+(unsigned short     a, const sc_int_ranged_ui& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_ui operator+(signed int         a, const sc_int_ranged_ui& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_ui operator+(unsigned int       a, const sc_int_ranged_ui& b) { return sc_int_ranged_ui(a) + b; }
inline const sc_int_ranged_sl operator+(signed long long   a, const sc_int_ranged_ui& b) { return sc_int_ranged_sl(a) + b; }
inline const sc_int_ranged_ul operator+(unsigned long long a, const sc_int_ranged_ui& b) { return sc_int_ranged_ul(a) + b; }

inline const sc_int_ranged_sl operator+(bool               a, const sc_int_ranged_sl& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_sl operator+(signed char        a, const sc_int_ranged_sl& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_sl operator+(unsigned char      a, const sc_int_ranged_sl& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_sl operator+(signed short       a, const sc_int_ranged_sl& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_sl operator+(unsigned short     a, const sc_int_ranged_sl& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_sl operator+(signed int         a, const sc_int_ranged_sl& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_sl operator+(unsigned int       a, const sc_int_ranged_sl& b) { return sc_int_ranged_ui(a) + b; }
inline const sc_int_ranged_sl operator+(signed long long   a, const sc_int_ranged_sl& b) { return sc_int_ranged_sl(a) + b; }
inline const sc_int_ranged_ul operator+(unsigned long long a, const sc_int_ranged_sl& b) { return sc_int_ranged_ul(a) + b; }

inline const sc_int_ranged_ul operator+(bool               a, const sc_int_ranged_ul& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_ul operator+(signed char        a, const sc_int_ranged_ul& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_ul operator+(unsigned char      a, const sc_int_ranged_ul& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_ul operator+(signed short       a, const sc_int_ranged_ul& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_ul operator+(unsigned short     a, const sc_int_ranged_ul& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_ul operator+(signed int         a, const sc_int_ranged_ul& b) { return sc_int_ranged_si(a) + b; }
inline const sc_int_ranged_ul operator+(unsigned int       a, const sc_int_ranged_ul& b) { return sc_int_ranged_ui(a) + b; }
inline const sc_int_ranged_ul operator+(signed long long   a, const sc_int_ranged_ul& b) { return sc_int_ranged_sl(a) + b; }
inline const sc_int_ranged_ul operator+(unsigned long long a, const sc_int_ranged_ul& b) { return sc_int_ranged_ul(a) + b; }

////////////////////////////////////////////////////////////////

template <typename S, int W>
struct sc_int_common {
  typedef sc_int_traits<S,W>  traits_type;
  typedef typename traits_type::value_type  value_type;
  typedef sc_int_ranged<S,(W>32)>  ranged_type;
  value_type m_value;
  operator value_type() const { return m_value; }
  sc_int_common() {}
  sc_int_common(value_type x) : m_value(traits_type::wrap(x)) {}
  template <int V> sc_int_common(const sc_int_common<S,V>& x) : m_value(sc_int_wrap_if<traits_type,(W<V)>::wrap(x.m_value)) {}
  template <typename SS, bool WIDE>
  sc_int_common(const sc_int_ranged<SS,WIDE>& x) : m_value(traits_type::wrap(x, x.width(S()))) {}
  const ranged_type to_ranged() const {
    return ranged_type(m_value, traits_type::MINVAL, traits_type::MAXVAL-traits_type::MINVAL);
  }

  template <typename SS, bool WIDE> struct binop_traits : sc_int_ranged_binop_traits<sc_int_ranged<SS,WIDE>,ranged_type> {};
  typedef typename binop_traits<  signed,false>::result_type binop_si_type;
  typedef typename binop_traits<unsigned,false>::result_type binop_ui_type;
  typedef typename binop_traits<  signed,true >::result_type binop_sl_type;
  typedef typename binop_traits<unsigned,true >::result_type binop_ul_type;
  friend const binop_si_type operator+(const sc_int_common& a, bool               b) { return a.to_ranged() + b; }
  friend const binop_si_type operator+(const sc_int_common& a, signed char        b) { return a.to_ranged() + b; }
  friend const binop_si_type operator+(const sc_int_common& a, unsigned char      b) { return a.to_ranged() + b; }
  friend const binop_si_type operator+(const sc_int_common& a, signed short       b) { return a.to_ranged() + b; }
  friend const binop_si_type operator+(const sc_int_common& a, unsigned short     b) { return a.to_ranged() + b; }
  friend const binop_si_type operator+(const sc_int_common& a, signed int         b) { return a.to_ranged() + b; }
  friend const binop_ui_type operator+(const sc_int_common& a, unsigned int       b) { return a.to_ranged() + b; }
  friend const binop_sl_type operator+(const sc_int_common& a, signed long long   b) { return a.to_ranged() + b; }
  friend const binop_ul_type operator+(const sc_int_common& a, unsigned long long b) { return a.to_ranged() + b; }
  friend const binop_si_type operator+(bool               a, const sc_int_common& b) { return a + b.to_ranged(); }
  friend const binop_si_type operator+(signed char        a, const sc_int_common& b) { return a + b.to_ranged(); }
  friend const binop_si_type operator+(unsigned char      a, const sc_int_common& b) { return a + b.to_ranged(); }
  friend const binop_si_type operator+(signed short       a, const sc_int_common& b) { return a + b.to_ranged(); }
  friend const binop_si_type operator+(unsigned short     a, const sc_int_common& b) { return a + b.to_ranged(); }
  friend const binop_si_type operator+(signed int         a, const sc_int_common& b) { return a + b.to_ranged(); }
  friend const binop_ui_type operator+(unsigned int       a, const sc_int_common& b) { return a + b.to_ranged(); }
  friend const binop_sl_type operator+(signed long long   a, const sc_int_common& b) { return a + b.to_ranged(); }
  friend const binop_ul_type operator+(unsigned long long a, const sc_int_common& b) { return a + b.to_ranged(); }
  template <typename SS, bool WIDE> friend const typename binop_traits<SS,WIDE>::result_type
  operator+(const sc_int_common& a, const sc_int_ranged<SS,WIDE>& b) { return a.to_ranged() + b; }
  template <typename SS, bool WIDE> friend const typename binop_traits<SS,WIDE>::result_type
  operator+(const sc_int_ranged<SS,WIDE>& a, const sc_int_common& b) { return a + b.to_ranged(); }

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

template <typename S1, int W1, typename S2, int W2>
inline const typename sc_int_ranged_binop_traits<typename sc_int_common<S1,W1>::ranged_type,
						 typename sc_int_common<S2,W2>::ranged_type>::result_type
operator+(const sc_int_common<S1,W1>& a, const sc_int_common<S2,W2>& b) { return a.to_ranged() + b.to_ranged(); }

////////////////////////////////////////////////////////////////

template <typename A, typename D> struct sc_int_catref {
  A& m_car;
  D& m_cdr;
  sc_int_catref(A& car, D& cdr) : m_car(car), m_cdr(cdr) {}
  const sc_int_catref& operator=(unsigned long long value) const { m_cdr=value; m_car=(value>>m_cdr.size()); return *this; }
  const sc_int_catref& operator=(const sc_int_catref& rhs) const { return operator=(static_cast<unsigned long long>(rhs)); }
  template <typename T> const sc_int_catref& operator+=(T val) const  { return *this = *this + val; }
  template <typename T> const sc_int_catref& operator-=(T val) const  { return *this = *this - val; }
  template <typename T> const sc_int_catref& operator*=(T val) const  { return *this = *this * val; }
  template <typename T> const sc_int_catref& operator/=(T val) const  { return *this = *this / val; }
  template <typename T> const sc_int_catref& operator%=(T val) const  { return *this = *this % val; }
  template <typename T> const sc_int_catref& operator&=(T val) const  { return *this = *this & val; }
  template <typename T> const sc_int_catref& operator|=(T val) const  { return *this = *this | val; }
  template <typename T> const sc_int_catref& operator^=(T val) const  { return *this = *this ^ val; }
  template <typename T> const sc_int_catref& operator<<=(T val) const { return *this = *this << val; }
  template <typename T> const sc_int_catref& operator>>=(T val) const { return *this = *this >> val; }
  const sc_int_catref& operator++() const { return *this += 1; }
  const sc_int_catref& operator--() const { return *this -= 1; }
  const sc_int_catref operator++(int) const { sc_int_catref ret = *this;  ++(*this);  return ret; }
  const sc_int_catref operator--(int) const { sc_int_catref ret = *this;  --(*this);  return ret; }
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
  operator unsigned long long() const { return to_subref_r(); }
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

} // namespace sc_dt

#endif
