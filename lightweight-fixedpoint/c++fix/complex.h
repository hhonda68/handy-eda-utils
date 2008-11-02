// -*- C++ -*-

//
// Lightweight fixed-point library for C++.
//

// Copyright (c) 2008 the handy-eda-utils develeper(s).
// Distributed under the MIT License.
// (See accompanying file COPYING or copy at
//  http://www.opensource.org/licenses/mit-license.php.)

// Last modified on October 2007.

#ifndef COMPLEX_H_INCLUDED
#define COMPLEX_H_INCLUDED

////////////////////////////////////////////////////////////////
//
// Usage example:
//
//  complex_dbl     af = complex_dbl(-1.25, 1.7);
//  complex<S,4,8>  a = af;   // or complex_ftoi(af)
//  complex<S,4,8>  b = complex_ftoi(3.14, -0.13);
//
//  complex<S,4,12> c = a + b;             // or complex_add(a,b)
//  complex<S,4,12> d = a - b;             // or complex_sub(a,b)
//  complex<S,4,12> g = -a;                // or complex_neg(a)
//  complex<S,7, 5> h = a.sft<3>();        // or complex_sft(a,3)
//  complex<S,4,12> k = a.cnv();           // or complex_cnv(a)
//
//  complex<S,4,12> m = a.cnj();           // or complex_cnj(a)
//  U<9,4>          n = a.sqmag<U,8,4>();  // or complex_sqmag(a,U,8,4)
//  complex<S,8,4>  p = b.scale(n);        // or complex_scale(a,n)
//
//  complex_dbl x = a.itof();              // or complex_itof(a)
//  double x_real_part = x.re;             // get real part
//  double x_imag_part = x.im;             // get imaginary part
//  std::string s = a.hex();               // or complex_hex(a)
//  S<4,8> real_part = a.re;               // peek internal data
//  S<4,8> imag_part = a.im;               // peek internal data
//
// complex_cnv()   : format conversion (with overflow check)
// complex_sft()   : NOP (shift by constant)
// complex_cnj()   : conjugate
// complex_sqmag() : square magnitude
// complex_scale() : scaling by a fixed-point real number
// complex_hex()   : hexadecimal string with $display("%h:%h") format

// complex_ftoi() can take one complex_dbl argument or two double
// arguments, and converts the argument(s) to a fixed-point
// complex number.
//
// copmlex_sqmag() computes the square-magnitude of a complex number.
// It first computes the squares of the real/imag parts as the
// specified intermediate format, and then adds them as the
// destination variable format.
//
// complex_itof() returns a complex_dbl which has "re","im" members.
//
// complex_hex() returns a hexadecimal string e.g. "ec0:1b3".
//
////////////////////////////////////////////////////////////////

struct complex_dbl {
  double re, im;
  complex_dbl() {}
  complex_dbl(double re_, double im_) : re(re_), im(im_) {}
};

template <bool TS, int TW, int TF> struct complex_base;

template <int LINE>
struct complex_ftoi_desc {
  double re_value, im_value;
  fix_loc<LINE> loc;
  complex_ftoi_desc(const fix_loc<LINE>& loc_, double re, double im)
    : re_value(re), im_value(im), loc(loc_) {}
  complex_ftoi_desc(const fix_loc<LINE>& loc_, const complex_dbl& src)
    : re_value(src.re), im_value(src.im), loc(loc_) {}
};

enum complex_uniop_type {
  COMPLEX_UNIOP_CNV, COMPLEX_UNIOP_NEG, COMPLEX_UNIOP_CNJ,
};

template <complex_uniop_type OP, bool SS, int SW, int SF, int LINE, int N = 0>
struct complex_uniop_desc {
  complex_base<SS,SW,SF> src;
  fix_loc<LINE> loc;
  complex_uniop_desc(const complex_base<SS,SW,SF>& src_, const fix_loc<LINE>& loc_)
    : src(src_), loc(loc_) {}
};

enum complex_binop_type {
  COMPLEX_BINOP_ADD, COMPLEX_BINOP_SUB,
};

