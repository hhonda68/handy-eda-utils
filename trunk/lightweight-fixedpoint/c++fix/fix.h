// -*- C++ -*-

//
// Lightweight fixed-point library for C++.
//

// Copyright (c) 2008 the handy-eda-utils develeper(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

// Last modified on October 2007.

#ifndef FIX_H_INCLUDED
#define FIX_H_INCLUDED

#define FIX_CHECK_OVERFLOW
#define FIX_CHECK_DIVBYZERO

////////////////////////////////////////////////////////////////
//
// Usage example:
//
//  S<4, 8> a = -1.25;          // or fix_ftoi(-1.25)
//  S<8, 4> b = 2.375;
//  S<4,12> c = a + b;          // or fix_add(a,b)
//  S<4,12> d = a - b;          // or fix_sub(a,b)
//  S<4,12> e = a * b;          // or fix_mul(a,b)
//  S<4,12> f = a / b;          // or fix_div(a,b)
//  S<4,12> g = -a;             // or fix_neg(a)
//  S<7, 5> h = a.sft<3>();     // or fix_sft(a,3)
//  S<4,12> k = a.cnv();        // or fix_cnv(a)
//  S<4,12> m = a.squ();        // or fix_squ(a)
//  S<4, 8> a2 = a;             // simple assignment (w/o oveflow check)
//
//  S<4, 8> p = b.inv();        // or fix_inv(a)
//  bool    ci = true;
//  S<4,12> q = a + b + ci;     // or fix_addci(a,b,ci)
//
//  double x = a.itof();        // or fix_itof(a)
//  std::string s = a.hex();    // or fix_hex(a)
//  int val = a.value;          // peek internal bitpattern directly
//
//  S<4,12> k2 = a;             // Compilation Error (FIX_ASSIGN_FORMAT_ERROR)
//  S<7, 5> h2 = fix_sft(a,2);  // Compilation Error (FIX_ASSIGN_FORMAT_ERROR)
//
// fix_cnv()   : format conversion (with overflow check)
// fix_sft()   : NOP (shift by constant)
// fix_hex()   : hexadecimal string with $display("%h") format
// fix_inv()   : bitwise NOT
// fix_addci() : add with carry in
//
// S<m,n> is a signed fixed-point type with "m" integral bits
// (including the sign bit) and "n" fractional bits.
// U<m,n> is an unsigned fixed-point type with "m" integral bits
// and "n" fractional bits.
//
// Assignments between types with different formats are not allowed.
// Write fix_cnv() explicitly to convert the format.
//
// fix_sft() re-interprets the bit pattern of a variable,
// without modifying the bit pattern.
// A positive shift-amount represents a left shift.
// A negative shift-amount represents a right shift.
//
// fix_addci() accepts a carry-in bit, assuming that the weight
// of the carry-in bit is the same as LSB of the "b" operand.
//
// If you use the fix_add(a,b) syntax, the program prints
// a friendly error message with filename and line number
// when a overflow or a division-by-zero occurs.
// On the other hand, if you use the "a+b" syntax, the
// program prints an unfriendly error message with
// hexadecimal address.
// Note that itof(),hex(),inv() never cause overflow/divbyzero,
// so there is no difference between a.itof() and fix_itof(a).
//
////////////////////////////////////////////////////////////////

#include <cstddef>
#include <string>

template <int LINE>
struct fix_loc {
  const char *file;
  fix_loc(const char *file_) : file(file_) {}
  fix_loc(const fix_loc& loc) : file(loc.file) {}
};

#ifdef FIX_CHECK_OVERFLOW
extern bool fix_overflow_flag;
extern void fix_overflow_error(const char *, int);

