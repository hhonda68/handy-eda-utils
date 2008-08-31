// -*- c++ -*-

//
// Lightweight restricted-width integer library for C++.
//

// Copyright (c) 2008 the handy-eda-utils develeper(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

// Examples:
//
//   int10 a = fx_imm(-123);     // set immediate constant
//   uint10 b = fx_imm(999);
//   int10 c0 = fx_add(a,b);
//   int10 c1 = fx_sub(a,b);
//   int10 c2 = fx_mul(a,b);
//   int10 c3 = fx_div(a,b);
//   int10 c4 = fx_rem(a,b);
//   int10 c5 = fx_and(a,b);     // bitwise and
//   int10 c6 = fx_or(a,b);      // bitwise or
//   int10 c7 = fx_xor(a,b);     // bitwise xor
//   int10 d0 = fx_lsft(a,3);    // left shift
//   int10 d1 = fx_rsft(a,3);    // right shift
//   int10 d2 = fx_neg(a);       // sign negation
//   int10 d3 = fx_squ(a);       // square
//   int10 d4 = fx_not(a);       // bitwise complement
//   int12 d5 = fx_cnv(a);       // format conversion
//   int7  d6 = fx_wrap(a);      // wraparound w/o overflow warning
//   int7  d7 = fx_clamp(a);     // saturate
//   int7  d8 = fx_wrap(a+b);
//   int7  d9 = fx_clamp(a+b);
//   bool j0 = (a == b);
//   bool j1 = (a != b);
//   bool j2 = (a >= b);
//   bool j3 = (a <= b);
//   bool j4 = (a > b);
//   bool j5 = (a < b);
//
// Unrecommended Examples:
//   int8 a = -12345;
//   int4 c = a + b;
//   int16 d = a + int4(99);
//   This coding style results in unfriendly error messages upon overflow.
//
// Customization:
//
//   fx::value_type  ... internal representation
//      (signed int or signd long long)
//  
//   #define FX_CHECK_OVERFLOW
//     Check overflow upon assignments w.r.t. "width"
//     (not upon operations w.r.t. sizeof(value_type))

#ifndef FX_H_INCLUDED
#define FX_H_INCLUDED

#define FX_CHECK_OVERFLOW