template <complex_binop_type OP, bool LS, int LW, int LF, bool RS, int RW, int RF, int LINE>
struct complex_binop_desc {
  complex_base<LS,LW,LF> lhs;
  complex_base<RS,RW,RF> rhs;
  fix_loc<LINE> loc;
  complex_binop_desc(const complex_base<LS,LW,LF>& lhs_, const complex_base<RS,RW,RF>& rhs_,
		     const fix_loc<LINE>& loc_)
    : lhs(lhs_), rhs(rhs_), loc(loc_) {}
};

template <bool LS, int LW, int LF, bool RS, int RW, int RF, int LINE>
struct complex_scale_desc {
  complex_base<LS,LW,LF> lhs;
  fix<RS,RW,RF> rhs;
  fix_loc<LINE> loc;
  complex_scale_desc(const complex_base<LS,LW,LF>& lhs_, const fix<RS,RW,RF>& rhs_,
		     const fix_loc<LINE>& loc_)
    : lhs(lhs_), rhs(rhs_), loc(loc_) {}
};

template <bool TS, int TW, int TF>
struct complex_base {
  fix<TS,TW,TF> re, im;
  complex_base() {}
  complex_base(const complex_dbl& rhs) : re(rhs.re), im(rhs.im) {}
  complex_base(const complex_base& rhs) : re(rhs.re), im(rhs.im) {}
  complex_base& operator=(const complex_base& rhs) {
    re = rhs.re;
    im = rhs.im;
    return *this;
  }

  // Inhibit assignments between different formats
  template <bool SS, int SW, int SF>
  complex_base(const complex_base<SS,SW,SF>& rhs) {
    enum { FIX_ASSIGN_FORMAT_ERROR = 0 };
    typedef char dummy[FIX_ASSIGN_FORMAT_ERROR];  // raise compilation error intentionally
  }
  template <bool SS, int SW, int SF>
  complex_base& operator=(const complex_base<SS,SW,SF>& rhs) {
    enum { FIX_ASSIGN_FORMAT_ERROR = 0 };
    typedef char dummy[FIX_ASSIGN_FORMAT_ERROR];  // raise compilation error intentionally
  }

  const complex_dbl itof() const {
    return complex_dbl(re.itof(), im.itof());
  }
  const std::string hex() const {
    return re.hex() + ":" + im.hex();
  }
  template <int N>
  const complex_base<TS,TW,(TF-N)>
  sft() const {
    complex_base<TS,TW,(TF-N)> ans;
    ans.re = re.sft<N>();
    ans.im = im.sft<N>();
    return ans;
  }

  template <int LINE>
  complex_base(const complex_ftoi_desc<LINE>& desc)
    : re(fix_ftoi_desc<LINE>(desc.re_value, desc.loc)),
      im(fix_ftoi_desc<LINE>(desc.im_value, desc.loc)) {}

  template <complex_uniop_type OP, bool SS, int SW, int SF, int LINE>
  complex_base(const complex_uniop_desc<OP,SS,SW,SF,LINE>& desc) {}

  template <bool SS, int SW, int SF, int LINE>
  complex_base(const complex_uniop_desc<COMPLEX_UNIOP_CNV,SS,SW,SF,LINE>& desc)
    : re(fix_uniop_desc<FIX_UNIOP_CNV,SS,SW,SF,LINE>(desc.src.re, desc.loc)),
      im(fix_uniop_desc<FIX_UNIOP_CNV,SS,SW,SF,LINE>(desc.src.im, desc.loc)) {}

  template <bool SS, int SW, int SF, int LINE>
  complex_base(const complex_uniop_desc<COMPLEX_UNIOP_NEG,SS,SW,SF,LINE>& desc)
    : re(fix_uniop_desc<FIX_UNIOP_NEG,SS,SW,SF,LINE>(desc.src.re, desc.loc)),
      im(fix_uniop_desc<FIX_UNIOP_NEG,SS,SW,SF,LINE>(desc.src.im, desc.loc)) {}

