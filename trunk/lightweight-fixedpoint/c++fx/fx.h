// -*- c++ -*-

//
// Lightweight restricted-width integer library for C++.
//

// Copyright (c) 2008 the handy-eda-utils developer(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

// Examples:
//
//   int10 a = fx(-123);
//   uint10 b = fx(999);
//   int10 c0 = fx(a+b);
//   int6  c1 = fx_wrap(a+b);    // wraparound w/o overflow warning
//   int6  c2 = fx_clamp(a+b);   // saturate
//   std::cout << a.hex();       // same as write("%h",a) in Verilog
//   std::cout << a.bin();       // same as write("%b",a) in Verilog
//
//   const int PIXEL_DEPTH = 8;
//   typedef fx::fxint<unsigned,PIXEL_DEPTH>  pixel;
//   pixel  pa, pb;
//   pixel::widen<1>::type        psum = pa + pb;
//   pixel::widen<1>::signed_type pdiff = pa - pb;
//
// Unrecommended Examples:
//
//   int8 a = -12345;
//   int4 c = a + b;
//   int6 d = a;       // implicit conversion between different formats
//   a ++;
//   a += b;
//     These five coding styles result in unfriendly error messages upon overflow.
//
//   int10 e = fx(a+b+c);
//     Format of the intermediate calculation result is undefined.
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

#include <string>

namespace fx {

typedef int value_type;   // or "long long"

template <int LINE>
struct loc {
  const char * const file;
  loc(const char *file_ = 0) : file(file_) {}
};

enum rhs_tag { Wrap, Clamp };

template <rhs_tag TAG>
struct rhs_tagged {
  const value_type value;
  rhs_tagged(value_type value_) : value(value_) {}
};

template <int LINE>
struct rhs_loc {
  const value_type value;
  const loc<LINE>& m_loc;
  rhs_loc(value_type value_, const char *file) : value(value_), m_loc(file) {}
};

template <typename S, int W> struct traits;
template <int W>
struct traits<unsigned,W> {
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
struct traits<signed,W> {
  static const value_type min_value = static_cast<value_type>(-1)<<(W-1);
  static const value_type max_value = (static_cast<value_type>(1)<<(W-1))-1;
  static value_type wrap(value_type value) {
    return (value << (sizeof(value_type)*8-W)) >> (sizeof(value_type)*8-W);
  }
  static value_type complement(value_type value) {
    return ~value;
  }
};

template <typename S> struct sign_check;
template <> struct sign_check<unsigned> { static const bool is_signed = false; };
template <> struct sign_check<signed>   { static const bool is_signed = true;  };

template <bool Signed, int W> struct width_check {
  // "INVALID_WIDTH" is 0 when invalid.
  enum { INVALID_WIDTH = (W>0)&&(W<=sizeof(value_type)*8-1+Signed) };
  typedef char dummy[INVALID_WIDTH];   // cf. boost/static_assert
  static const int corrected_width = (INVALID_WIDTH==0) ? 1 : W;
};

template <typename S, int W_>
struct fxint {
  static const bool is_signed = sign_check<S>::is_signed;
  static const int W = width_check<is_signed,W_>::corrected_width;
  static const int width = W;
  static const value_type min_value = traits<S,W>::min_value;
  static const value_type max_value = traits<S,W>::max_value;
  typedef fxint<unsigned,W> unsigned_type;
  typedef fxint<signed,  W> signed_type;
  template <int D>
  struct widen {
    typedef fxint<S,        W+D> type;
    typedef fxint<unsigned, W+D> unsigned_type;
    typedef fxint<signed,   W+D> signed_type;
  };
  value_type value;
  fxint() {}
  fxint(const fxint& other) : value(other.value) {}
  fxint(value_type val) { assign(val); }
  template <int LINE>
  fxint(const rhs_loc<LINE>& rhs) { assign(rhs); }
  template <rhs_tag TAG>
  fxint(const rhs_tagged<TAG>& rhs) { assign(rhs); }
  fxint& operator=(const fxint& other) {
    if (this != &other)  value = other.value;
    return *this;
  }

  operator value_type() const { return value; }

  std::string hex() const {
    extern std::string convert_to_hexadecimal_string(value_type, int);
    return convert_to_hexadecimal_string(value, W);
  }
  std::string bin() const {
    extern std::string convert_to_binary_string(value_type, int);
    return convert_to_binary_string(value, W);
  }