template <bool TrivialError, bool TS, int TW> struct fix_ovfchk_aux;
template <bool TS, int TW> struct fix_ovfchk_aux<true,TS,TW> {
  template <typename TT>
  static bool outofrange(TT val) { return true; }
};
template <int TW> struct fix_ovfchk_aux<false,false,TW> {
  static bool outofrange(int val) {
    return ((static_cast<unsigned int>(val) >> TW) != 0);
  }
  static bool outofrange(long long val) {
    return ((static_cast<unsigned long long>(val) >> TW) != 0);
  }
};
template <int TW> struct fix_ovfchk_aux<false,true,TW> {
  static bool outofrange(int val) {
    return (((val >> (TW-1)) ^ (val >> 31)) != 0);
  }
  static bool outofrange(long long val) {
    return (((val >> (TW-1)) ^ (val >> 63)) != 0);
  }
};

template <bool TS, int TW> struct fix_ovfchk {
  template <typename TT, int LINE>
  static void check(TT val, const fix_loc<LINE>& loc) {
    if (fix_ovfchk_aux<(TW<=0),TS,TW>::outofrange(val))  fix_overflow_error(loc.file, LINE);
  }
};
#else
template <bool TS, int TW> struct fix_ovfchk {
  template <typename TT, int LINE>
  static void check(TT val, const fix_loc<LINE>& loc) {}
};
#endif

template <typename T> struct fix_unsigned_traits {};
template<> struct fix_unsigned_traits<int> {
  typedef unsigned int unsined_type;
};
template<> struct fix_unsigned_traits<long long> {
  typedef unsigned long long unsined_type;
};

template <bool Signed, bool Negative, int ShiftAmount>
struct fix_adjuster_aux {};
template <bool Signed>
struct fix_adjuster_aux<Signed, false, 0> {
  template <typename T, int LINE>
  static T adjust(T x, const fix_loc<LINE>& loc) { return x; }
};
template <bool Signed, int ShiftAmount>
struct fix_adjuster_aux<Signed, false, ShiftAmount> {
  template <typename T, int LINE>
  static T adjust(T x, const fix_loc<LINE>& loc) {
    fix_ovfchk<Signed,sizeof(T)*8-ShiftAmount>::check(x, loc);
    return x << ShiftAmount;
  }
};
template <int ShiftAmount>
struct fix_adjuster_aux<false, true, ShiftAmount> {
  template <typename T, int LINE>
  static T adjust(T x, const fix_loc<LINE>& loc) {
    typedef typename fix_unsigned_traits<T>::unsigend_type unsigned_type;
    return static_cast<T>(static_cast<unsigned_type>(x) >> (-ShiftAmount));
  }
};
template <int ShiftAmount>
struct fix_adjuster_aux<true, true, ShiftAmount> {
  template <typename T, int LINE>
  static T adjust(T x, const fix_loc<LINE>& loc) {
    return x >> (-ShiftAmount);
  }
};

template <bool SS, int SF, int DF>
struct fix_adjuster : public fix_adjuster_aux<SS,(DF-SF<0),(DF-SF)> {};

template <bool Signed, int Width, int Lsbpos = 0>
struct fix_slicer {
  template <int LINE>
  static int slice(int val, const fix_loc<LINE>& loc) {
    int sval = fix_adjuster<Signed,Lsbpos,32-Width>::adjust(val, loc);
    return (Signed
	    ? (sval >> (32-Width))
	    : static_cast<int>((static_cast<unsigned int>(sval) >> (32-Width))));
  }
};

template <bool TS, int TW, int TF> struct fix;

template <int LINE>
struct fix_ftoi_desc {
  double src_value;
  fix_loc<LINE> loc;
  fix_ftoi_desc(double src, const fix_loc<LINE>& loc_) : src_value(src), loc(loc_) {}
};

enum fix_uniop_type {
  FIX_UNIOP_CNV, FIX_UNIOP_NEG, FIX_UNIOP_SQU,
};

template <fix_uniop_type OP, bool SS, int SW, int SF, int LINE, int N = 0>
struct fix_uniop_desc {
  fix<SS,SW,SF> src;
  fix_loc<LINE> loc;
  fix_uniop_desc(const fix<SS,SW,SF>& src_, const fix_loc<LINE>& loc_)
    : src(src_), loc(loc_) {}
};