  template <bool SS, int SW, int SF, int LINE>
  complex_base(const complex_uniop_desc<COMPLEX_UNIOP_CNJ,SS,SW,SF,LINE>& desc)
    : re(fix_uniop_desc<FIX_UNIOP_CNV,SS,SW,SF,LINE>(desc.src.re, desc.loc)),
      im(fix_uniop_desc<FIX_UNIOP_NEG,SS,SW,SF,LINE>(desc.src.im, desc.loc)) {}

  template <bool LS, int LW, int LF, bool RS, int RW, int RF, int LINE>
  complex_base(const complex_binop_desc<COMPLEX_BINOP_ADD,LS,LW,LF,RS,RW,RF,LINE>& desc)
    : re(fix_binop_desc<FIX_BINOP_ADD,LS,LW,LF,RS,RW,RF,LINE>
	 (desc.lhs.re, desc.rhs.re, desc.loc)),
      im(fix_binop_desc<FIX_BINOP_ADD,LS,LW,LF,RS,RW,RF,LINE>
	 (desc.lhs.im, desc.rhs.im, desc.loc)) {}

  template <bool LS, int LW, int LF, bool RS, int RW, int RF, int LINE>
  complex_base(const complex_binop_desc<COMPLEX_BINOP_SUB,LS,LW,LF,RS,RW,RF,LINE>& desc)
    : re(fix_binop_desc<FIX_BINOP_SUB,LS,LW,LF,RS,RW,RF,LINE>
	 (desc.lhs.re, desc.rhs.re, desc.loc)),
      im(fix_binop_desc<FIX_BINOP_SUB,LS,LW,LF,RS,RW,RF,LINE>
	 (desc.lhs.im, desc.rhs.im, desc.loc)) {}

  template <bool LS, int LW, int LF, bool RS, int RW, int RF, int LINE>
  complex_base(const complex_scale_desc<LS,LW,LF,RS,RW,RF,LINE>& desc)
    : re(fix_binop_desc<FIX_BINOP_MUL,LS,LW,LF,RS,RW,RF,LINE>
	 (desc.lhs.re, desc.rhs, desc.loc)),
      im(fix_binop_desc<FIX_BINOP_MUL,LS,LW,LF,RS,RW,RF,LINE>
	 (desc.lhs.im, desc.rhs, desc.loc)) {}

  template <int LINE>
  const complex_uniop_desc<COMPLEX_UNIOP_CNV,TS,TW,TF,LINE>
  cnv(const fix_loc<LINE>& loc) const {
    return complex_uniop_desc<COMPLEX_UNIOP_CNV,TS,TW,TF,LINE>(*this, loc);
  }
  template <int LINE>
  const complex_uniop_desc<COMPLEX_UNIOP_NEG,TS,TW,TF,LINE>
  neg(const fix_loc<LINE>& loc) const {
    return complex_uniop_desc<COMPLEX_UNIOP_NEG,TS,TW,TF,LINE>(*this, loc);
  }
  template <int LINE>
  const complex_uniop_desc<COMPLEX_UNIOP_CNJ,TS,TW,TF,LINE>
  cnj(const fix_loc<LINE>& loc) const {
    return complex_uniop_desc<COMPLEX_UNIOP_CNJ,TS,TW,TF,LINE>(*this, loc);
  }

  template <bool RS, int RW, int RF, int LINE>
  const complex_binop_desc<COMPLEX_BINOP_ADD,TS,TW,TF,RS,RW,RF,LINE>
  add(const complex_base<RS,RW,RF>& rhs, const fix_loc<LINE>& loc) const {
    return complex_binop_desc<COMPLEX_BINOP_ADD,TS,TW,TF,RS,RW,RF,LINE>(*this, rhs, loc);
  }
  template <bool RS, int RW, int RF, int LINE>
  const complex_binop_desc<COMPLEX_BINOP_SUB,TS,TW,TF,RS,RW,RF,LINE>
  sub(const complex_base<RS,RW,RF>& rhs, const fix_loc<LINE>& loc) const {
    return complex_binop_desc<COMPLEX_BINOP_SUB,TS,TW,TF,RS,RW,RF,LINE>(*this, rhs, loc);
  }