  // unrecommended features which issue unfriendly warning messages upon overflow
  template <typename S2, int W2> fxint(const fxint<S2,W2>& rhs) { assign(rhs.value); }
  template <typename S2, int W2> operator fxint<S2,W2>() const { return fxint<S2,W2>(value); }
  template <typename T> fxint& operator+=(T other) { assign(value + other); return *this; }
  template <typename T> fxint& operator-=(T other) { assign(value - other); return *this; }
  template <typename T> fxint& operator*=(T other) { assign(value * other); return *this; }
  template <typename T> fxint& operator/=(T other) { assign(value / other); return *this; }
  template <typename T> fxint& operator%=(T other) { assign(value % other); return *this; }
  template <typename T> fxint& operator&=(T other) { assign(value & other); return *this; }
  template <typename T> fxint& operator|=(T other) { assign(value | other); return *this; }
  template <typename T> fxint& operator^=(T other) { assign(value ^ other); return *this; }
  template <typename T> fxint& operator<<=(T other) { assign(value << other); return *this; }
  template <typename T> fxint& operator>>=(T other) { assign(value >> other); return *this; }
  fxint& operator++() { assign(value+1);  return *this; }
  fxint operator++(int) { fxint ret = *this;  assign(value+1);  return ret; }
  fxint& operator--() { assign(value-1);  return *this; }
  fxint operator--(int) { fxint ret = *this;  assign(value-1);  return ret; }

private:
  template <int LINE>
  void assign(const rhs_loc<LINE>& rhs) {
    value = traits<S,W>::wrap(rhs.value);
    extern void overflow_error(const char *file, int line);
    if (value != rhs.value)  overflow_error(rhs.m_loc.file, LINE);
  }
  void assign(value_type val) {
#ifdef FX_CHECK_OVERFLOW
    assign(rhs_loc<0>(val,0));
#else
    value = traits<S,W>::wrap(val);
#endif
  }
  void assign(const rhs_tagged<Wrap>& rhs) {
    value = traits<S,W>::wrap(rhs.value);
  }
  void assign(const rhs_tagged<Clamp>& rhs) {
    value = ((rhs.value < min_value)
	     ? min_value
	     : (rhs.value > max_value) ? max_value : rhs.value);
  }
};

};

typedef fx::fxint<signed, 1>   int1;
typedef fx::fxint<signed, 2>   int2;
typedef fx::fxint<signed, 3>   int3;
typedef fx::fxint<signed, 4>   int4;
typedef fx::fxint<signed, 5>   int5;
typedef fx::fxint<signed, 6>   int6;
typedef fx::fxint<signed, 7>   int7;
typedef fx::fxint<signed, 8>   int8;
typedef fx::fxint<signed, 9>   int9;
typedef fx::fxint<signed,10>   int10;
typedef fx::fxint<signed,11>   int11;
typedef fx::fxint<signed,12>   int12;
typedef fx::fxint<signed,13>   int13;
typedef fx::fxint<signed,14>   int14;
typedef fx::fxint<signed,15>   int15;
typedef fx::fxint<signed,16>   int16;
typedef fx::fxint<signed,17>   int17;
typedef fx::fxint<signed,18>   int18;
typedef fx::fxint<signed,19>   int19;
typedef fx::fxint<signed,20>   int20;
typedef fx::fxint<signed,21>   int21;
typedef fx::fxint<signed,22>   int22;
typedef fx::fxint<signed,23>   int23;
typedef fx::fxint<signed,24>   int24;
typedef fx::fxint<signed,25>   int25;
typedef fx::fxint<signed,26>   int26;
typedef fx::fxint<signed,27>   int27;
typedef fx::fxint<signed,28>   int28;
typedef fx::fxint<signed,29>   int29;
typedef fx::fxint<signed,30>   int30;
typedef fx::fxint<signed,31>   int31;
typedef fx::fxint<signed,32>   int32;

typedef fx::fxint<unsigned, 1>   uint1;
typedef fx::fxint<unsigned, 2>   uint2;
typedef fx::fxint<unsigned, 3>   uint3;
typedef fx::fxint<unsigned, 4>   uint4;
typedef fx::fxint<unsigned, 5>   uint5;
typedef fx::fxint<unsigned, 6>   uint6;
typedef fx::fxint<unsigned, 7>   uint7;
typedef fx::fxint<unsigned, 8>   uint8;
typedef fx::fxint<unsigned, 9>   uint9;
typedef fx::fxint<unsigned,10>   uint10;
typedef fx::fxint<unsigned,11>   uint11;
typedef fx::fxint<unsigned,12>   uint12;
typedef fx::fxint<unsigned,13>   uint13;
typedef fx::fxint<unsigned,14>   uint14;
typedef fx::fxint<unsigned,15>   uint15;
typedef fx::fxint<unsigned,16>   uint16;
typedef fx::fxint<unsigned,17>   uint17;
typedef fx::fxint<unsigned,18>   uint18;
typedef fx::fxint<unsigned,19>   uint19;
typedef fx::fxint<unsigned,20>   uint20;
typedef fx::fxint<unsigned,21>   uint21;
typedef fx::fxint<unsigned,22>   uint22;
typedef fx::fxint<unsigned,23>   uint23;
typedef fx::fxint<unsigned,24>   uint24;
typedef fx::fxint<unsigned,25>   uint25;
typedef fx::fxint<unsigned,26>   uint26;
typedef fx::fxint<unsigned,27>   uint27;
typedef fx::fxint<unsigned,28>   uint28;
typedef fx::fxint<unsigned,29>   uint29;
typedef fx::fxint<unsigned,30>   uint30;
typedef fx::fxint<unsigned,31>   uint31;

#ifdef FX_CHECK_OVERFLOW
#define fx(v)                      (fx::rhs_loc<__LINE__>((v),__FILE__))
#else
#define fx(v)                      (static_cast<fx::value_type>(v))
#endif
typedef fx::rhs_tagged<fx::Wrap>   fx_wrap;
typedef fx::rhs_tagged<fx::Clamp>  fx_clamp;

#endif

