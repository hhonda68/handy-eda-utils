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

#ifndef BCASYSC_DATATYPES_INT_SCINT_H
#define BCASYSC_DATATYPES_INT_SCINT_H

namespace sc_dt {

#define sc_inline __attribute__((always_inline)) inline

template <typename S, bool WIDE> struct sc_int_traits_body;
template <> struct sc_int_traits_body<signed,   false> { typedef signed int         value_type; };
template <> struct sc_int_traits_body<signed,   true>  { typedef signed long long   value_type; };
template <> struct sc_int_traits_body<unsigned, false> { typedef unsigned int       value_type; };
template <> struct sc_int_traits_body<unsigned, true>  { typedef unsigned long long value_type; };

template <typename S, int W> struct sc_int_traits;
template <int W> struct sc_int_traits<signed,W> {
  typedef typename sc_int_traits_body<  signed,(W>32)>::value_type value_type;
  typedef typename sc_int_traits_body<unsigned,(W>32)>::value_type uvalue_type;
  static const value_type MINVAL = static_cast<value_type>(-1) << (W-1);
  static const value_type MAXVAL = ~MINVAL;
  static const uvalue_type RANGE = static_cast<uvalue_type>(-1) >> (sizeof(uvalue_type)*8-W);
  sc_inline static value_type wrap(value_type value) { return (value << (sizeof(value_type)*8-W)) >> (sizeof(value_type)*8-W); }
  sc_inline static value_type wrap(value_type value, int width) { return (width <= W) ? value : wrap(value); }
  sc_inline static value_type adjust_setbit(value_type newvalue, int pos)
    { return (__builtin_constant_p(pos) && pos != (W-1)) ? newvalue :  wrap(newvalue); }
};
template <int W> struct sc_int_traits<unsigned,W> {
  typedef typename sc_int_traits_body<unsigned,(W>32)>::value_type value_type;
  typedef value_type uvalue_type;
  static const value_type MINVAL = 0;
  static const value_type MAXVAL = static_cast<value_type>(-1) >> (sizeof(value_type)*8-W);
  static const uvalue_type RANGE = MAXVAL;
  sc_inline static value_type wrap(value_type value) {
    value_type max_value = static_cast<value_type>(-1) >> (sizeof(value_type)*8-W);
    return value & max_value;
  }
  sc_inline static value_type wrap(value_type value, int width) {
    return (width <= W) ? value : wrap(value);
  }
  sc_inline static value_type adjust_setbit(value_type newvalue, int pos) { return newvalue; }
};

struct sc_int_bitref_r {
  const bool m_val;
  sc_inline explicit sc_int_bitref_r(bool val) : m_val(val) {}
  sc_inline operator bool() const { return m_val; }
private:
  void operator=(const sc_int_bitref_r&);  // disabled
};

struct sc_int_subref_r {
  const unsigned long long m_value;
  const int m_size;
  sc_inline sc_int_subref_r(unsigned long long value, int size) : m_value(value), m_size(size) {}
  sc_inline explicit sc_int_subref_r(bool value) : m_value(value), m_size(1) {}
  sc_inline sc_int_subref_r(const sc_int_bitref_r& x) : m_value(static_cast<bool>(x)), m_size(1) {}
  sc_inline operator unsigned long long() const { return m_value; }
  sc_inline int size() const { return m_size; }
  sc_inline const sc_int_subref_r to_catref_r() const { return *this; }
private:
  void operator=(const sc_int_subref_r&);  // disabled
};

template <typename A, typename D>
struct sc_int_catref_r {
  const A& m_car;
  const D& m_cdr;
  sc_inline sc_int_catref_r(const A& car, const D& cdr) : m_car(car), m_cdr(cdr) {}
  typedef sc_int_catref_r<sc_int_catref_r,sc_int_subref_r> sc_int_csref_r;
  typedef sc_int_catref_r<sc_int_subref_r,sc_int_catref_r> sc_int_scref_r;
  sc_inline friend const sc_int_csref_r operator,(const sc_int_catref_r& lhs, bool rhs)
    { return sc_int_csref_r(lhs, sc_int_subref_r(rhs)); }
  sc_inline friend const sc_int_csref_r operator,(const sc_int_catref_r& lhs, const sc_int_bitref_r& rhs)
    { return sc_int_csref_r(lhs, sc_int_subref_r(rhs)); }
  sc_inline friend const sc_int_csref_r operator,(const sc_int_catref_r& lhs, const sc_int_subref_r& rhs)
    { return sc_int_csref_r(lhs, rhs); }
  sc_inline friend const sc_int_scref_r operator,(bool lhs, const sc_int_catref_r& rhs)
    { return sc_int_scref_r(sc_int_subref_r(lhs), rhs); }
  sc_inline friend const sc_int_scref_r operator,(const sc_int_bitref_r& lhs, const sc_int_catref_r& rhs)
    { return sc_int_scref_r(sc_int_subref_r(lhs), rhs); }
  sc_inline friend const sc_int_scref_r operator,(const sc_int_subref_r& lhs, const sc_int_catref_r& rhs)
    { return sc_int_scref_r(lhs, rhs); }
  sc_inline operator unsigned long long() const { return to_subref_r(); }
  sc_inline int size() const { return m_car.size() + m_cdr.size(); }
  sc_inline const sc_int_subref_r to_subref_r() const
    { return sc_int_subref_r((m_car << m_cdr.size()) | m_cdr, m_car.size() + m_cdr.size()); }
private:
  void operator=(const sc_int_catref_r&);  // disabled
};

typedef sc_int_catref_r<sc_int_subref_r,sc_int_subref_r> sc_int_ssref_r;

sc_inline const sc_int_ssref_r operator,(const sc_int_subref_r& lhs, const sc_int_subref_r& rhs)
  { return sc_int_ssref_r(lhs, rhs); }
sc_inline const sc_int_ssref_r operator,(const sc_int_subref_r& lhs, const sc_int_bitref_r& rhs)
  { return sc_int_ssref_r(lhs, sc_int_subref_r(rhs)); }
sc_inline const sc_int_ssref_r operator,(const sc_int_subref_r& lhs, bool rhs)
  { return sc_int_ssref_r(lhs, sc_int_subref_r(rhs)); }
sc_inline const sc_int_ssref_r operator,(const sc_int_bitref_r& lhs, const sc_int_subref_r& rhs)
  { return sc_int_ssref_r(sc_int_subref_r(lhs), rhs); }
sc_inline const sc_int_ssref_r operator,(const sc_int_bitref_r& lhs, const sc_int_bitref_r& rhs)
  { return sc_int_ssref_r(sc_int_subref_r(lhs), sc_int_subref_r(rhs)); }
sc_inline const sc_int_ssref_r operator,(const sc_int_bitref_r& lhs, bool rhs)
  { return sc_int_ssref_r(sc_int_subref_r(lhs), sc_int_subref_r(rhs)); }
sc_inline const sc_int_ssref_r operator,(bool lhs, const sc_int_subref_r& rhs)
  { return sc_int_ssref_r(sc_int_subref_r(lhs), rhs); }
sc_inline const sc_int_ssref_r operator,(bool lhs, const sc_int_bitref_r& rhs)
  { return sc_int_ssref_r(sc_int_subref_r(lhs), sc_int_subref_r(rhs)); }

template <typename A1, typename D1, typename A2, typename D2>
sc_inline const sc_int_catref_r< sc_int_catref_r<A1,D1>, sc_int_catref_r<A2,D2> >
operator,(const sc_int_catref_r<A1,D1>& lhs, const sc_int_catref_r<A2,D2>& rhs)
  { return sc_int_catref_r< sc_int_catref_r<A1,D1>, sc_int_catref_r<A2,D2> >(lhs, rhs); }

template <typename S, int W> struct sc_int_common;

template <typename S, int W> struct sc_int_bitref {
  typedef sc_int_common<S,W> obj_type;
  typedef typename obj_type::value_type value_type;
  obj_type& m_obj;
  const int m_pos;
  sc_inline sc_int_bitref(obj_type& obj, int pos) : m_obj(obj), m_pos(pos) {}
  sc_inline const sc_int_bitref& operator=(bool val) const { m_obj.set_bit(m_pos, val);  return *this; }
  sc_inline const sc_int_bitref& operator=(const sc_int_bitref& rhs) const { return operator=(static_cast<bool>(rhs)); }
  template <typename T> sc_inline const sc_int_bitref& operator&=(T val) const { return *this = *this & val; }
  template <typename T> sc_inline const sc_int_bitref& operator|=(T val) const { return *this = *this | val; }
  template <typename T> sc_inline const sc_int_bitref& operator^=(T val) const { return *this = *this ^ val; }
  sc_inline operator bool() const { return (static_cast<const obj_type&>(m_obj))[m_pos]; }
  sc_inline friend const sc_int_ssref_r operator,(const sc_int_bitref& lhs, bool rhs)
    { return (lhs.to_subref_r(), sc_int_subref_r(rhs)); }
  sc_inline friend const sc_int_ssref_r operator,(const sc_int_bitref& lhs, const sc_int_bitref_r& rhs)
    { return (lhs.to_subref_r(), sc_int_subref_r(rhs)); }
  sc_inline friend const sc_int_ssref_r operator,(const sc_int_bitref& lhs, const sc_int_subref_r& rhs)
    { return (lhs.to_subref_r(), rhs); }
  sc_inline friend const sc_int_ssref_r operator,(bool lhs, const sc_int_bitref& rhs)
    { return (sc_int_subref_r(lhs), rhs.to_subref_r()); }
  sc_inline friend const sc_int_ssref_r operator,(const sc_int_bitref_r& lhs, const sc_int_bitref& rhs)
    { return (sc_int_subref_r(lhs), rhs.to_subref_r()); }
  sc_inline friend const sc_int_ssref_r operator,(const sc_int_subref_r& lhs, const sc_int_bitref& rhs)
    { return (lhs, rhs.to_subref_r()); }
  // API for sc_int_catref
  sc_inline int size() const { return 1; }
  sc_inline const sc_int_subref_r to_subref_r() const { return sc_int_subref_r(*this); }
  typedef sc_int_subref_r catref_r_type;
  sc_inline const catref_r_type to_catref_r() const { return to_subref_r(); }
};

template <typename S, int W> struct sc_int_subref {
  typedef sc_int_common<S,W> obj_type;
  typedef typename obj_type::value_type value_type;
  typedef typename sc_int_traits<S,W>::uvalue_type uvalue_type;
  obj_type& m_obj;
  const int m_msb, m_lsb;
  sc_inline sc_int_subref(obj_type& obj, int msb, int lsb) : m_obj(obj), m_msb(msb), m_lsb(lsb) {}
  sc_inline const sc_int_subref& operator=(value_type val) const { m_obj.set_sub(m_msb, m_lsb, val);  return *this; }
  sc_inline const sc_int_subref& operator=(const sc_int_subref& rhs) const { return operator=(static_cast<value_type>(rhs)); }
  template <typename T> sc_inline const sc_int_subref& operator+=(T val) const { return *this = *this + val; }
  template <typename T> sc_inline const sc_int_subref& operator-=(T val) const { return *this = *this - val; }
  template <typename T> sc_inline const sc_int_subref& operator*=(T val) const { return *this = *this * val; }
  template <typename T> sc_inline const sc_int_subref& operator/=(T val) const { return *this = *this / val; }
  template <typename T> sc_inline const sc_int_subref& operator%=(T val) const { return *this = *this % val; }
  template <typename T> sc_inline const sc_int_subref& operator&=(T val) const { return *this = *this & val; }
  template <typename T> sc_inline const sc_int_subref& operator|=(T val) const { return *this = *this | val; }
  template <typename T> sc_inline const sc_int_subref& operator^=(T val) const { return *this = *this ^ val; }
  template <typename T> sc_inline const sc_int_subref& operator<<=(T val) const { return *this = *this << val; }
  template <typename T> sc_inline const sc_int_subref& operator>>=(T val) const { return *this = *this >> val; }
  sc_inline const sc_int_subref& operator++() const { return *this += 1; }
  sc_inline const sc_int_subref& operator--() const { return *this -= 1; }
  sc_inline const sc_int_subref operator++(int) const { sc_int_subref ret = *this;  ++(*this);  return ret; }
  sc_inline const sc_int_subref operator--(int) const { sc_int_subref ret = *this;  --(*this);  return ret; }
  sc_inline const sc_int_subref_r to_subref_r() const { return (static_cast<const obj_type&>(m_obj))(m_msb, m_lsb); }
  typedef sc_int_subref_r catref_r_type;
  sc_inline const catref_r_type to_catref_r() const { return to_subref_r(); }
  sc_inline operator uvalue_type() const { return to_subref_r(); }
  sc_inline int size() const { return m_msb - m_lsb + 1; }
  sc_inline friend const sc_int_ssref_r operator,(const sc_int_subref& lhs, bool rhs)
    { return (lhs.to_subref_r(), sc_int_subref_r(rhs)); }
  sc_inline friend const sc_int_ssref_r operator,(const sc_int_subref& lhs, const sc_int_bitref_r& rhs)
    { return (lhs.to_subref_r(), sc_int_subref_r(rhs)); }
  sc_inline friend const sc_int_ssref_r operator,(const sc_int_subref& lhs, const sc_int_subref_r& rhs)
    { return (lhs.to_subref_r(), rhs); }
  sc_inline friend const sc_int_ssref_r operator,(bool lhs, const sc_int_subref& rhs)
    { return (sc_int_subref_r(lhs), rhs.to_subref_r()); }
  sc_inline friend const sc_int_ssref_r operator,(const sc_int_bitref_r& lhs, const sc_int_subref& rhs)
    { return (sc_int_subref_r(lhs), rhs.to_subref_r()); }
  sc_inline friend const sc_int_ssref_r operator,(const sc_int_subref_r& lhs, const sc_int_subref& rhs)
    { return (lhs, rhs.to_subref_r()); }
};

////////////////////////////////////////////////////////////////

sc_inline int sc_int_clz(unsigned int       x) { return __builtin_clz(x); }
sc_inline int sc_int_clz(unsigned long long x) { return __builtin_clzll(x); }

template <bool WIDE> struct sc_int_ranged_data {
  typedef typename sc_int_traits_body<signed,  WIDE>::value_type svalue_type;
  typedef typename sc_int_traits_body<unsigned,WIDE>::value_type uvalue_type;
  const uvalue_type m_uval, m_minval, m_range;
  sc_inline sc_int_ranged_data(uvalue_type val, uvalue_type minval, uvalue_type range)
    : m_uval(val), m_minval(__builtin_constant_p(val) ? val : minval), m_range(__builtin_constant_p(val) ? 0 : range) {}
  sc_inline bool overflow_as_unsigned() const { return m_minval+m_range < m_minval; }
  sc_inline bool overflow_as_signed() const
    { return static_cast<svalue_type>(m_minval+m_range) < static_cast<svalue_type>(m_minval); }
  sc_inline int width(signed) const {
    uvalue_type x = m_minval;
    uvalue_type y = m_minval + m_range;
    bool overflow = static_cast<svalue_type>(y) < static_cast<svalue_type>(x);
    uvalue_type xx = x ^ (static_cast<svalue_type>(x)>>(sizeof(uvalue_type)*8-1));
    uvalue_type yy = y ^ (static_cast<svalue_type>(y)>>(sizeof(uvalue_type)*8-1));
    uvalue_type zz = xx | yy;
    return sizeof(svalue_type)*8 - (overflow ? 0 : sc_int_clz(zz+zz+1));
  }
  sc_inline int width(unsigned) const {
    return sizeof(uvalue_type)*8 - (overflow_as_unsigned() ? 0 : sc_int_clz((m_minval+m_range)|1));
  }
};

template <typename S, bool WIDE> struct sc_int_ranged_body : public sc_int_ranged_data<WIDE> {
  typedef typename sc_int_traits_body<S,WIDE>::value_type value_type;
  typedef typename sc_int_ranged_data<WIDE>::uvalue_type uvalue_type;
  sc_inline sc_int_ranged_body(uvalue_type val, uvalue_type minval, uvalue_type range)
    : sc_int_ranged_data<WIDE>(val, minval, range) {}
  using sc_int_ranged_data<WIDE>::m_uval;
  using sc_int_ranged_data<WIDE>::m_minval;
  using sc_int_ranged_data<WIDE>::m_range;
  using sc_int_ranged_data<WIDE>::overflow_as_unsigned;
  using sc_int_ranged_data<WIDE>::width;
  sc_inline operator value_type() const { return m_uval; }
  sc_inline bool overflow() const { return static_cast<value_type>(m_minval+m_range) < static_cast<value_type>(m_minval); }
  sc_inline int width() const { return width(S()); }
  sc_inline uvalue_type minval_as_shiftcount() const
    { return overflow_as_unsigned() ? 0 : m_minval; }
  sc_inline uvalue_type maxval_as_shiftcount() const
    { return overflow() ? sc_int_traits<S,(WIDE?64:32)>::MAXVAL : (m_minval+m_range); }
  // minval_as_shiftcount() and maxval_as_shiftcount() return meaningless value
  // for meaningless argument such that S=singned and (signed)minval < (signed)maxval < 0.
};

template <typename S, bool WIDE> struct sc_int_ranged;
typedef sc_int_ranged<  signed,false>  sc_int_rsi; // ranged signed integer
typedef sc_int_ranged<unsigned,false>  sc_int_rui; // ranged unsigned integer
typedef sc_int_ranged<  signed,true >  sc_int_rsl; // ranged signed long long
typedef sc_int_ranged<unsigned,true >  sc_int_rul; // ranged unsigned long long

template <> struct sc_int_ranged<signed,false> : public sc_int_ranged_body<signed,false> {
  typedef sc_int_ranged_body<signed,false> base_type;
  sc_inline sc_int_ranged(uvalue_type val, uvalue_type minval, uvalue_type range) : base_type(val, minval, range) {}
  sc_inline explicit sc_int_ranged(bool x)           : base_type(x, 0, 1) {}
  sc_inline explicit sc_int_ranged(signed char x)    : base_type(x, static_cast<uvalue_type>(-128), 255) {}
  sc_inline explicit sc_int_ranged(unsigned char x)  : base_type(x, 0, 255) {}
  sc_inline explicit sc_int_ranged(signed short x)   : base_type(x, static_cast<uvalue_type>(-32768), 65535) {}
  sc_inline explicit sc_int_ranged(unsigned short x) : base_type(x, 0, 65535) {}
  sc_inline explicit sc_int_ranged(signed int x)     : base_type(x, 0x80000000u, 0xffffffffu) {}
};

template <> struct sc_int_ranged<unsigned,false> : public sc_int_ranged_body<unsigned,false> {
  typedef sc_int_ranged_body<unsigned,false> base_type;
  sc_inline sc_int_ranged(uvalue_type val, uvalue_type minval, uvalue_type range) : base_type(val, minval, range) {}
  sc_inline explicit sc_int_ranged(unsigned int x)      : base_type(x, 0u, 0xffffffffu) {}
  sc_inline explicit sc_int_ranged(const sc_int_rsi& x) : base_type(x.m_uval, x.m_minval, x.m_range) {}
};

template <> struct sc_int_ranged<signed,true> : public sc_int_ranged_body<signed,true> {
  typedef sc_int_ranged_body<signed,true> base_type;
  sc_inline sc_int_ranged(uvalue_type val, uvalue_type minval, uvalue_type range) : base_type(val, minval, range) {}
  sc_inline explicit sc_int_ranged(signed long long x) : base_type(x, 0x8000000000000000ull, 0xffffffffffffffffull) {}
  sc_inline explicit sc_int_ranged(const sc_int_rsi& x)
    : base_type(static_cast<int>(x.m_uval),
		x.overflow_as_signed() ? static_cast<int>(0x80000000u) : static_cast<int>(x.m_minval),
		x.overflow_as_signed() ? 0xffffffffu : x.m_range) {}
  sc_inline explicit sc_int_ranged(const sc_int_rui& x)
    : base_type(x.m_uval, x.overflow_as_unsigned() ? 0 : x.m_minval, x.overflow_as_unsigned() ? 0xffffffffu : x.m_range) {}
};

template <> struct sc_int_ranged<unsigned,true> : public sc_int_ranged_body<unsigned,true> {
  typedef sc_int_ranged_body<unsigned,true> base_type;
  sc_inline sc_int_ranged(uvalue_type val, uvalue_type minval, uvalue_type range) : base_type(val, minval, range) {}
  sc_inline explicit sc_int_ranged(unsigned long long x) : base_type(x, 0, 0xffffffffffffffffull) {}
  sc_inline explicit sc_int_ranged(const sc_int_rsi& x)
    : base_type(static_cast<int>(x.m_uval),
		x.overflow_as_signed() ? static_cast<int>(0x80000000u) : static_cast<int>(x.m_minval),
		x.overflow_as_signed() ? 0xffffffffu : x.m_range) {}
  sc_inline explicit sc_int_ranged(const sc_int_rui& x)
    : base_type(x.m_uval, x.overflow_as_unsigned() ? 0 : x.m_minval, x.overflow_as_unsigned() ? 0xffffffffu : x.m_range) {}
  sc_inline explicit sc_int_ranged(const sc_int_rsl& x) : base_type(x.m_uval, x.m_minval, x.m_range) {}
};

template <typename S, bool WIDE>
sc_inline const sc_int_ranged<S,WIDE> operator+(const sc_int_ranged<S,WIDE>& a, const sc_int_ranged<S,WIDE>& b) {
  typedef typename sc_int_ranged<S,WIDE>::uvalue_type uvalue_type;
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

template <typename S, bool WIDE>
sc_inline const sc_int_ranged<S,WIDE> operator-(const sc_int_ranged<S,WIDE>& a, const sc_int_ranged<S,WIDE>& b) {
  typedef typename sc_int_ranged<S,WIDE>::uvalue_type uvalue_type;
  uvalue_type val = a.m_uval - b.m_uval;
  uvalue_type minval = a.m_minval - (b.m_minval+b.m_range);
  uvalue_type range  = a.m_range + b.m_range;
  if (range < a.m_range) {
    // overflow
    minval = sc_int_traits<S,(WIDE?64:32)>::MINVAL;
    range = static_cast<uvalue_type>(-1);
  }
  return sc_int_ranged<S,WIDE>(val, minval, range);
}

sc_inline const sc_int_rui operator+(const sc_int_rsi& a, const sc_int_rui& b) { return static_cast<sc_int_rui>(a) + b; }
sc_inline const sc_int_rsl operator+(const sc_int_rsi& a, const sc_int_rsl& b) { return static_cast<sc_int_rsl>(a) + b; }
sc_inline const sc_int_rsl operator+(const sc_int_rui& a, const sc_int_rsl& b) { return static_cast<sc_int_rsl>(a) + b; }
sc_inline const sc_int_rul operator+(const sc_int_rsi& a, const sc_int_rul& b) { return static_cast<sc_int_rul>(a) + b; }
sc_inline const sc_int_rul operator+(const sc_int_rui& a, const sc_int_rul& b) { return static_cast<sc_int_rul>(a) + b; }
sc_inline const sc_int_rul operator+(const sc_int_rsl& a, const sc_int_rul& b) { return static_cast<sc_int_rul>(a) + b; }
sc_inline const sc_int_rui operator+(const sc_int_rui& a, const sc_int_rsi& b) { return a + static_cast<sc_int_rui>(b); }
sc_inline const sc_int_rsl operator+(const sc_int_rsl& a, const sc_int_rsi& b) { return a + static_cast<sc_int_rsl>(b); }
sc_inline const sc_int_rsl operator+(const sc_int_rsl& a, const sc_int_rui& b) { return a + static_cast<sc_int_rsl>(b); }
sc_inline const sc_int_rul operator+(const sc_int_rul& a, const sc_int_rsi& b) { return a + static_cast<sc_int_rul>(b); }
sc_inline const sc_int_rul operator+(const sc_int_rul& a, const sc_int_rui& b) { return a + static_cast<sc_int_rul>(b); }
sc_inline const sc_int_rul operator+(const sc_int_rul& a, const sc_int_rsl& b) { return a + static_cast<sc_int_rul>(b); }

sc_inline const sc_int_rui operator-(const sc_int_rsi& a, const sc_int_rui& b) { return static_cast<sc_int_rui>(a) - b; }
sc_inline const sc_int_rsl operator-(const sc_int_rsi& a, const sc_int_rsl& b) { return static_cast<sc_int_rsl>(a) - b; }
sc_inline const sc_int_rsl operator-(const sc_int_rui& a, const sc_int_rsl& b) { return static_cast<sc_int_rsl>(a) - b; }
sc_inline const sc_int_rul operator-(const sc_int_rsi& a, const sc_int_rul& b) { return static_cast<sc_int_rul>(a) - b; }
sc_inline const sc_int_rul operator-(const sc_int_rui& a, const sc_int_rul& b) { return static_cast<sc_int_rul>(a) - b; }
sc_inline const sc_int_rul operator-(const sc_int_rsl& a, const sc_int_rul& b) { return static_cast<sc_int_rul>(a) - b; }
sc_inline const sc_int_rui operator-(const sc_int_rui& a, const sc_int_rsi& b) { return a - static_cast<sc_int_rui>(b); }
sc_inline const sc_int_rsl operator-(const sc_int_rsl& a, const sc_int_rsi& b) { return a - static_cast<sc_int_rsl>(b); }
sc_inline const sc_int_rsl operator-(const sc_int_rsl& a, const sc_int_rui& b) { return a - static_cast<sc_int_rsl>(b); }
sc_inline const sc_int_rul operator-(const sc_int_rul& a, const sc_int_rsi& b) { return a - static_cast<sc_int_rul>(b); }
sc_inline const sc_int_rul operator-(const sc_int_rul& a, const sc_int_rui& b) { return a - static_cast<sc_int_rul>(b); }
sc_inline const sc_int_rul operator-(const sc_int_rul& a, const sc_int_rsl& b) { return a - static_cast<sc_int_rul>(b); }

template <typename L, typename R> struct sc_int_ranged_binop_traits;
template <> struct sc_int_ranged_binop_traits<sc_int_rsi,sc_int_rsi> { typedef sc_int_rsi result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_rsi,sc_int_rui> { typedef sc_int_rui result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_rsi,sc_int_rsl> { typedef sc_int_rsl result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_rsi,sc_int_rul> { typedef sc_int_rul result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_rui,sc_int_rsi> { typedef sc_int_rui result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_rui,sc_int_rui> { typedef sc_int_rui result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_rui,sc_int_rsl> { typedef sc_int_rsl result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_rui,sc_int_rul> { typedef sc_int_rul result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_rsl,sc_int_rsi> { typedef sc_int_rsl result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_rsl,sc_int_rui> { typedef sc_int_rsl result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_rsl,sc_int_rsl> { typedef sc_int_rsl result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_rsl,sc_int_rul> { typedef sc_int_rul result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_rul,sc_int_rsi> { typedef sc_int_rul result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_rul,sc_int_rui> { typedef sc_int_rul result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_rul,sc_int_rsl> { typedef sc_int_rul result_type; };
template <> struct sc_int_ranged_binop_traits<sc_int_rul,sc_int_rul> { typedef sc_int_rul result_type; };

sc_inline const sc_int_rsi operator+(const sc_int_rsi& a, bool               b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rsi operator+(const sc_int_rsi& a, signed char        b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rsi operator+(const sc_int_rsi& a, unsigned char      b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rsi operator+(const sc_int_rsi& a, signed short       b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rsi operator+(const sc_int_rsi& a, unsigned short     b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rsi operator+(const sc_int_rsi& a, signed int         b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rui operator+(const sc_int_rsi& a, unsigned int       b) { return a + sc_int_rui(b); }
sc_inline const sc_int_rsl operator+(const sc_int_rsi& a, signed long long   b) { return a + sc_int_rsl(b); }
sc_inline const sc_int_rul operator+(const sc_int_rsi& a, unsigned long long b) { return a + sc_int_rul(b); }

sc_inline const sc_int_rui operator+(const sc_int_rui& a, bool               b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rui operator+(const sc_int_rui& a, signed char        b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rui operator+(const sc_int_rui& a, unsigned char      b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rui operator+(const sc_int_rui& a, signed short       b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rui operator+(const sc_int_rui& a, unsigned short     b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rui operator+(const sc_int_rui& a, signed int         b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rui operator+(const sc_int_rui& a, unsigned int       b) { return a + sc_int_rui(b); }
sc_inline const sc_int_rsl operator+(const sc_int_rui& a, signed long long   b) { return a + sc_int_rsl(b); }
sc_inline const sc_int_rul operator+(const sc_int_rui& a, unsigned long long b) { return a + sc_int_rul(b); }

sc_inline const sc_int_rsl operator+(const sc_int_rsl& a, bool               b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rsl operator+(const sc_int_rsl& a, signed char        b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rsl operator+(const sc_int_rsl& a, unsigned char      b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rsl operator+(const sc_int_rsl& a, signed short       b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rsl operator+(const sc_int_rsl& a, unsigned short     b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rsl operator+(const sc_int_rsl& a, signed int         b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rsl operator+(const sc_int_rsl& a, unsigned int       b) { return a + sc_int_rui(b); }
sc_inline const sc_int_rsl operator+(const sc_int_rsl& a, signed long long   b) { return a + sc_int_rsl(b); }
sc_inline const sc_int_rul operator+(const sc_int_rsl& a, unsigned long long b) { return a + sc_int_rul(b); }

sc_inline const sc_int_rul operator+(const sc_int_rul& a, bool               b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rul operator+(const sc_int_rul& a, signed char        b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rul operator+(const sc_int_rul& a, unsigned char      b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rul operator+(const sc_int_rul& a, signed short       b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rul operator+(const sc_int_rul& a, unsigned short     b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rul operator+(const sc_int_rul& a, signed int         b) { return a + sc_int_rsi(b); }
sc_inline const sc_int_rul operator+(const sc_int_rul& a, unsigned int       b) { return a + sc_int_rui(b); }
sc_inline const sc_int_rul operator+(const sc_int_rul& a, signed long long   b) { return a + sc_int_rsl(b); }
sc_inline const sc_int_rul operator+(const sc_int_rul& a, unsigned long long b) { return a + sc_int_rul(b); }

sc_inline const sc_int_rsi operator+(bool               a, const sc_int_rsi& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rsi operator+(signed char        a, const sc_int_rsi& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rsi operator+(unsigned char      a, const sc_int_rsi& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rsi operator+(signed short       a, const sc_int_rsi& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rsi operator+(unsigned short     a, const sc_int_rsi& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rsi operator+(signed int         a, const sc_int_rsi& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rui operator+(unsigned int       a, const sc_int_rsi& b) { return sc_int_rui(a) + b; }
sc_inline const sc_int_rsl operator+(signed long long   a, const sc_int_rsi& b) { return sc_int_rsl(a) + b; }
sc_inline const sc_int_rul operator+(unsigned long long a, const sc_int_rsi& b) { return sc_int_rul(a) + b; }

sc_inline const sc_int_rui operator+(bool               a, const sc_int_rui& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rui operator+(signed char        a, const sc_int_rui& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rui operator+(unsigned char      a, const sc_int_rui& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rui operator+(signed short       a, const sc_int_rui& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rui operator+(unsigned short     a, const sc_int_rui& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rui operator+(signed int         a, const sc_int_rui& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rui operator+(unsigned int       a, const sc_int_rui& b) { return sc_int_rui(a) + b; }
sc_inline const sc_int_rsl operator+(signed long long   a, const sc_int_rui& b) { return sc_int_rsl(a) + b; }
sc_inline const sc_int_rul operator+(unsigned long long a, const sc_int_rui& b) { return sc_int_rul(a) + b; }

sc_inline const sc_int_rsl operator+(bool               a, const sc_int_rsl& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rsl operator+(signed char        a, const sc_int_rsl& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rsl operator+(unsigned char      a, const sc_int_rsl& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rsl operator+(signed short       a, const sc_int_rsl& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rsl operator+(unsigned short     a, const sc_int_rsl& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rsl operator+(signed int         a, const sc_int_rsl& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rsl operator+(unsigned int       a, const sc_int_rsl& b) { return sc_int_rui(a) + b; }
sc_inline const sc_int_rsl operator+(signed long long   a, const sc_int_rsl& b) { return sc_int_rsl(a) + b; }
sc_inline const sc_int_rul operator+(unsigned long long a, const sc_int_rsl& b) { return sc_int_rul(a) + b; }

sc_inline const sc_int_rul operator+(bool               a, const sc_int_rul& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rul operator+(signed char        a, const sc_int_rul& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rul operator+(unsigned char      a, const sc_int_rul& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rul operator+(signed short       a, const sc_int_rul& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rul operator+(unsigned short     a, const sc_int_rul& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rul operator+(signed int         a, const sc_int_rul& b) { return sc_int_rsi(a) + b; }
sc_inline const sc_int_rul operator+(unsigned int       a, const sc_int_rul& b) { return sc_int_rui(a) + b; }
sc_inline const sc_int_rul operator+(signed long long   a, const sc_int_rul& b) { return sc_int_rsl(a) + b; }
sc_inline const sc_int_rul operator+(unsigned long long a, const sc_int_rul& b) { return sc_int_rul(a) + b; }

sc_inline const sc_int_rsi operator-(const sc_int_rsi& a, bool               b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rsi operator-(const sc_int_rsi& a, signed char        b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rsi operator-(const sc_int_rsi& a, unsigned char      b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rsi operator-(const sc_int_rsi& a, signed short       b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rsi operator-(const sc_int_rsi& a, unsigned short     b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rsi operator-(const sc_int_rsi& a, signed int         b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rui operator-(const sc_int_rsi& a, unsigned int       b) { return a - sc_int_rui(b); }
sc_inline const sc_int_rsl operator-(const sc_int_rsi& a, signed long long   b) { return a - sc_int_rsl(b); }
sc_inline const sc_int_rul operator-(const sc_int_rsi& a, unsigned long long b) { return a - sc_int_rul(b); }

sc_inline const sc_int_rui operator-(const sc_int_rui& a, bool               b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rui operator-(const sc_int_rui& a, signed char        b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rui operator-(const sc_int_rui& a, unsigned char      b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rui operator-(const sc_int_rui& a, signed short       b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rui operator-(const sc_int_rui& a, unsigned short     b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rui operator-(const sc_int_rui& a, signed int         b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rui operator-(const sc_int_rui& a, unsigned int       b) { return a - sc_int_rui(b); }
sc_inline const sc_int_rsl operator-(const sc_int_rui& a, signed long long   b) { return a - sc_int_rsl(b); }
sc_inline const sc_int_rul operator-(const sc_int_rui& a, unsigned long long b) { return a - sc_int_rul(b); }

sc_inline const sc_int_rsl operator-(const sc_int_rsl& a, bool               b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rsl operator-(const sc_int_rsl& a, signed char        b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rsl operator-(const sc_int_rsl& a, unsigned char      b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rsl operator-(const sc_int_rsl& a, signed short       b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rsl operator-(const sc_int_rsl& a, unsigned short     b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rsl operator-(const sc_int_rsl& a, signed int         b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rsl operator-(const sc_int_rsl& a, unsigned int       b) { return a - sc_int_rui(b); }
sc_inline const sc_int_rsl operator-(const sc_int_rsl& a, signed long long   b) { return a - sc_int_rsl(b); }
sc_inline const sc_int_rul operator-(const sc_int_rsl& a, unsigned long long b) { return a - sc_int_rul(b); }

sc_inline const sc_int_rul operator-(const sc_int_rul& a, bool               b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rul operator-(const sc_int_rul& a, signed char        b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rul operator-(const sc_int_rul& a, unsigned char      b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rul operator-(const sc_int_rul& a, signed short       b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rul operator-(const sc_int_rul& a, unsigned short     b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rul operator-(const sc_int_rul& a, signed int         b) { return a - sc_int_rsi(b); }
sc_inline const sc_int_rul operator-(const sc_int_rul& a, unsigned int       b) { return a - sc_int_rui(b); }
sc_inline const sc_int_rul operator-(const sc_int_rul& a, signed long long   b) { return a - sc_int_rsl(b); }
sc_inline const sc_int_rul operator-(const sc_int_rul& a, unsigned long long b) { return a - sc_int_rul(b); }

sc_inline const sc_int_rsi operator-(bool               a, const sc_int_rsi& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rsi operator-(signed char        a, const sc_int_rsi& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rsi operator-(unsigned char      a, const sc_int_rsi& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rsi operator-(signed short       a, const sc_int_rsi& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rsi operator-(unsigned short     a, const sc_int_rsi& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rsi operator-(signed int         a, const sc_int_rsi& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rui operator-(unsigned int       a, const sc_int_rsi& b) { return sc_int_rui(a) - b; }
sc_inline const sc_int_rsl operator-(signed long long   a, const sc_int_rsi& b) { return sc_int_rsl(a) - b; }
sc_inline const sc_int_rul operator-(unsigned long long a, const sc_int_rsi& b) { return sc_int_rul(a) - b; }

sc_inline const sc_int_rui operator-(bool               a, const sc_int_rui& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rui operator-(signed char        a, const sc_int_rui& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rui operator-(unsigned char      a, const sc_int_rui& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rui operator-(signed short       a, const sc_int_rui& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rui operator-(unsigned short     a, const sc_int_rui& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rui operator-(signed int         a, const sc_int_rui& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rui operator-(unsigned int       a, const sc_int_rui& b) { return sc_int_rui(a) - b; }
sc_inline const sc_int_rsl operator-(signed long long   a, const sc_int_rui& b) { return sc_int_rsl(a) - b; }
sc_inline const sc_int_rul operator-(unsigned long long a, const sc_int_rui& b) { return sc_int_rul(a) - b; }

sc_inline const sc_int_rsl operator-(bool               a, const sc_int_rsl& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rsl operator-(signed char        a, const sc_int_rsl& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rsl operator-(unsigned char      a, const sc_int_rsl& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rsl operator-(signed short       a, const sc_int_rsl& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rsl operator-(unsigned short     a, const sc_int_rsl& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rsl operator-(signed int         a, const sc_int_rsl& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rsl operator-(unsigned int       a, const sc_int_rsl& b) { return sc_int_rui(a) - b; }
sc_inline const sc_int_rsl operator-(signed long long   a, const sc_int_rsl& b) { return sc_int_rsl(a) - b; }
sc_inline const sc_int_rul operator-(unsigned long long a, const sc_int_rsl& b) { return sc_int_rul(a) - b; }

sc_inline const sc_int_rul operator-(bool               a, const sc_int_rul& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rul operator-(signed char        a, const sc_int_rul& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rul operator-(unsigned char      a, const sc_int_rul& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rul operator-(signed short       a, const sc_int_rul& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rul operator-(unsigned short     a, const sc_int_rul& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rul operator-(signed int         a, const sc_int_rul& b) { return sc_int_rsi(a) - b; }
sc_inline const sc_int_rul operator-(unsigned int       a, const sc_int_rul& b) { return sc_int_rui(a) - b; }
sc_inline const sc_int_rul operator-(signed long long   a, const sc_int_rul& b) { return sc_int_rsl(a) - b; }
sc_inline const sc_int_rul operator-(unsigned long long a, const sc_int_rul& b) { return sc_int_rul(a) - b; }

template <typename LS, bool LWIDE, typename RS, bool RWIDE>
sc_inline const sc_int_ranged<LS,LWIDE> operator<<(const sc_int_ranged<LS,LWIDE>& a, const sc_int_ranged<RS,RWIDE>& b) {
  typedef typename sc_int_ranged<LS,LWIDE>::uvalue_type luvalue_type;
  typedef typename sc_int_ranged<RS,RWIDE>::uvalue_type ruvalue_type;
  typedef typename sc_int_ranged<LS,LWIDE>::value_type  lvalue_type;
  typedef typename sc_int_ranged<RS,RWIDE>::value_type  rvalue_type;
  ruvalue_type maxsft = b.maxval_as_shiftcount();
  bool overflow = (maxsft >= sizeof(luvalue_type)*8) || (a.width() > static_cast<int>(sizeof(luvalue_type)*8 - maxsft));
  luvalue_type val = static_cast<lvalue_type>(a) << static_cast<rvalue_type>(b);
  luvalue_type minval = overflow ? static_cast<luvalue_type>(sc_int_traits<LS,(LWIDE?64:32)>::MINVAL) : (a.m_minval << maxsft);
  luvalue_type range  = overflow ? static_cast<luvalue_type>(-1)                                      : (a.m_range  << maxsft);
  return sc_int_ranged<LS,LWIDE>(val, minval, range);
}

template <typename LS, bool LWIDE, typename RS, bool RWIDE>
sc_inline const sc_int_ranged<LS,LWIDE> operator>>(const sc_int_ranged<LS,LWIDE>& a, const sc_int_ranged<RS,RWIDE>& b) {
  typedef typename sc_int_ranged<LS,LWIDE>::uvalue_type luvalue_type;
  typedef typename sc_int_ranged<RS,RWIDE>::uvalue_type ruvalue_type;
  typedef typename sc_int_ranged<LS,LWIDE>::value_type  lvalue_type;
  typedef typename sc_int_ranged<RS,RWIDE>::value_type  rvalue_type;
  ruvalue_type minsft = b.minval_as_shiftcount();
  bool overflow = (minsft >= sizeof(luvalue_type)*8) || a.overflow();
  luvalue_type val = static_cast<lvalue_type>(a) >> static_cast<rvalue_type>(b);
  luvalue_type minval = (overflow
			 ? static_cast<luvalue_type>(sc_int_traits<LS,(LWIDE?64:32)>::MINVAL)
			 : (static_cast<lvalue_type>(a.m_minval) >> minsft));
  luvalue_type range = (overflow
			? static_cast<luvalue_type>(-1)
			: ((static_cast<lvalue_type>(a.m_minval+a.m_range) >> minsft) - minval));
  return sc_int_ranged<LS,LWIDE>(val, minval, range);
}

template <typename S, bool WIDE> sc_inline const sc_int_ranged<S,WIDE>
operator<<(const sc_int_ranged<S,WIDE>& a, bool               b) { return a << sc_int_rsi(b); }
template <typename S, bool WIDE> sc_inline const sc_int_ranged<S,WIDE>
operator<<(const sc_int_ranged<S,WIDE>& a, int                b) { return a << sc_int_rsi(b); }
template <typename S, bool WIDE> sc_inline const sc_int_ranged<S,WIDE>
operator<<(const sc_int_ranged<S,WIDE>& a, unsigned int       b) { return a << sc_int_rui(b); }
template <typename S, bool WIDE> sc_inline const sc_int_ranged<S,WIDE>
operator<<(const sc_int_ranged<S,WIDE>& a, signed long long   b) { return a << sc_int_rsl(b); }
template <typename S, bool WIDE> sc_inline const sc_int_ranged<S,WIDE>
operator<<(const sc_int_ranged<S,WIDE>& a, unsigned long long b) { return a << sc_int_rul(b); }
template <typename S, bool WIDE>
sc_inline const sc_int_rsi operator<<(bool               a, const sc_int_ranged<S,WIDE>& b) { return sc_int_rsi(a) << b; }
template <typename S, bool WIDE>
sc_inline const sc_int_rsi operator<<(signed char        a, const sc_int_ranged<S,WIDE>& b) { return sc_int_rsi(a) << b; }
template <typename S, bool WIDE>
sc_inline const sc_int_rsi operator<<(unsigned char      a, const sc_int_ranged<S,WIDE>& b) { return sc_int_rsi(a) << b; }
template <typename S, bool WIDE>
sc_inline const sc_int_rsi operator<<(signed short       a, const sc_int_ranged<S,WIDE>& b) { return sc_int_rsi(a) << b; }
template <typename S, bool WIDE>
sc_inline const sc_int_rsi operator<<(unsigned short     a, const sc_int_ranged<S,WIDE>& b) { return sc_int_rsi(a) << b; }
template <typename S, bool WIDE>
sc_inline const sc_int_rsi operator<<(signed int         a, const sc_int_ranged<S,WIDE>& b) { return sc_int_rsi(a) << b; }
template <typename S, bool WIDE>
sc_inline const sc_int_rui operator<<(unsigned int       a, const sc_int_ranged<S,WIDE>& b) { return sc_int_rui(a) << b; }
template <typename S, bool WIDE>
sc_inline const sc_int_rsl operator<<(signed long long   a, const sc_int_ranged<S,WIDE>& b) { return sc_int_rsl(a) << b; }
template <typename S, bool WIDE>
sc_inline const sc_int_rul operator<<(unsigned long long a, const sc_int_ranged<S,WIDE>& b) { return sc_int_rul(a) << b; }

template <typename S, bool WIDE> sc_inline const sc_int_ranged<S,WIDE>
operator>>(const sc_int_ranged<S,WIDE>& a, bool               b) { return a >> sc_int_rsi(b); }
template <typename S, bool WIDE> sc_inline const sc_int_ranged<S,WIDE>
operator>>(const sc_int_ranged<S,WIDE>& a, int                b) { return a >> sc_int_rsi(b); }
template <typename S, bool WIDE> sc_inline const sc_int_ranged<S,WIDE>
operator>>(const sc_int_ranged<S,WIDE>& a, unsigned int       b) { return a >> sc_int_rui(b); }
template <typename S, bool WIDE> sc_inline const sc_int_ranged<S,WIDE>
operator>>(const sc_int_ranged<S,WIDE>& a, signed long long   b) { return a >> sc_int_rsl(b); }
template <typename S, bool WIDE> sc_inline const sc_int_ranged<S,WIDE>
operator>>(const sc_int_ranged<S,WIDE>& a, unsigned long long b) { return a >> sc_int_rul(b); }
template <typename S, bool WIDE>
sc_inline const sc_int_rsi operator>>(bool               a, const sc_int_ranged<S,WIDE>& b) { return sc_int_rsi(a) >> b; }
template <typename S, bool WIDE>
sc_inline const sc_int_rsi operator>>(signed char        a, const sc_int_ranged<S,WIDE>& b) { return sc_int_rsi(a) >> b; }
template <typename S, bool WIDE>
sc_inline const sc_int_rsi operator>>(unsigned char      a, const sc_int_ranged<S,WIDE>& b) { return sc_int_rsi(a) >> b; }
template <typename S, bool WIDE>
sc_inline const sc_int_rsi operator>>(signed short       a, const sc_int_ranged<S,WIDE>& b) { return sc_int_rsi(a) >> b; }
template <typename S, bool WIDE>
sc_inline const sc_int_rsi operator>>(unsigned short     a, const sc_int_ranged<S,WIDE>& b) { return sc_int_rsi(a) >> b; }
template <typename S, bool WIDE>
sc_inline const sc_int_rsi operator>>(signed int         a, const sc_int_ranged<S,WIDE>& b) { return sc_int_rsi(a) >> b; }
template <typename S, bool WIDE>
sc_inline const sc_int_rui operator>>(unsigned int       a, const sc_int_ranged<S,WIDE>& b) { return sc_int_rui(a) >> b; }
template <typename S, bool WIDE>
sc_inline const sc_int_rsl operator>>(signed long long   a, const sc_int_ranged<S,WIDE>& b) { return sc_int_rsl(a) >> b; }
template <typename S, bool WIDE>
sc_inline const sc_int_rul operator>>(unsigned long long a, const sc_int_ranged<S,WIDE>& b) { return sc_int_rul(a) >> b; }

////////////////////////////////////////////////////////////////

template <typename S, int W>
struct sc_int_common {
  typedef sc_int_traits<S,W>  traits_type;
  typedef typename traits_type::value_type  value_type;
  typedef sc_int_ranged<S,(W>32)>  ranged_type;
  value_type m_value;
  sc_inline operator value_type() const { return m_value; }
  sc_inline sc_int_common() {}
  template <typename SS, bool WIDE>
  sc_inline static value_type wrap(const sc_int_ranged<SS,WIDE>& x)
    { return traits_type::wrap(x, sc_int_ranged<SS,(WIDE||(W>32))>(x).width(S())); }
  sc_inline sc_int_common(bool               x) : m_value(wrap(sc_int_rsi(x))) {}
  sc_inline sc_int_common(signed char        x) : m_value(wrap(sc_int_rsi(x))) {}
  sc_inline sc_int_common(unsigned char      x) : m_value(wrap(sc_int_rsi(x))) {}
  sc_inline sc_int_common(signed short       x) : m_value(wrap(sc_int_rsi(x))) {}
  sc_inline sc_int_common(unsigned short     x) : m_value(wrap(sc_int_rsi(x))) {}
  sc_inline sc_int_common(signed int         x) : m_value(wrap(sc_int_rsi(x))) {}
  sc_inline sc_int_common(unsigned int       x) : m_value(wrap(sc_int_rui(x))) {}
  sc_inline sc_int_common(signed long long   x) : m_value(wrap(sc_int_rsl(x))) {}
  sc_inline sc_int_common(unsigned long long x) : m_value(wrap(sc_int_rul(x))) {}
  template <typename SS, bool WIDE> sc_inline sc_int_common(const sc_int_ranged<SS,WIDE>& x) : m_value(wrap(x)) {}
  template <int V> sc_inline sc_int_common(const sc_int_common<S,V>& x) : m_value(wrap(x.to_ranged())) {}
  sc_inline const ranged_type to_ranged() const { return ranged_type(m_value, traits_type::MINVAL, traits_type::RANGE); }

  template <typename SS, bool WIDE> struct binop_traits : sc_int_ranged_binop_traits<sc_int_ranged<SS,WIDE>,ranged_type> {};
  typedef typename binop_traits<  signed,false>::result_type binop_si_type;
  typedef typename binop_traits<unsigned,false>::result_type binop_ui_type;
  typedef typename binop_traits<  signed,true >::result_type binop_sl_type;
  typedef typename binop_traits<unsigned,true >::result_type binop_ul_type;
  sc_inline friend const binop_si_type operator+(const sc_int_common& a, bool               b) { return a.to_ranged() + b; }
  sc_inline friend const binop_si_type operator+(const sc_int_common& a, signed char        b) { return a.to_ranged() + b; }
  sc_inline friend const binop_si_type operator+(const sc_int_common& a, unsigned char      b) { return a.to_ranged() + b; }
  sc_inline friend const binop_si_type operator+(const sc_int_common& a, signed short       b) { return a.to_ranged() + b; }
  sc_inline friend const binop_si_type operator+(const sc_int_common& a, unsigned short     b) { return a.to_ranged() + b; }
  sc_inline friend const binop_si_type operator+(const sc_int_common& a, signed int         b) { return a.to_ranged() + b; }
  sc_inline friend const binop_ui_type operator+(const sc_int_common& a, unsigned int       b) { return a.to_ranged() + b; }
  sc_inline friend const binop_sl_type operator+(const sc_int_common& a, signed long long   b) { return a.to_ranged() + b; }
  sc_inline friend const binop_ul_type operator+(const sc_int_common& a, unsigned long long b) { return a.to_ranged() + b; }
  sc_inline friend const binop_si_type operator+(bool               a, const sc_int_common& b) { return a + b.to_ranged(); }
  sc_inline friend const binop_si_type operator+(signed char        a, const sc_int_common& b) { return a + b.to_ranged(); }
  sc_inline friend const binop_si_type operator+(unsigned char      a, const sc_int_common& b) { return a + b.to_ranged(); }
  sc_inline friend const binop_si_type operator+(signed short       a, const sc_int_common& b) { return a + b.to_ranged(); }
  sc_inline friend const binop_si_type operator+(unsigned short     a, const sc_int_common& b) { return a + b.to_ranged(); }
  sc_inline friend const binop_si_type operator+(signed int         a, const sc_int_common& b) { return a + b.to_ranged(); }
  sc_inline friend const binop_ui_type operator+(unsigned int       a, const sc_int_common& b) { return a + b.to_ranged(); }
  sc_inline friend const binop_sl_type operator+(signed long long   a, const sc_int_common& b) { return a + b.to_ranged(); }
  sc_inline friend const binop_ul_type operator+(unsigned long long a, const sc_int_common& b) { return a + b.to_ranged(); }
  template <typename SS, bool WIDE> sc_inline friend const typename binop_traits<SS,WIDE>::result_type
  operator+(const sc_int_common& a, const sc_int_ranged<SS,WIDE>& b) { return a.to_ranged() + b; }
  template <typename SS, bool WIDE> sc_inline friend const typename binop_traits<SS,WIDE>::result_type
  operator+(const sc_int_ranged<SS,WIDE>& a, const sc_int_common& b) { return a + b.to_ranged(); }
  sc_inline friend const binop_si_type operator-(const sc_int_common& a, bool               b) { return a.to_ranged() - b; }
  sc_inline friend const binop_si_type operator-(const sc_int_common& a, signed char        b) { return a.to_ranged() - b; }
  sc_inline friend const binop_si_type operator-(const sc_int_common& a, unsigned char      b) { return a.to_ranged() - b; }
  sc_inline friend const binop_si_type operator-(const sc_int_common& a, signed short       b) { return a.to_ranged() - b; }
  sc_inline friend const binop_si_type operator-(const sc_int_common& a, unsigned short     b) { return a.to_ranged() - b; }
  sc_inline friend const binop_si_type operator-(const sc_int_common& a, signed int         b) { return a.to_ranged() - b; }
  sc_inline friend const binop_ui_type operator-(const sc_int_common& a, unsigned int       b) { return a.to_ranged() - b; }
  sc_inline friend const binop_sl_type operator-(const sc_int_common& a, signed long long   b) { return a.to_ranged() - b; }
  sc_inline friend const binop_ul_type operator-(const sc_int_common& a, unsigned long long b) { return a.to_ranged() - b; }
  sc_inline friend const binop_si_type operator-(bool               a, const sc_int_common& b) { return a - b.to_ranged(); }
  sc_inline friend const binop_si_type operator-(signed char        a, const sc_int_common& b) { return a - b.to_ranged(); }
  sc_inline friend const binop_si_type operator-(unsigned char      a, const sc_int_common& b) { return a - b.to_ranged(); }
  sc_inline friend const binop_si_type operator-(signed short       a, const sc_int_common& b) { return a - b.to_ranged(); }
  sc_inline friend const binop_si_type operator-(unsigned short     a, const sc_int_common& b) { return a - b.to_ranged(); }
  sc_inline friend const binop_si_type operator-(signed int         a, const sc_int_common& b) { return a - b.to_ranged(); }
  sc_inline friend const binop_ui_type operator-(unsigned int       a, const sc_int_common& b) { return a - b.to_ranged(); }
  sc_inline friend const binop_sl_type operator-(signed long long   a, const sc_int_common& b) { return a - b.to_ranged(); }
  sc_inline friend const binop_ul_type operator-(unsigned long long a, const sc_int_common& b) { return a - b.to_ranged(); }
  template <typename SS, bool WIDE> sc_inline friend const typename binop_traits<SS,WIDE>::result_type
  operator-(const sc_int_common& a, const sc_int_ranged<SS,WIDE>& b) { return a.to_ranged() - b; }
  template <typename SS, bool WIDE> sc_inline friend const typename binop_traits<SS,WIDE>::result_type
  operator-(const sc_int_ranged<SS,WIDE>& a, const sc_int_common& b) { return a - b.to_ranged(); }

  sc_inline friend const ranged_type operator<<(const sc_int_common& a, bool               b) { return a.to_ranged() << b; }
  sc_inline friend const ranged_type operator<<(const sc_int_common& a, int                b) { return a.to_ranged() << b; }
  sc_inline friend const ranged_type operator<<(const sc_int_common& a, unsigned int       b) { return a.to_ranged() << b; }
  sc_inline friend const ranged_type operator<<(const sc_int_common& a, signed long long   b) { return a.to_ranged() << b; }
  sc_inline friend const ranged_type operator<<(const sc_int_common& a, unsigned long long b) { return a.to_ranged() << b; }
  sc_inline friend const sc_int_rsi operator<<(bool               a, const sc_int_common& b) { return a << b.to_ranged(); }
  sc_inline friend const sc_int_rsi operator<<(signed char        a, const sc_int_common& b) { return a << b.to_ranged(); }
  sc_inline friend const sc_int_rsi operator<<(unsigned char      a, const sc_int_common& b) { return a << b.to_ranged(); }
  sc_inline friend const sc_int_rsi operator<<(signed short       a, const sc_int_common& b) { return a << b.to_ranged(); }
  sc_inline friend const sc_int_rsi operator<<(unsigned short     a, const sc_int_common& b) { return a << b.to_ranged(); }
  sc_inline friend const sc_int_rsi operator<<(signed int         a, const sc_int_common& b) { return a << b.to_ranged(); }
  sc_inline friend const sc_int_rui operator<<(unsigned int       a, const sc_int_common& b) { return a << b.to_ranged(); }
  sc_inline friend const sc_int_rsl operator<<(signed long long   a, const sc_int_common& b) { return a << b.to_ranged(); }
  sc_inline friend const sc_int_rul operator<<(unsigned long long a, const sc_int_common& b) { return a << b.to_ranged(); }
  template <typename SS, bool WIDE> sc_inline friend const ranged_type
  operator<<(const sc_int_common& a, const sc_int_ranged<SS,WIDE>& b) { return a.to_ranged() << b; }
  template <typename SS, bool WIDE> sc_inline friend const sc_int_ranged<SS,WIDE>
  operator<<(const sc_int_ranged<SS,WIDE>& a, const sc_int_common& b) { return a << b.to_ranged(); }
  sc_inline friend const ranged_type operator>>(const sc_int_common& a, bool               b) { return a.to_ranged() >> b; }
  sc_inline friend const ranged_type operator>>(const sc_int_common& a, int                b) { return a.to_ranged() >> b; }
  sc_inline friend const ranged_type operator>>(const sc_int_common& a, unsigned int       b) { return a.to_ranged() >> b; }
  sc_inline friend const ranged_type operator>>(const sc_int_common& a, signed long long   b) { return a.to_ranged() >> b; }
  sc_inline friend const ranged_type operator>>(const sc_int_common& a, unsigned long long b) { return a.to_ranged() >> b; }
  sc_inline friend const sc_int_rsi operator>>(bool               a, const sc_int_common& b) { return a >> b.to_ranged(); }
  sc_inline friend const sc_int_rsi operator>>(signed char        a, const sc_int_common& b) { return a >> b.to_ranged(); }
  sc_inline friend const sc_int_rsi operator>>(unsigned char      a, const sc_int_common& b) { return a >> b.to_ranged(); }
  sc_inline friend const sc_int_rsi operator>>(signed short       a, const sc_int_common& b) { return a >> b.to_ranged(); }
  sc_inline friend const sc_int_rsi operator>>(unsigned short     a, const sc_int_common& b) { return a >> b.to_ranged(); }
  sc_inline friend const sc_int_rsi operator>>(signed int         a, const sc_int_common& b) { return a >> b.to_ranged(); }
  sc_inline friend const sc_int_rui operator>>(unsigned int       a, const sc_int_common& b) { return a >> b.to_ranged(); }
  sc_inline friend const sc_int_rsl operator>>(signed long long   a, const sc_int_common& b) { return a >> b.to_ranged(); }
  sc_inline friend const sc_int_rul operator>>(unsigned long long a, const sc_int_common& b) { return a >> b.to_ranged(); }
  template <typename SS, bool WIDE> sc_inline friend const ranged_type
  operator>>(const sc_int_common& a, const sc_int_ranged<SS,WIDE>& b) { return a.to_ranged() >> b; }
  template <typename SS, bool WIDE> sc_inline friend const sc_int_ranged<SS,WIDE>
  operator>>(const sc_int_ranged<SS,WIDE>& a, const sc_int_common& b) { return a >> b.to_ranged(); }

  template <typename T> sc_inline sc_int_common& operator+=(T val)  { return *this = *this + val; }
  template <typename T> sc_inline sc_int_common& operator-=(T val)  { return *this = *this - val; }
  template <typename T> sc_inline sc_int_common& operator*=(T val)  { return *this = *this * val; }
  template <typename T> sc_inline sc_int_common& operator/=(T val)  { return *this = *this / val; }
  template <typename T> sc_inline sc_int_common& operator%=(T val)  { return *this = *this % val; }
  template <typename T> sc_inline sc_int_common& operator&=(T val)  { return *this = *this & val; }
  template <typename T> sc_inline sc_int_common& operator|=(T val)  { return *this = *this | val; }
  template <typename T> sc_inline sc_int_common& operator^=(T val)  { return *this = *this ^ val; }
  template <typename T> sc_inline sc_int_common& operator<<=(T val) { return *this = *this << val; }
  template <typename T> sc_inline sc_int_common& operator>>=(T val) { return *this = *this >> val; }
  sc_inline sc_int_common& operator++() { return *this += 1; }
  sc_inline sc_int_common& operator--() { return *this -= 1; }
  sc_inline const sc_int_common operator++(int) { sc_int_common ret = *this;  ++(*this);  return ret; }
  sc_inline const sc_int_common operator--(int) { sc_int_common ret = *this;  --(*this);  return ret; }
  sc_inline const sc_int_bitref<S,W> operator[](int pos) { return sc_int_bitref<S,W>(*this, pos); }
  sc_inline const sc_int_bitref_r operator[](int pos) const { return sc_int_bitref_r((m_value>>pos)&1); }
  sc_inline const sc_int_subref<S,W> operator()(int msb, int lsb) { return sc_int_subref<S,W>(*this, msb, lsb); }
  sc_inline const sc_int_subref_r operator()(int msb, int lsb) const {
    int size = msb - lsb + 1;
    value_type mask = (1LL << size) - 1;
    return sc_int_subref_r((m_value>>lsb) & mask, size);
  }
  sc_inline void set_bit(int pos, bool val) {
    value_type mask = static_cast<value_type>(1) << pos;
    value_type newval;
    if (__builtin_constant_p(val)) {
      newval = val ? (m_value | mask) : (m_value & ~mask);
    } else {
      newval = (m_value & ~mask) | (static_cast<value_type>(val) << pos);
    }
    m_value = traits_type::adjust_setbit(newval, pos);
  }
  sc_inline void set_sub(int msb, int lsb, value_type value) {
    int size = msb - lsb + 1;
    value_type mask = (static_cast<value_type>(1) << size) - 1;
    value &= mask;
    mask <<= lsb;
    value_type newval = (m_value & ~mask) | (value << lsb);
    m_value = traits_type::adjust_setbit(newval, msb);
  }
  sc_inline friend const sc_int_ssref_r operator,(bool lhs, const sc_int_common& rhs)
    { return (sc_int_subref_r(lhs), rhs.to_subref_r()); }
  sc_inline friend const sc_int_ssref_r operator,(const sc_int_bitref_r& lhs, const sc_int_common& rhs)
    { return (sc_int_subref_r(lhs), rhs.to_subref_r()); }
  sc_inline friend const sc_int_ssref_r operator,(const sc_int_subref_r& lhs, const sc_int_common& rhs)
    { return (lhs, rhs.to_subref_r()); }
  sc_inline friend const sc_int_ssref_r operator,(const sc_int_common& lhs, bool rhs)
    { return (lhs.to_subref_r(), sc_int_subref_r(rhs)); }
  sc_inline friend const sc_int_ssref_r operator,(const sc_int_common& lhs, const sc_int_bitref_r& rhs)
    { return (lhs.to_subref_r(), sc_int_subref_r(rhs)); }
  sc_inline friend const sc_int_ssref_r operator,(const sc_int_common& lhs, const sc_int_subref_r& rhs)
    { return (lhs.to_subref_r(), rhs); }
  // API for sc_int_catref
  sc_inline int size() const { return W; }
  sc_inline const sc_int_subref_r to_subref_r() const { return (*this)(W-1,0); }
  typedef sc_int_subref_r catref_r_type;
  sc_inline const catref_r_type to_catref_r() const { return to_subref_r(); }
};

template <typename A, typename D, typename S2, int W2>
sc_inline const sc_int_catref_r< sc_int_catref_r<A,D>,sc_int_subref_r >
operator,(const sc_int_catref_r<A,D>& lhs, const sc_int_common<S2,W2>& rhs) { return (lhs, rhs.to_subref_r()); }
template <typename S1, int W1, typename A, typename D>
sc_inline const sc_int_catref_r< sc_int_subref_r,sc_int_catref_r<A,D> >
operator,(const sc_int_common<S1,W1>& lhs, const sc_int_catref_r<A,D>& rhs) { return (lhs.to_subref_r(), rhs); }

template <typename S1, int W1, typename S2, int W2>
sc_inline const sc_int_ssref_r operator,(const sc_int_common<S1,W1>& lhs, const sc_int_bitref<S2,W2>& rhs)
  { return (lhs.to_subref_r(), rhs.to_subref_r()); }
template <typename S1, int W1, typename S2, int W2>
sc_inline const sc_int_ssref_r operator,(const sc_int_bitref<S2,W2>& lhs, const sc_int_common<S1,W1>& rhs)
  { return (lhs.to_subref_r(), rhs.to_subref_r()); }
template <typename S1, int W1, typename S2, int W2>
sc_inline const sc_int_ssref_r operator,(const sc_int_common<S1,W1>& lhs, const sc_int_subref<S2,W2>& rhs)
  { return (lhs.to_subref_r(), rhs.to_subref_r()); }
template <typename S1, int W1, typename S2, int W2>
sc_inline const sc_int_ssref_r operator,(const sc_int_subref<S1,W1>& lhs, const sc_int_common<S2,W2>& rhs)
  { return (lhs.to_subref_r(), rhs.to_subref_r()); }
template <typename S1, int W1, typename S2, int W2>
sc_inline const sc_int_ssref_r operator,(const sc_int_common<S1,W1>& lhs, const sc_int_common<S2,W2>& rhs)
  { return (lhs.to_subref_r(), rhs.to_subref_r()); }

template <typename S1, int W1, typename S2, int W2>
sc_inline const typename sc_int_ranged_binop_traits<typename sc_int_common<S1,W1>::ranged_type,
						    typename sc_int_common<S2,W2>::ranged_type>::result_type
operator+(const sc_int_common<S1,W1>& a, const sc_int_common<S2,W2>& b) { return a.to_ranged() + b.to_ranged(); }
template <typename S1, int W1, typename S2, int W2>
sc_inline const typename sc_int_ranged_binop_traits<typename sc_int_common<S1,W1>::ranged_type,
						    typename sc_int_common<S2,W2>::ranged_type>::result_type
operator-(const sc_int_common<S1,W1>& a, const sc_int_common<S2,W2>& b) { return a.to_ranged() - b.to_ranged(); }
template <typename S1, int W1, typename S2, int W2>
sc_inline const typename sc_int_common<S1,W1>::ranged_type
operator<<(const sc_int_common<S1,W1>& a, const sc_int_common<S2,W2>& b) { return a.to_ranged() << b.to_ranged(); }
template <typename S1, int W1, typename S2, int W2>
sc_inline const typename sc_int_common<S1,W1>::ranged_type
operator>>(const sc_int_common<S1,W1>& a, const sc_int_common<S2,W2>& b) { return a.to_ranged() >> b.to_ranged(); }

////////////////////////////////////////////////////////////////

template <int W> struct sc_biguint;
template <int W> struct sc_biguint_subref_r;

template <typename A, typename D> struct sc_int_catref {
  A& m_car;
  D& m_cdr;
  sc_inline sc_int_catref(A& car, D& cdr) : m_car(car), m_cdr(cdr) {}
  sc_inline const sc_int_catref& operator=(unsigned long long value) const
    { m_cdr=value; m_car=(value>>m_cdr.size()); return *this; }
  sc_inline const sc_int_catref& operator=(const sc_int_catref& rhs) const
    { return operator=(static_cast<unsigned long long>(rhs)); }
  template <int W> sc_inline const sc_int_catref& operator=(const sc_biguint<W>& rhs) const;
  template <int W> sc_inline const sc_int_catref& operator=(const sc_biguint_subref_r<W>& rhs) const;
  template <typename T> sc_inline const sc_int_catref& operator+=(T val) const  { return *this = *this + val; }
  template <typename T> sc_inline const sc_int_catref& operator-=(T val) const  { return *this = *this - val; }
  template <typename T> sc_inline const sc_int_catref& operator*=(T val) const  { return *this = *this * val; }
  template <typename T> sc_inline const sc_int_catref& operator/=(T val) const  { return *this = *this / val; }
  template <typename T> sc_inline const sc_int_catref& operator%=(T val) const  { return *this = *this % val; }
  template <typename T> sc_inline const sc_int_catref& operator&=(T val) const  { return *this = *this & val; }
  template <typename T> sc_inline const sc_int_catref& operator|=(T val) const  { return *this = *this | val; }
  template <typename T> sc_inline const sc_int_catref& operator^=(T val) const  { return *this = *this ^ val; }
  template <typename T> sc_inline const sc_int_catref& operator<<=(T val) const { return *this = *this << val; }
  template <typename T> sc_inline const sc_int_catref& operator>>=(T val) const { return *this = *this >> val; }
  sc_inline const sc_int_catref& operator++() const { return *this += 1; }
  sc_inline const sc_int_catref& operator--() const { return *this -= 1; }
  sc_inline const sc_int_catref operator++(int) const { sc_int_catref ret = *this;  ++(*this);  return ret; }
  sc_inline const sc_int_catref operator--(int) const { sc_int_catref ret = *this;  --(*this);  return ret; }
  typedef sc_int_catref_r<typename A::catref_r_type, typename D::catref_r_type> catref_r_type;
  typedef sc_int_catref_r<catref_r_type,sc_int_subref_r> sc_int_csref_r;
  typedef sc_int_catref_r<sc_int_subref_r,catref_r_type> sc_int_scref_r;
  sc_inline friend const sc_int_csref_r operator,(const sc_int_catref& lhs, bool rhs)
    { return (lhs.to_catref_r(), sc_int_subref_r(rhs)); }
  sc_inline friend const sc_int_csref_r operator,(const sc_int_catref& lhs, const sc_int_bitref_r& rhs)
    { return (lhs.to_catref_r(), sc_int_subref_r(rhs)); }
  sc_inline friend const sc_int_csref_r operator,(const sc_int_catref& lhs, const sc_int_subref_r& rhs)
    { return (lhs.to_catref_r(), rhs); }
  sc_inline friend const sc_int_scref_r operator,(bool lhs, const sc_int_catref& rhs)
    { return (sc_int_subref_r(lhs), rhs.to_catref_r()); }
  sc_inline friend const sc_int_scref_r operator,(const sc_int_bitref_r& lhs, const sc_int_catref& rhs)
    { return (sc_int_subref_r(lhs), rhs.to_catref_r()); }
  sc_inline friend const sc_int_scref_r operator,(const sc_int_subref_r& lhs, const sc_int_catref& rhs)
    { return (lhs, rhs.to_catref_r()); }
  sc_inline operator unsigned long long() const { return to_catref_r(); }
  sc_inline int size() const { return m_car.size() + m_cdr.size(); }
  sc_inline const catref_r_type to_catref_r() const { return (m_car.to_catref_r(), m_cdr.to_catref_r()); }
};

template <typename A, typename D, typename S2, int W2>
sc_inline const sc_int_catref_r< typename sc_int_catref<A,D>::catref_r_type, sc_int_subref_r >
operator,(const sc_int_catref<A,D>& lhs, const sc_int_common<S2,W2>& rhs) { return (lhs.to_catref_r(), rhs.to_subref_r()); }
template <typename S1, int W1, typename A, typename D>
sc_inline const sc_int_catref_r< sc_int_subref_r, typename sc_int_catref<A,D>::catref_r_type >
operator,(const sc_int_common<S1,W1>& lhs, const sc_int_catref<A,D>& rhs) { return (lhs.to_subref_r(), rhs.to_catref_r()); }
template <typename A1, typename D1, typename A2, typename D2>
sc_inline const sc_int_catref_r< typename sc_int_catref<A1,D1>::catref_r_type, sc_int_catref_r<A2,D2> >
operator,(const sc_int_catref<A1,D1>& lhs, const sc_int_catref_r<A2,D2>& rhs) { return (lhs.to_catref_r(), rhs); }
template <typename A1, typename D1, typename A2, typename D2>
sc_inline const sc_int_catref_r< sc_int_catref_r<A1,D1>, typename sc_int_catref<A2,D2>::catref_r_type >
operator,(const sc_int_catref_r<A1,D1>& lhs, const sc_int_catref<A2,D2>& rhs) { return (lhs, rhs.to_catref_r()); }

template <typename S1, int W1, typename S2, int W2>
sc_inline const sc_int_catref< const sc_int_bitref<S1,W1>, const sc_int_bitref<S2,W2> >
operator,(const sc_int_bitref<S1,W1>& lhs, const sc_int_bitref<S2,W2>& rhs)
  { return sc_int_catref< const sc_int_bitref<S1,W1>, const sc_int_bitref<S2,W2> > (lhs, rhs); }
template <typename S1, int W1, typename S2, int W2>
sc_inline const sc_int_catref< const sc_int_bitref<S1,W1>, const sc_int_subref<S2,W2> >
operator,(const sc_int_bitref<S1,W1>& lhs, const sc_int_subref<S2,W2>& rhs)
  { return sc_int_catref< const sc_int_bitref<S1,W1>, const sc_int_subref<S2,W2> > (lhs, rhs); }
template <typename S1, int W1, typename S2, int W2>
sc_inline const sc_int_catref< const sc_int_bitref<S1,W1>, sc_int_common<S2,W2> >
operator,(const sc_int_bitref<S1,W1>& lhs, sc_int_common<S2,W2>& rhs)
  { return sc_int_catref< const sc_int_bitref<S1,W1>, sc_int_common<S2,W2> > (lhs, rhs); }
template <typename S1, int W1, typename A, typename D>
sc_inline const sc_int_catref< const sc_int_bitref<S1,W1>, const sc_int_catref<A,D> >
operator,(const sc_int_bitref<S1,W1>& lhs, const sc_int_catref<A,D>& rhs)
  { return sc_int_catref< const sc_int_bitref<S1,W1>, const sc_int_catref<A,D> > (lhs, rhs); }
template <typename S1, int W1, typename S2, int W2>
sc_inline const sc_int_catref< const sc_int_subref<S1,W1>, const sc_int_bitref<S2,W2> >
operator,(const sc_int_subref<S1,W1>& lhs, const sc_int_bitref<S2,W2>& rhs)
  { return sc_int_catref< const sc_int_subref<S1,W1>, const sc_int_bitref<S2,W2> > (lhs, rhs); }
template <typename S1, int W1, typename S2, int W2>
sc_inline const sc_int_catref< const sc_int_subref<S1,W1>, const sc_int_subref<S2,W2> >
operator,(const sc_int_subref<S1,W1>& lhs, const sc_int_subref<S2,W2>& rhs)
  { return sc_int_catref< const sc_int_subref<S1,W1>, const sc_int_subref<S2,W2> > (lhs, rhs); }
template <typename S1, int W1, typename S2, int W2>
sc_inline const sc_int_catref< const sc_int_subref<S1,W1>, sc_int_common<S2,W2> >
operator,(const sc_int_subref<S1,W1>& lhs, sc_int_common<S2,W2>& rhs)
  { return sc_int_catref< const sc_int_subref<S1,W1>, sc_int_common<S2,W2> > (lhs, rhs); }
template <typename S1, int W1, typename A, typename D>
sc_inline const sc_int_catref< const sc_int_subref<S1,W1>, const sc_int_catref<A,D> >
operator,(const sc_int_subref<S1,W1>& lhs, const sc_int_catref<A,D>& rhs)
  { return sc_int_catref< const sc_int_subref<S1,W1>, const sc_int_catref<A,D> > (lhs, rhs); }
template <typename S1, int W1, typename S2, int W2>
sc_inline const sc_int_catref< sc_int_common<S1,W1>, const sc_int_bitref<S2,W2> >
operator,(sc_int_common<S1,W1>& lhs, const sc_int_bitref<S2,W2>& rhs)
  { return sc_int_catref< sc_int_common<S1,W1>, const sc_int_bitref<S2,W2> > (lhs, rhs); }
template <typename S1, int W1, typename S2, int W2>
sc_inline const sc_int_catref< sc_int_common<S1,W1>, const sc_int_subref<S2,W2> >
operator,(sc_int_common<S1,W1>& lhs, const sc_int_subref<S2,W2>& rhs)
  { return sc_int_catref< sc_int_common<S1,W1>, const sc_int_subref<S2,W2> > (lhs, rhs); }
template <typename S1, int W1, typename S2, int W2>
sc_inline const sc_int_catref< sc_int_common<S1,W1>, sc_int_common<S2,W2> >
operator,(sc_int_common<S1,W1>& lhs, sc_int_common<S2,W2>& rhs)
  { return sc_int_catref< sc_int_common<S1,W1>, sc_int_common<S2,W2> > (lhs, rhs); }
template <typename S1, int W1, typename A, typename D>
sc_inline const sc_int_catref< sc_int_common<S1,W1>, const sc_int_catref<A,D> >
operator,(sc_int_common<S1,W1>& lhs, const sc_int_catref<A,D>& rhs)
  { return sc_int_catref< sc_int_common<S1,W1>, const sc_int_catref<A,D> > (lhs, rhs); }
template <typename A, typename D, typename S2, int W2>
sc_inline const sc_int_catref< const sc_int_catref<A,D>, const sc_int_bitref<S2,W2> >
operator,(const sc_int_catref<A,D>& lhs, const sc_int_bitref<S2,W2>& rhs)
  { return sc_int_catref< const sc_int_catref<A,D>, const sc_int_bitref<S2,W2> > (lhs, rhs); }
template <typename A, typename D, typename S2, int W2>
sc_inline const sc_int_catref< const sc_int_catref<A,D>, const sc_int_subref<S2,W2> >
operator,(const sc_int_catref<A,D>& lhs, const sc_int_subref<S2,W2>& rhs)
  { return sc_int_catref< const sc_int_catref<A,D>, const sc_int_subref<S2,W2> > (lhs, rhs); }
template <typename A, typename D, typename S2, int W2>
sc_inline const sc_int_catref< const sc_int_catref<A,D>, sc_int_common<S2,W2> >
operator,(const sc_int_catref<A,D>& lhs, sc_int_common<S2,W2>& rhs)
  { return sc_int_catref< const sc_int_catref<A,D>, sc_int_common<S2,W2> > (lhs, rhs); }
template <typename A1, typename D1, typename A2, typename D2>
sc_inline const sc_int_catref< const sc_int_catref<A1, D1>, const sc_int_catref<A2,D2> >
operator,(const sc_int_catref<A1,D1>& lhs, const sc_int_catref<A2,D2>& rhs)
  { return sc_int_catref< const sc_int_catref<A1,D1>, const sc_int_catref<A2,D2> > (lhs, rhs); }

////////////////////////////////////////////////////////////////

// Define sc_int<W> is an alias of sc_int_common<signed,W>,
//   and sc_uint<W> as an alias of sc_int_common<unsigned,W>.
// In C++0x, we will need only 2 lines of code using "template typedef" syntax.

template <int W> struct sc_int : sc_int_common<signed,W> {
  typedef sc_int_common<signed,W>  base_type;
  sc_inline sc_int() : base_type() {}
  template <typename T> sc_inline sc_int(T x) : base_type(x) {}
};

template <int W> struct sc_uint : sc_int_common<unsigned,W> {
  typedef sc_int_common<unsigned,W>  base_type;
  sc_inline sc_uint() : base_type() {}
  template <typename T> sc_inline sc_uint(T x) : base_type(x) {}
};

////////////////////////////////////////////////////////////////

template <int W, bool WIDE> struct sc_biguint_base;

template <int W> struct sc_biguint_base<W,false> : sc_uint<W> {
  using sc_uint<W>::operator=;
  template <typename T>
  sc_inline void bigassign_recur(const T& rhs, unsigned long long rbits, int nr_rbits) {
    if (nr_rbits == 0) {
      *this = rhs;
    } else {
      *this = (rhs, sc_int_subref_r(rbits, nr_rbits));
    }
  }
};

template <int W> struct sc_biguint_base<W,true> {
  sc_biguint_base<W-64,(W>128)>  m_high;
  sc_uint<64>                    m_low;
  sc_inline void bigassign_recur(unsigned long long rhs, unsigned long long rbits, int nr_rbits) {
    m_low = (rhs << nr_rbits) | rbits;
    m_high.bigassign_recur(rhs >> (64 - nr_rbits), 0ull, 0);
  }
  template <typename A, typename D>
  sc_inline void bigassign_recur(const sc_int_catref_r<A,D>& rhs, unsigned long long rbits, int nr_rbits) {
    int sz = rhs.m_cdr.size();
    rbits |= (rhs.m_cdr << nr_rbits);
    int nr_lbits = 64 - nr_rbits;
    if (sz < nr_lbits) {
      bigassign_recur(rhs.m_car, rbits, nr_rbits + sz);
    } else {
      m_low = rbits;
      m_high.bigassign_recur(rhs.m_car, (sz==nr_lbits) ? 0 : (rhs.m_cdr>>nr_lbits), sz-nr_lbits);
    }
  }
  template <typename A, typename D>
  sc_inline sc_biguint_base& operator=(const sc_int_catref_r<A,D>& rhs) { bigassign_recur(rhs, 0ull, 0); return *this; }
  template <typename A, typename D>
  sc_inline sc_biguint_base& operator=(const sc_int_catref<A,D>& rhs) { return operator=(rhs.to_catref_r()); }
};

template <int W> struct sc_biguint : sc_biguint_base<W,(W>64)> { using sc_biguint_base<W,(W>64)>::operator=; };

template <int W, bool WIDE> struct sc_biguint_subref_r_aux;
template <int W> struct sc_biguint_subref_r_aux<W,false> {
  sc_inline static unsigned long long to_ull(const sc_biguint_base<W,false>& obj, int msb, int lsb) { return obj(msb, lsb); }
};
template <int W> struct sc_biguint_subref_r_aux<W,true> {
  sc_inline static unsigned long long to_ull(const sc_biguint_base<W,true>& obj, int msb, int lsb) {
    if (lsb >= 64) {
      return sc_biguint_subref_r_aux<W-64,(W>128)>::to_ull(obj.m_high, msb-64, lsb-64);
    } else if (msb < 64) {
      return obj.m_low(msb, lsb);
    } else {
      unsigned long long high = sc_biguint_subref_r_aux<W-64,(W>128)>::to_ull(obj.m_high, msb-64, 0);
      unsigned long long low = obj.m_low(63, lsb);
      return (high << (64-lsb)) | low;
    }
  }
};

template <int W> struct sc_biguint_subref_r {
  typedef sc_biguint<W> obj_type;
  const obj_type& m_obj;
  int             m_msb, m_lsb;
  sc_inline sc_biguint_subref_r(const obj_type& obj, int msb, int lsb) : m_obj(obj), m_msb(msb), m_lsb(lsb) {}
  sc_inline const sc_biguint_subref_r operator()(int msb, int lsb) const
    { return sc_biguint_subref_r(m_obj, m_lsb+msb, m_lsb+lsb); }
  sc_inline operator unsigned long long() const { return sc_biguint_subref_r_aux<W,(W>64)>::to_ull(m_obj, m_msb, m_lsb); }
  template <typename S, int V>
  sc_inline operator sc_int_common<S,V>() const { return static_cast<unsigned long long>(*this); }
};

template <typename A, typename D> template <int W>
sc_inline const sc_int_catref<A,D>& sc_int_catref<A,D>::operator=(const sc_biguint_subref_r<W>& rhs) const {
  int sz_cdr = m_cdr.size();
  int sz_car = m_car.size();
  m_cdr = rhs(sz_cdr-1, 0);
  m_car = rhs(sz_cdr+sz_car-1, sz_cdr);
  return *this;
}

template <typename A, typename D> template <int W>
sc_inline const sc_int_catref<A,D>& sc_int_catref<A,D>::operator=(const sc_biguint<W>& rhs) const {
  return operator=(sc_biguint_subref_r<W>(rhs, W-1, 0));
}

#undef sc_inline

} // namespace sc_dt

#endif