enum fix_binop_type {
  FIX_BINOP_ADD, FIX_BINOP_ADDCI, FIX_BINOP_SUB, FIX_BINOP_MUL, FIX_BINOP_DIV,
};

template <fix_binop_type OP, bool LS, int LW, int LF, bool RS, int RW, int RF, int LINE>
struct fix_binop_desc {
  fix<LS,LW,LF> lhs;
  fix<RS,RW,RF> rhs;
  fix_loc<LINE> loc;
  bool          rhs_ci;
  fix_binop_desc(const fix<LS,LW,LF>& lhs_, const fix<RS,RW,RF>& rhs_,
		 const fix_loc<LINE>& loc_, bool rhs_ci_ = false)
    : lhs(lhs_), rhs(rhs_), loc(loc_), rhs_ci(rhs_ci_) {}
  const fix_binop_desc<FIX_BINOP_ADDCI,LS,LW,LF,RS,RW,RF,LINE>
  operator+(bool ci) const {
    enum { FIX_ADDCI_FORMAT_ERROR = (OP == FIX_BINOP_ADD) };
    typedef char dummy[FIX_ADDCI_FORMAT_ERROR];
    return fix_binop_desc<FIX_BINOP_ADDCI,LS,LW,LF,RS,RW,RF,LINE>(lhs,rhs,loc,ci);
  }
};

template <bool Nodiscard, bool Partialdiscard, int Maskbits>
struct fix_sub_cimask_traits_aux {};
template <int Maskbits>
struct fix_sub_cimask_traits_aux<true,true,Maskbits> {
  static const int mask = 0;
};
template <int Maskbits>
struct fix_sub_cimask_traits_aux<false,true,Maskbits> {
  static const int mask = (1<<Maskbits)-1;
};
template <int Maskbits>
struct fix_sub_cimask_traits_aux<false,false,Maskbits> {
  static const int mask = ~0;
};

template <int RF, int MF>
struct fix_sub_cimask_traits
  : public fix_sub_cimask_traits_aux<(RF-MF<0),(RF-MF<=31),(RF-MF)> {};

template <bool TS, int TW, int TF>
struct fix {
  int value;
  fix() {}
  fix(double arg) {
    extern int fix_ftoi_body(double, int);
    int val = fix_ftoi_body(arg, TF);
    value = fix_slicer<TS,TW>::slice(val, fix_loc<0>(NULL));
  }
  fix(const fix& rhs) : value(rhs.value) {}
  fix& operator=(const fix& rhs) { value = rhs.value; return *this; }

  // Inhibit assignments between different formats
  template <bool SS, int SW, int SF>
  fix(const fix<SS,SW,SF>& rhs) {
    enum { FIX_ASSIGN_FORMAT_ERROR = 0 };
    typedef char dummy[FIX_ASSIGN_FORMAT_ERROR];  // raise compilation error intentionally
  }
  template <bool SS, int SW, int SF>
  fix& operator=(const fix<SS,SW,SF>& rhs) {
    enum { FIX_ASSIGN_FORMAT_ERROR = 0 };
    typedef char dummy[FIX_ASSIGN_FORMAT_ERROR];  // raise compilation error intentionally
  }

  double itof() const {
    extern double fix_itof_body(int, int);
    return fix_itof_body(value, TF);
  }
  const std::string hex() const {
    extern const std::string fix_hex_body(int, int);
    return fix_hex_body(value, TW);
  }

  const fix inv() const {
    fix ans;
    ans.value = TS ? (~value) : ((~value) & ((1<<(TW))-1));
    return ans;
  }

  template <int N>
  const fix<TS,TW,(TF-N)> sft() const {
    fix<TS,TW,(TF-N)> ans;
    ans.value = value;
    return ans;
  }

  template <int LINE>
  fix(const fix_ftoi_desc<LINE>& desc) {
    extern int fix_ftoi_body(double, int);
    int val = fix_ftoi_body(desc.src_value, TF);
    value = fix_slicer<TS,TW>::slice(val, desc.loc);
  }

