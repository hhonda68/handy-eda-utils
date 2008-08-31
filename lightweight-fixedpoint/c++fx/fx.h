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
//   int10 a = fx(-123);
//   uint10 b = fx(999);
//   int10 c0 = fx(a+b);
//   int6  c1 = fx_wrap(a+b);    // wraparound w/o overflow warning
//   int6  c2 = fx_clamp(a+b);   // saturate
//   std::cout << a.hex();       // same as write("%h",a) in Verilog
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
struct fxint {
  static const bool is_signed = Signed;
  static const int width = W;
  static const value_type min_value = traits<Signed,W>::min_value;
  static const value_type max_value = traits<Signed,W>::max_value;
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

  // unrecommended features which issue unfriendly warning messages upon overflow
  template <bool S2, int W2> fxint(const fxint<S2,W2>& rhs) { assign(rhs.value); }
  template <bool S2, int W2> operator fxint<S2,W2>() const { return fxint<S2,W2>(value); }
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
    value = traits<Signed,W>::wrap(rhs.value);
    extern void overflow_error(const char *file, int line);
    if (value != rhs.value)  overflow_error(rhs.m_loc.file, LINE);
  }
  void assign(value_type val) {
#ifdef FX_CHECK_OVERFLOW
    assign(rhs_loc<0>(val,0));
#else
    value = traits<Signed,W>::wrap(val);
#endif
  }
  void assign(const rhs_tagged<Wrap>& rhs) {
    value = traits<Signed,W>::wrap(rhs.value);
  }
  void assign(const rhs_tagged<Clamp>& rhs) {
    value = ((rhs.value < min_value)
	     ? min_value
	     : (rhs.value > max_value) ? max_value : rhs.value);
  }
};

};

typedef fx::fxint<true, 1>   int1;
typedef fx::fxint<true, 2>   int2;
typedef fx::fxint<true, 3>   int3;
typedef fx::fxint<true, 4>   int4;
typedef fx::fxint<true, 5>   int5;
typedef fx::fxint<true, 6>   int6;
typedef fx::fxint<true, 7>   int7;
typedef fx::fxint<true, 8>   int8;
typedef fx::fxint<true, 9>   int9;
typedef fx::fxint<true,10>   int10;
typedef fx::fxint<true,11>   int11;
typedef fx::fxint<true,12>   int12;
typedef fx::fxint<true,13>   int13;
typedef fx::fxint<true,14>   int14;
typedef fx::fxint<true,15>   int15;
typedef fx::fxint<true,16>   int16;
typedef fx::fxint<true,17>   int17;
typedef fx::fxint<true,18>   int18;
typedef fx::fxint<true,19>   int19;
typedef fx::fxint<true,20>   int20;
typedef fx::fxint<true,21>   int21;
typedef fx::fxint<true,22>   int22;
typedef fx::fxint<true,23>   int23;
typedef fx::fxint<true,24>   int24;
typedef fx::fxint<true,25>   int25;
typedef fx::fxint<true,26>   int26;
typedef fx::fxint<true,27>   int27;
typedef fx::fxint<true,28>   int28;
typedef fx::fxint<true,29>   int29;
typedef fx::fxint<true,30>   int30;
typedef fx::fxint<true,31>   int31;
typedef fx::fxint<true,32>   int32;

typedef fx::fxint<false, 1>   uint1;
typedef fx::fxint<false, 2>   uint2;
typedef fx::fxint<false, 3>   uint3;
typedef fx::fxint<false, 4>   uint4;
typedef fx::fxint<false, 5>   uint5;
typedef fx::fxint<false, 6>   uint6;
typedef fx::fxint<false, 7>   uint7;
typedef fx::fxint<false, 8>   uint8;
typedef fx::fxint<false, 9>   uint9;
typedef fx::fxint<false,10>   uint10;
typedef fx::fxint<false,11>   uint11;
typedef fx::fxint<false,12>   uint12;
typedef fx::fxint<false,13>   uint13;
typedef fx::fxint<false,14>   uint14;
typedef fx::fxint<false,15>   uint15;
typedef fx::fxint<false,16>   uint16;
typedef fx::fxint<false,17>   uint17;
typedef fx::fxint<false,18>   uint18;
typedef fx::fxint<false,19>   uint19;
typedef fx::fxint<false,20>   uint20;
typedef fx::fxint<false,21>   uint21;
typedef fx::fxint<false,22>   uint22;
typedef fx::fxint<false,23>   uint23;
typedef fx::fxint<false,24>   uint24;
typedef fx::fxint<false,25>   uint25;
typedef fx::fxint<false,26>   uint26;
typedef fx::fxint<false,27>   uint27;
typedef fx::fxint<false,28>   uint28;
typedef fx::fxint<false,29>   uint29;
typedef fx::fxint<false,30>   uint30;
typedef fx::fxint<false,31>   uint31;

#ifdef FX_CHECK_OVERFLOW
#define fx(v)                      (fx::rhs_loc<__LINE__>((v),__FILE__))
#else
#define fx(v)                      (static_cast<fx::value_type>(v))
#endif
typedef fx::rhs_tagged<fx::Wrap>   fx_wrap;
typedef fx::rhs_tagged<fx::Clamp>  fx_clamp;

#endif