namespace fx {

typedef int value_type;   // or "long long"

template <int LINE>
struct loc {
#ifdef FX_CHECK_OVERFLOW
  const char * const file;
  loc(const char *file_ = 0) : file(file_) {}
#else
  loc(const char *file_ = 0) {}
#endif
};

enum rhs_tag { Wrap, Clamp };

template <rhs_tag TAG>
struct rhs_tagged {
  const value_type value;
  rhs_tagged(value_type value_) : value(value_) {}
};

template <int LINE>
struct rhs {
  const value_type value;
  const loc<LINE>& m_loc;
  rhs(value_type value_, const loc<LINE>& loc_) : value(value_), m_loc(loc_) {}
  rhs_tagged<Wrap>  wrap()  { return rhs_tagged<Wrap> (value); }
  rhs_tagged<Clamp> clamp() { return rhs_tagged<Clamp>(value); }
};

template <bool Signed, int W> struct traits;
template <int W>
struct traits<false,W> {
  static const value_type min_value = 0;
  static const value_type max_value = (static_cast<value_type>(1)<<W)-1;
  static value_type wrap(value_type value) {
    return value & max_value;
  }
  static value_type complement(value_type value) {
    return value ^ max_value;
  }
};
template <int W>
struct traits<true,W> {
  static const value_type min_value = static_cast<value_type>(-1)<<(W-1);
  static const value_type max_value = (static_cast<value_type>(1)<<(W-1))-1;
  static value_type wrap(value_type value) {
    return (value << (sizeof(value_type)*8-W)) >> (sizeof(value_type)*8-W);
  }
  static value_type complement(value_type value) {
    return ~value;
  }
};

template <bool Signed, int W>
struct fx {
  static const bool is_signed = Signed;
  static const int width = W;
  static const value_type min_value = traits<Signed,W>::min_value;
  static const value_type max_value = traits<Signed,W>::max_value;
  value_type value;
  fx() {}
  fx(const fx& other) : value(other.value) {}
  fx(value_type value_) { assign(value_); }
  template <int LINE>
  fx(const rhs<LINE>& rhs_) { assign(rhs_); }
  template <rhs_tag TAG>
  fx(const rhs_tagged<TAG>& rhs_) { assign(rhs_); }
  fx& operator=(const fx& other) {
    if (this != &other)  value = other.value;
    return *this;
  }
  fx& operator=(value_type value_) { assign(value_); return *this; }
  template <int LINE>
  fx& operator=(const rhs<LINE>& rhs_) { assign(rhs_); return *this; }
  template <rhs_tag TAG>
  fx& operator=(const rhs_tagged<TAG>& rhs_) { assign(rhs_); return *this; }
  template <bool S2, int W2, int LINE>
  rhs<LINE> add(const fx<S2,W2>& other, const loc<LINE>& loc_ = loc<LINE>()) {
    return rhs<LINE>(value + other.value, loc_);
  }
  template <bool S2, int W2, int LINE>
  rhs<LINE> sub(const fx<S2,W2>& other, const loc<LINE>& loc_ = loc<LINE>()) {
    return rhs<LINE>(value - other.value, loc_);
  }
  template <bool S2, int W2, int LINE>
  rhs<LINE> mul(const fx<S2,W2>& other, const loc<LINE>& loc_ = loc<LINE>()) {
    return rhs<LINE>(value * other.value, loc_);
  }
  template <bool S2, int W2, int LINE>
  rhs<LINE> div(const fx<S2,W2>& other, const loc<LINE>& loc_ = loc<LINE>()) {
    return rhs<LINE>(value / other.value, loc_);
  }
  template <bool S2, int W2, int LINE>
  rhs<LINE> rem(const fx<S2,W2>& other, const loc<LINE>& loc_ = loc<LINE>()) {
    return rhs<LINE>(value % other.value, loc_);
  }
  template <bool S2, int W2, int LINE>
  rhs<LINE> bwand(const fx<S2,W2>& other, const loc<LINE>& loc_ = loc<LINE>()) {
    return rhs<LINE>(value & other.value, loc_);
  }
  template <bool S2, int W2, int LINE>
  rhs<LINE> bwor(const fx<S2,W2>& other, const loc<LINE>& loc_ = loc<LINE>()) {
    return rhs<LINE>(value | other.value, loc_);
  }
  template <bool S2, int W2, int LINE>
  rhs<LINE> bwxor(const fx<S2,W2>& other, const loc<LINE>& loc_ = loc<LINE>()) {
    return rhs<LINE>(value ^ other.value, loc_);
  }

  template <int LINE>
  rhs<LINE> rsft(int other, const loc<LINE>& loc_ = loc<LINE>()) {
    return rhs<LINE>(value >> other, loc_);
  }
  template <int LINE>
  rhs<LINE> lsft(int other, const loc<LINE>& loc_ = loc<LINE>()) {
    return rhs<LINE>(value << other, loc_);
  }
  template <int LINE>
  rhs<LINE> neg(const loc<LINE>& loc_ = loc<LINE>()) {
    return rhs<LINE>(-value, loc_);
  }
  template <int LINE>
  rhs<LINE> squ(const loc<LINE>& loc_ = loc<LINE>()) {
    return rhs<LINE>(value*value, loc_);
  }
  template <int LINE>
  rhs<LINE> cnv(const loc<LINE>& loc_ = loc<LINE>()) {
    return rhs<LINE>(value, loc_);
  }
  template <int LINE>
  rhs<LINE> bwnot(const loc<LINE>& loc_ = loc<LINE>()) {
    return rhs<LINE>(traits<Signed,W>::complement(value), loc_);
  }
  rhs_tagged<Wrap>  wrap()  { return rhs_tagged<Wrap> (value); }
  rhs_tagged<Clamp> clamp() { return rhs_tagged<Clamp>(value); }