  template <fix_uniop_type OP, bool SS, int SW, int SF, int LINE>
  fix(const fix_uniop_desc<OP,SS,SW,SF,LINE>& desc) {}

  template <bool SS, int SW, int SF, int LINE>
  fix(const fix_uniop_desc<FIX_UNIOP_CNV,SS,SW,SF,LINE>& desc) {
    value = fix_slicer<TS,TW,SF-TF>::slice(desc.src.value, desc.loc);
  }

  template <bool SS, int SW, int SF, int LINE>
  fix(const fix_uniop_desc<FIX_UNIOP_NEG,SS,SW,SF,LINE>& desc) {
    value = fix_slicer<TS,TW,SF-TF>::slice(-desc.src.value, desc.loc);
  }

  template <bool SS, int SW, int SF, int LINE>
  fix(const fix_uniop_desc<FIX_UNIOP_SQU,SS,SW,SF,LINE>& desc) {
    long long src = static_cast<long long>(desc.src.value);
    long long prod = src * src;
    long long prod_adj = fix_adjuster<true,SF+SF,TF>::adjust(prod, desc.loc);
    fix_ovfchk<TS,32>::check(prod_adj, desc.loc);
    value = fix_slicer<TS,TW>::slice(static_cast<int>(prod_adj), desc.loc);
  }

  template <fix_binop_type OP, bool LS, int LW, int LF, bool RS, int RW, int RF, int LINE>
  fix(const fix_binop_desc<OP,LS,LW,LF,RS,RW,RF,LINE>& binop) {}

  template <bool LS, int LW, int LF, bool RS, int RW, int RF, int LINE>
  fix(const fix_binop_desc<FIX_BINOP_ADD,LS,LW,LF,RS,RW,RF,LINE>& desc) {
    const int MF = ((RF>LF)
		    ? ((LF>TF) ? LF : ((RF>TF) ? TF : RF))
		    : ((RF>TF) ? RF : ((LF>TF) ? TF : LF)));
    int ml = fix_adjuster<LS,LF,MF>::adjust(desc.lhs.value, desc.loc);
    int mr = fix_adjuster<RS,RF,MF>::adjust(desc.rhs.value, desc.loc);
    int sum = ml + mr;
    value = fix_slicer<TS,TW,MF-TF>::slice(sum, desc.loc);
  }

  template <bool LS, int LW, int LF, bool RS, int RW, int RF, int LINE>
  fix(const fix_binop_desc<FIX_BINOP_ADDCI,LS,LW,LF,RS,RW,RF,LINE>& desc) {
    const int MF = ((RF>LF)
		    ? ((LF>TF) ? LF : ((RF>TF) ? TF : RF))
		    : ((RF>TF) ? RF : ((LF>TF) ? TF : LF)));
    int ml = fix_adjuster<LS,LF,MF>::adjust(desc.lhs.value, desc.loc);
    int mr = fix_adjuster<RS,RF,MF>::adjust(desc.rhs.value + desc.rhs_ci, desc.loc);
    int sum = ml + mr;
    value = fix_slicer<TS,TW,MF-TF>::slice(sum, desc.loc);
  }

  template <bool LS, int LW, int LF, bool RS, int RW, int RF, int LINE>
  fix(const fix_binop_desc<FIX_BINOP_SUB,LS,LW,LF,RS,RW,RF,LINE>& desc) {
    const int MF = ((RF>LF)
		    ? ((LF>TF) ? LF : ((RF>TF) ? TF : RF))
		    : ((RF>TF) ? RF : ((LF>TF) ? TF : LF)));
    int ml = fix_adjuster<LS,LF,MF>::adjust(desc.lhs.value, desc.loc);
    int mr = fix_adjuster<RS,RF,MF>::adjust(desc.rhs.value, desc.loc);
    int cimask = fix_sub_cimask_traits<RF,MF>::mask;
    int ci = ((desc.rhs.value & cimask) == 0) ? 1 : 0;
    int diff = ml + (~mr) + ci;
    value = fix_slicer<TS,TW,MF-TF>::slice(diff, desc.loc);
  }