  template <template <int MI, int MF> class MS, int MI, int MF, int LINE>
  const fix_binop_desc<FIX_BINOP_ADD,
		       MS<MI,MF>::SIGNED,(MI+MF),MF,
		       MS<MI,MF>::SIGNED,(MI+MF),MF,
		       LINE>
  sqmag(const fix_loc<LINE>& loc) const {
    MS<MI,MF> rr = re.squ(loc);
    MS<MI,MF> ii = im.squ(loc);
    return rr.add(ii, loc);
  }

  template <bool RS, int RW, int RF, int LINE>
  const complex_scale_desc<TS,TW,TF,RS,RW,RF,LINE>
  scale(const fix<RS,RW,RF>& rhs, const fix_loc<LINE>& loc) const {
    return complex_scale_desc<TS,TW,TF,RS,RW,RF,LINE>(*this, rhs, loc);
  }

  const complex_uniop_desc<COMPLEX_UNIOP_CNV,TS,TW,TF,0>
  cnv() const {
    return cnv(fix_loc<0>(NULL));
  }
  const complex_uniop_desc<COMPLEX_UNIOP_NEG,TS,TW,TF,0>
  operator-() const {
    return neg(fix_loc<0>(NULL));
  }
  const complex_uniop_desc<COMPLEX_UNIOP_CNJ,TS,TW,TF,0>
  cnj() const {
    return cnj(fix_loc<0>(NULL));
  }

  template <bool RS, int RW, int RF>
  const complex_binop_desc<COMPLEX_BINOP_ADD,TS,TW,TF,RS,RW,RF,0>
  operator+(const complex_base<RS,RW,RF>& rhs) const {
    return add(rhs, fix_loc<0>(NULL));
  }
  template <bool RS, int RW, int RF>
  const complex_binop_desc<COMPLEX_BINOP_SUB,TS,TW,TF,RS,RW,RF,0>
  operator-(const complex_base<RS,RW,RF>& rhs) const {
    return sub(rhs, fix_loc<0>(NULL));
  }

  template <template <int MI, int MF> class MS, int MI, int MF>
  const fix_binop_desc<FIX_BINOP_ADD,
		       MS<MI,MF>::SIGNED,(MI+MF),MF,
		       MS<MI,MF>::SIGNED,(MI+MF),MF,
		       0>
  sqmag() const {
    return sqmag<MS,MI,MF>(fix_loc<0>(NULL));
  }

  template <bool RS, int RW, int RF>
  const complex_scale_desc<TS,TW,TF,RS,RW,RF,0>
  scale(const fix<RS,RW,RF>& rhs) const {
    return scale(rhs, fix_loc<0>(NULL));
  }

};

template <template <int TI, int TF> class TS, int TI, int TF>
struct complex : complex_base<TS<TI,TF>::SIGNED,(TI+TF),TF> {
  static const bool SIGNED = TS<TI,TF>::SIGNED;
  complex() : complex_base<SIGNED,(TI+TF),TF>() {}
  template <typename T> complex(T x) : complex_base<SIGNED,(TI+TF),TF>(x) {}
};

#define complex_ftoi(...)  (complex_ftoi_desc<__LINE__>(fix_loc<__LINE__>(__FILE__),\
                           __VA_ARGS__))
#define complex_itof(a)    ((a).itof())
#define complex_hex(a)     ((a).hex())
#define complex_sft(a,n)   ((a).sft<(n)>())
#define complex_cnv(a)     ((a).cnv(fix_loc<__LINE__>(__FILE__)))
#define complex_neg(a)     ((a).neg(fix_loc<__LINE__>(__FILE__)))
#define complex_cnj(a)     ((a).cnj(fix_loc<__LINE__>(__FILE__)))
#define complex_add(a,b)   ((a).add((b),fix_loc<__LINE__>(__FILE__)))
#define complex_sub(a,b)   ((a).sub((b),fix_loc<__LINE__>(__FILE__)))
#define complex_sqmag(a,MS,MI,MF) ((a).sqmag<MS,MI,MF>(fix_loc<__LINE__>(__FILE__)))
#define complex_scale(a,b) ((a).scale((b),fix_loc<__LINE__>(__FILE__)))

#endif