  template <bool S2, int W2>
  rhs<0> operator+(const fx<S2,W2>& other) { return add<S2,W2,0>(other); }
  template <bool S2, int W2>
  rhs<0> operator-(const fx<S2,W2>& other) { return sub<S2,W2,0>(other); }
  template <bool S2, int W2>
  rhs<0> operator*(const fx<S2,W2>& other) { return mul<S2,W2,0>(other); }
  template <bool S2, int W2>
  rhs<0> operator/(const fx<S2,W2>& other) { return div<S2,W2,0>(other); }
  template <bool S2, int W2>
  rhs<0> operator%(const fx<S2,W2>& other) { return rem<S2,W2,0>(other); }
  template <bool S2, int W2>
  rhs<0> operator&(const fx<S2,W2>& other) { return bwand<S2,W2,0>(other); }
  template <bool S2, int W2>
  rhs<0> operator|(const fx<S2,W2>& other) { return bwor<S2,W2,0>(other); }
  template <bool S2, int W2>
  rhs<0> operator^(const fx<S2,W2>& other) { return bwxor<S2,W2,0>(other); }
  rhs<0> operator>>(int other) { return rsft<0>(other); }
  rhs<0> operator<<(int other) { return lsft<0>(other); }
  rhs<0> operator-() { return neg<0>(); }
  rhs<0> operator~() { return bwnot<0>(); }

  template <bool S2, int W2>
  bool operator==(const fx<S2,W2>& other) { return value == other.value; }
  template <bool S2, int W2>
  bool operator!=(const fx<S2,W2>& other) { return value != other.value; }
  template <bool S2, int W2>
  bool operator>=(const fx<S2,W2>& other) { return value >= other.value; }
  template <bool S2, int W2>
  bool operator<=(const fx<S2,W2>& other) { return value <= other.value; }
  template <bool S2, int W2>
  bool operator> (const fx<S2,W2>& other) { return value >  other.value; }
  template <bool S2, int W2>
  bool operator< (const fx<S2,W2>& other) { return value <  other.value; }

private:
  template <int LINE>
  void assign(const rhs<LINE>& rhs_) {
    value = traits<Signed,W>::wrap(rhs_.value);
#ifdef FX_CHECK_OVERFLOW
    extern void overflow_error(const char *file, int line);
    if (value != rhs_.value)  overflow_error(rhs_.m_loc.file, LINE);
#endif
  }
  void assign(value_type value_) { assign<0>(rhs<0>(value_, loc<0>())); }
  void assign(const rhs_tagged<Wrap>& rhs_) {
    value = traits<Signed,W>::wrap(rhs_.value);
  }
  void assign(const rhs_tagged<Clamp>& rhs_) {
    value = ((rhs_.value < min_value)
	     ? min_value
	     : (rhs_.value > max_value) ? max_value : rhs_.value);
  }
};

};

typedef fx::fx<true, 1>   int1;
typedef fx::fx<true, 2>   int2;
typedef fx::fx<true, 3>   int3;
typedef fx::fx<true, 4>   int4;
typedef fx::fx<true, 5>   int5;
typedef fx::fx<true, 6>   int6;
typedef fx::fx<true, 7>   int7;
typedef fx::fx<true, 8>   int8;
typedef fx::fx<true, 9>   int9;
typedef fx::fx<true,10>   int10;
typedef fx::fx<true,11>   int11;
typedef fx::fx<true,12>   int12;
typedef fx::fx<true,13>   int13;
typedef fx::fx<true,14>   int14;
typedef fx::fx<true,15>   int15;
typedef fx::fx<true,16>   int16;
typedef fx::fx<true,17>   int17;
typedef fx::fx<true,18>   int18;
typedef fx::fx<true,19>   int19;
typedef fx::fx<true,20>   int20;
typedef fx::fx<true,21>   int21;
typedef fx::fx<true,22>   int22;
typedef fx::fx<true,23>   int23;
typedef fx::fx<true,24>   int24;
typedef fx::fx<true,25>   int25;
typedef fx::fx<true,26>   int26;
typedef fx::fx<true,27>   int27;
typedef fx::fx<true,28>   int28;
typedef fx::fx<true,29>   int29;
typedef fx::fx<true,30>   int30;
typedef fx::fx<true,31>   int31;
typedef fx::fx<true,32>   int32;