  template <bool LS, int LW, int LF, bool RS, int RW, int RF, int LINE>
  fix(const fix_binop_desc<FIX_BINOP_MUL,LS,LW,LF,RS,RW,RF,LINE>& desc) {
    long long ml = static_cast<long long>(desc.lhs.value);
    long long mr = static_cast<long long>(desc.rhs.value);
    long long prod = ml * mr;
    long long prod_adj = fix_adjuster<true,LF+RF,TF>::adjust(prod, desc.loc);
    fix_ovfchk<TS,32>::check(prod_adj, desc.loc);
    value = fix_slicer<TS,TW>::slice(static_cast<int>(prod_adj), desc.loc);
  }

  template <bool LS, int LW, int LF, bool RS, int RW, int RF, int LINE>
  fix(const fix_binop_desc<FIX_BINOP_DIV,LS,LW,LF,RS,RW,RF,LINE>& desc) {
    long long ml = static_cast<long long>(desc.lhs.value);
    long long mr = static_cast<long long>(desc.rhs.value);
    if (mr == 0) {
#ifdef FIX_CHECK_DIVBYZERO
      extern void fix_divbyzero_error(const char *, int);
      fix_divbyzero_error(desc.loc.file, LINE);
#endif
      value = (1<<(TW-TS))-1; 
    } else {
      const int n = RF - LF + TF;
      if (n >= 0) {
	ml = fix_adjuster<true,0,n>::adjust(ml, desc.loc);
      } else {
	mr = fix_adjuster<true,0,-n>::adjust(mr, desc.loc);
      }
      long long quo = ml / mr;
      fix_ovfchk<TS,32>::check(quo, desc.loc);
      value = fix_slicer<TS,TW>::slice(static_cast<int>(quo), desc.loc);
    }
  }

  template <int LINE>
  const fix_uniop_desc<FIX_UNIOP_CNV,TS,TW,TF,LINE>
  cnv(const fix_loc<LINE>& loc) const {
    return fix_uniop_desc<FIX_UNIOP_CNV,TS,TW,TF,LINE>(*this, loc);
  }
  template <int LINE>
  const fix_uniop_desc<FIX_UNIOP_NEG,TS,TW,TF,LINE>
  neg(const fix_loc<LINE>& loc) const {
    return fix_uniop_desc<FIX_UNIOP_NEG,TS,TW,TF,LINE>(*this, loc);
  }
  template <int LINE>
  const fix_uniop_desc<FIX_UNIOP_SQU,TS,TW,TF,LINE>
  squ(const fix_loc<LINE>& loc) const {
    return fix_uniop_desc<FIX_UNIOP_SQU,TS,TW,TF,LINE>(*this, loc);
  }

  template <bool RS, int RW, int RF, int LINE>
  const fix_binop_desc<FIX_BINOP_ADD,TS,TW,TF,RS,RW,RF,LINE>
  add(const fix<RS,RW,RF>& rhs, const fix_loc<LINE>& loc) const {
    return fix_binop_desc<FIX_BINOP_ADD,TS,TW,TF,RS,RW,RF,LINE>(*this, rhs, loc);
  }
  template <bool RS, int RW, int RF, int LINE>
  const fix_binop_desc<FIX_BINOP_SUB,TS,TW,TF,RS,RW,RF,LINE>
  sub(const fix<RS,RW,RF>& rhs, const fix_loc<LINE>& loc) const {
    return fix_binop_desc<FIX_BINOP_SUB,TS,TW,TF,RS,RW,RF,LINE>(*this, rhs, loc);
  }
  template <bool RS, int RW, int RF, int LINE>
  const fix_binop_desc<FIX_BINOP_MUL,TS,TW,TF,RS,RW,RF,LINE>
  mul(const fix<RS,RW,RF>& rhs, const fix_loc<LINE>& loc) const {
    return fix_binop_desc<FIX_BINOP_MUL,TS,TW,TF,RS,RW,RF,LINE>(*this, rhs, loc);
  }
  template <bool RS, int RW, int RF, int LINE>
  const fix_binop_desc<FIX_BINOP_DIV,TS,TW,TF,RS,RW,RF,LINE>
  div(const fix<RS,RW,RF>& rhs, const fix_loc<LINE>& loc) const {
    return fix_binop_desc<FIX_BINOP_DIV,TS,TW,TF,RS,RW,RF,LINE>(*this, rhs, loc);
  }