typedef fx::fx<false, 1>   uint1;
typedef fx::fx<false, 2>   uint2;
typedef fx::fx<false, 3>   uint3;
typedef fx::fx<false, 4>   uint4;
typedef fx::fx<false, 5>   uint5;
typedef fx::fx<false, 6>   uint6;
typedef fx::fx<false, 7>   uint7;
typedef fx::fx<false, 8>   uint8;
typedef fx::fx<false, 9>   uint9;
typedef fx::fx<false,10>   uint10;
typedef fx::fx<false,11>   uint11;
typedef fx::fx<false,12>   uint12;
typedef fx::fx<false,13>   uint13;
typedef fx::fx<false,14>   uint14;
typedef fx::fx<false,15>   uint15;
typedef fx::fx<false,16>   uint16;
typedef fx::fx<false,17>   uint17;
typedef fx::fx<false,18>   uint18;
typedef fx::fx<false,19>   uint19;
typedef fx::fx<false,20>   uint20;
typedef fx::fx<false,21>   uint21;
typedef fx::fx<false,22>   uint22;
typedef fx::fx<false,23>   uint23;
typedef fx::fx<false,24>   uint24;
typedef fx::fx<false,25>   uint25;
typedef fx::fx<false,26>   uint26;
typedef fx::fx<false,27>   uint27;
typedef fx::fx<false,28>   uint28;
typedef fx::fx<false,29>   uint29;
typedef fx::fx<false,30>   uint30;
typedef fx::fx<false,31>   uint31;

#define fx_imm(v)     (fx::rhs<__LINE__>((v),fx::loc<__LINE__>(__FILE__)))
#define fx_add(a,b)   ((a).add((b), fx::loc<__LINE__>(__FILE__)))
#define fx_sub(a,b)   ((a).sub((b), fx::loc<__LINE__>(__FILE__)))
#define fx_mul(a,b)   ((a).mul((b), fx::loc<__LINE__>(__FILE__)))
#define fx_div(a,b)   ((a).div((b), fx::loc<__LINE__>(__FILE__)))
#define fx_rem(a,b)   ((a).rem((b), fx::loc<__LINE__>(__FILE__)))
#define fx_and(a,b)   ((a).bwand((b), fx::loc<__LINE__>(__FILE__)))
#define fx_or(a,b)    ((a).bwor((b), fx::loc<__LINE__>(__FILE__)))
#define fx_xor(a,b)   ((a).bwxor((b), fx::loc<__LINE__>(__FILE__)))
#define fx_lsft(a,b)  ((a).lsft((b), fx::loc<__LINE__>(__FILE__)))
#define fx_rsft(a,b)  ((a).rsft((b), fx::loc<__LINE__>(__FILE__)))
#define fx_neg(a)     ((a).neg(fx::loc<__LINE__>(__FILE__)))
#define fx_squ(a)     ((a).squ(fx::loc<__LINE__>(__FILE__)))
#define fx_not(a)     ((a).bwnot(fx::loc<__LINE__>(__FILE__)))
#define fx_cnv(a)     ((a).cnv(fx::loc<__LINE__>(__FILE__)))
#define fx_wrap(x)    ((x).wrap())   // "x" may be fx<> or rhs<>
#define fx_clamp(x)   ((x).clamp())  // "x" may be fx<> or rhs<>

#endif