  const fix_uniop_desc<FIX_UNIOP_CNV,TS,TW,TF,0>
  cnv() const {
    return cnv(fix_loc<0>(NULL));
  }
  const fix_uniop_desc<FIX_UNIOP_NEG,TS,TW,TF,0>
  operator-() const {
    return neg(fix_loc<0>(NULL));
  }
  const fix_uniop_desc<FIX_UNIOP_SQU,TS,TW,TF,0>
  squ() const {
    return squ(fix_loc<0>(NULL));
  }

  template <bool RS, int RW, int RF>
  const fix_binop_desc<FIX_BINOP_ADD,TS,TW,TF,RS,RW,RF,0>
  operator+(const fix<RS,RW,RF>& rhs) const {
    return add(rhs, fix_loc<0>(NULL));
  }
  template <bool RS, int RW, int RF>
  const fix_binop_desc<FIX_BINOP_SUB,TS,TW,TF,RS,RW,RF,0>
  operator-(const fix<RS,RW,RF>& rhs) const {
    return sub(rhs, fix_loc<0>(NULL));
  }
  template <bool RS, int RW, int RF>
  const fix_binop_desc<FIX_BINOP_MUL,TS,TW,TF,RS,RW,RF,0>
  operator*(const fix<RS,RW,RF>& rhs) const {
    return mul(rhs, fix_loc<0>(NULL));
  }
  template <bool RS, int RW, int RF>
  const fix_binop_desc<FIX_BINOP_DIV,TS,TW,TF,RS,RW,RF,0>
  operator/(const fix<RS,RW,RF>& rhs) const {
    return div(rhs, fix_loc<0>(NULL));
  }

};

template <int TI, int TF>
struct U : fix<false,(TI+TF),TF> {
  static const bool SIGNED = false;
  U() : fix<false,(TI+TF),TF>() {}
  template <typename T> U(T x) : fix<false,(TI+TF),TF>(x) {}
};

template <int TI, int TF>
struct S : fix<true,(TI+TF),TF> {
  static const bool SIGNED = true;
  S() : fix<true,(TI+TF),TF>() {}
  template <typename T> S(T x) : fix<true,(TI+TF),TF>(x) {}
};

#define fix_ftoi(a)       (fix_ftoi_desc<__LINE__>((a),fix_loc<__LINE__>(__FILE__)))
#define fix_itof(a)       ((a).itof())
#define fix_hex(a)        ((a).hex())
#define fix_inv(a)        ((a).inv())
#define fix_sft(a,n)      ((a).sft<(n)>())
#define fix_cnv(a)        ((a).cnv(fix_loc<__LINE__>(__FILE__)))
#define fix_neg(a)        ((a).neg(fix_loc<__LINE__>(__FILE__)))
#define fix_squ(a)        ((a).squ(fix_loc<__LINE__>(__FILE__)))
#define fix_add(a,b)      ((a).add((b),fix_loc<__LINE__>(__FILE__)))
#define fix_addci(a,b,ci) ((a).add((b),fix_loc<__LINE__>(__FILE__)) + (ci))
#define fix_sub(a,b)      ((a).sub((b),fix_loc<__LINE__>(__FILE__)))
#define fix_mul(a,b)      ((a).mul((b),fix_loc<__LINE__>(__FILE__)))
#define fix_div(a,b)      ((a).div((b),fix_loc<__LINE__>(__FILE__)))

#endif
