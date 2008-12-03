/*
 * Lightweight fixed-point library for C.
 *
 * Copyright (c) 2008 the handy-eda-utils developer(s).
 * Distributed under the MIT License.
 * (See accompanying file COPYING or copy at
 *  http://www.opensource.org/licenses/mit-license.php.)
 *
 * Last modified on November 2007.
 */

#ifndef FIX_H_INCLUDED
#define FIX_H_INCLUDED

/* Experimental definitions */

#define FIX_CHECK_OVERFLOW
//#define FIX_CHECK_DIVBYZERO
//#define FIX_CHECK_ARG
//#define FIX_CHECK_UNUSED_LSB

/****************************************************************
 *
 * Usage example:
 *   y = fix_cnv(a,U,12,4, U,11,3);
 *   y = fix_sft(a,U,12,4, 3, U,12,1);
 *   y = fix_add(a,U,12,4, b,U,12,4, U,13,4);
 *   y = fix_sub(a,U,12,4, b,U,12,4, U,13,4);
 *   y = fix_neg(a,U,12,4, S,13,4);
 *   y = fix_addci(a,U,12,4, b,U,12,4, ci, U,13,4);
 *   y = fix_mul(a,U,12,4, b,U,12,4, U,24,8);
 *   y = fix_squ(a,U,12,4, U,24,8);
 *   y = fix_div(a,U,12,4, b,U,12,4, U,24,12);
 *   i = fix_ftoi(f, U,12,4);   ... for getting constant
 *   f = fix_itof(i,U,12,4);    ... for debug-printf
 *   printf("%0*x", fix_hex(a,U,12,4));  ... for debug-printf
 *
 * fix_cnv   : format conversion
 * fix_sft   : NOP (shift by constant)
 * fix_addci : add with carry in
 *
 * fix_cnv() always masks (or sign-extends) upper bits of the output.
 * If you want simple fast assignments,
 * use "y = a;" with a comment describing the fixed-point format.
 *
 * fix_sft() does not mask (or sign-extend) upper bits of the output.
 * It only checks whether or not the fixed-point formats of
 * the operands and the shift-amount are consistent.
 * A positive shift-amount represents a left shift.
 * A negative shift-amount represents a right shift.
 *
 * fix_addci() accepts a carry-in bit, assuming that the weight
 * of the carry-in bit is the same as LSB of the "b" operand.
 *
 ****************************************************************/

#define U		0
#define S		1

#ifdef FIX_CHECK_OVERFLOW

extern void fix_overflow_error(const char */*file*/, int /*line*/);
extern bool fix_overflow_flag;

#define	fix_ovfchk(in,sflg,size)							\
  do {											\
    int fix_ovfchk_in_ = (in);								\
    int fix_ovfchk_sflg_ = (sflg);							\
    int fix_ovfchk_size_ = (size);							\
    int fix_ovfchk_err_;								\
    if (fix_ovfchk_sflg_) {								\
      fix_ovfchk_err_ = (((fix_ovfchk_in_ >> (fix_ovfchk_size_-1))			\
			  ^ (fix_ovfchk_in_ >> 31)) != 0);				\
    } else {										\
      fix_ovfchk_err_ = (((unsigned int)fix_ovfchk_in_ >> fix_ovfchk_size_) != 0);	\
    }											\
    if (fix_ovfchk_err_)  fix_overflow_error(__FILE__, __LINE__);			\
  } while (0)
    
#define	fix_ovfchk_ll(in,sflg,size)							\
  do {											\
    long long fix_ovfchk_in_ = (in);							\
    int fix_ovfchk_sflg_ = (sflg);							\
    int fix_ovfchk_size_ = (size);							\
    int fix_ovfchk_err_;								\
    if (fix_ovfchk_sflg_) {								\
      fix_ovfchk_err_ = (((fix_ovfchk_in_ >> (fix_ovfchk_size_-1))			\
			  ^ (fix_ovfchk_in_ >> 63)) != 0);				\
    } else {										\
      fix_ovfchk_err_ = (((unsigned long long)fix_ovfchk_in_>>fix_ovfchk_size_) != 0);	\
    }											\
    if (fix_ovfchk_err_)  fix_overflow_error(__FILE__, __LINE__);			\
  } while (0)
    
#else

#define	fix_ovfchk(in,sflg,size)       do { (void)(in);(void)(sflg);(void)(size); } while (0)
#define	fix_ovfchk_ll(in,sflg,size)    do { (void)(in);(void)(sflg);(void)(size); } while (0)

#endif

#ifdef FIX_CHECK_DIVBYZERO

extern void fix_divbyzero_error_body(const char */*file*/, int /*line*/);
#define	fix_divbyzero_error()	fix_divbyzero_error_body(__FILE__,__LINE__)

#else

#define	fix_divbyzero_error()

#endif

#define	fix_lshift_ll(in,sflg,sft)							\
  ({											\
     long long fix_lshift_ll_in_ = (in);						\
     int fix_lshift_ll_sflg_ = (sflg);							\
     int fix_lshift_ll_sft_ = (sft);							\
     if (fix_lshift_ll_sft_ > 0) {							\
       fix_ovfchk_ll(fix_lshift_ll_in_, fix_lshift_ll_sflg_, 64-fix_lshift_ll_sft_);	\
       fix_lshift_ll_in_ <<= fix_lshift_ll_sft_;					\
     } else if (fix_lshift_ll_sft_ >= -63) {						\
       if (fix_lshift_ll_sflg_) {							\
	 fix_lshift_ll_in_ >>= (-fix_lshift_ll_sft_);					\
       } else {										\
	 fix_lshift_ll_in_ = ((unsigned long long)fix_lshift_ll_in_			\
			      >> (-fix_lshift_ll_sft_));				\
       }										\
     } else {										\
       fix_lshift_ll_in_ = fix_lshift_ll_sflg_ ? (fix_lshift_ll_in_ >> 63) : 0;		\
     }											\
     fix_lshift_ll_in_;									\
  })

#define	fix_lshift(in,sflg,sft)							\
  ({										\
     int fix_lshift_in_ = (in);							\
     int fix_lshift_sflg_ = (sflg);						\
     int fix_lshift_sft_ = (sft);						\
     if (fix_lshift_sft_ > 0) {							\
       fix_ovfchk(fix_lshift_in_, fix_lshift_sflg_, 32-fix_lshift_sft_);	\
       fix_lshift_in_ <<= fix_lshift_sft_;					\
     } else if (fix_lshift_sft_ >= -31) {					\
       if (fix_lshift_sflg_) {							\
	 fix_lshift_in_ >>= (-fix_lshift_sft_);					\
       } else {									\
	 fix_lshift_in_ = (unsigned int)fix_lshift_in_ >> (-fix_lshift_sft_);	\
       }									\
     } else {									\
       fix_lshift_in_ = fix_lshift_sflg_ ? (fix_lshift_in_ >> 31) : 0;		\
     }										\
     fix_lshift_in_;								\
  })

#define	fix_slice(in, sflg, wordlen, lsbpos)					\
  ({										\
    int fix_slice_in_ = (in);							\
    int fix_slice_sflg_ = (sflg);						\
    int fix_slice_wordlen_ = (wordlen);						\
    int fix_slice_lsbpos_ = (lsbpos);						\
    fix_slice_in_ = fix_lshift(fix_slice_in_, fix_slice_sflg_,			\
			       32-(fix_slice_wordlen_+fix_slice_lsbpos_));	\
    (fix_slice_sflg_								\
     ? (int)fix_slice_in_ >> (32-fix_slice_wordlen_)				\
     : (int)((unsigned int)fix_slice_in_ >> (32-fix_slice_wordlen_)));		\
  })

#ifdef FIX_CHECK_ARG
#define fix_slice_arg   fix_slice
#else
#define fix_slice_arg(in,sflg,wordlen,lsbpos)    (in)
#endif

#ifdef FIX_CHECK_UNUSED_LSB
extern void fix_checklsb_warning(int /*type*/, const char */*file*/, int /*line*/);
#define fix_checklsb_add(af,bf,of)				\
  do {								\
    if (((af)>(bf) && (af)>(of)) || ((bf)>(af) && (bf)>(of))) {	\
      fix_checklsb_warning(0,__FILE__,__LINE__);		\
    }								\
  } while (0)
#define fix_checklsb_sub(af,bf,of)						\
  do {										\
    if ((af)>(bf) && (af)>(of))  fix_checklsb_warning(0,__FILE__,__LINE__);	\
    if ((bf)>(af) && (bf)>(of))  fix_checklsb_warning(1,__FILE__,__LINE__);	\
  } while (0)
#else
#define fix_checklsb_add(af,bf,of)
#define fix_checklsb_sub(af,bf,of)
#endif

/****************************************************************/

#define	fix_cnv(a,as,aw,af, os,ow,of)						\
  ({										\
    int fix_cnv_a_ = (a);							\
    int fix_cnv_as_ = (as);							\
    int fix_cnv_aw_ = (aw);							\
    int fix_cnv_af_ = (af);							\
    int fix_cnv_os_ = (os);							\
    int fix_cnv_ow_ = (ow);							\
    int fix_cnv_of_ = (of);							\
    int fix_cnv_o_;								\
    if (fix_cnv_as_ == fix_cnv_os_						\
	&& fix_cnv_aw_ == fix_cnv_ow_						\
	&& fix_cnv_af_ == fix_cnv_of_) {					\
      fix_cnv_o_ = fix_slice_arg(fix_cnv_a_, fix_cnv_as_, fix_cnv_aw_, 0);	\
    } else {									\
      fix_cnv_a_ = fix_slice_arg(fix_cnv_a_, fix_cnv_as_, fix_cnv_aw_, 0);	\
      fix_cnv_o_ = fix_slice(fix_cnv_a_, fix_cnv_os_, fix_cnv_ow_,		\
			     fix_cnv_af_ - fix_cnv_of_);			\
    }										\
    fix_cnv_o_;									\
  })

extern void fix_sft_invalid_error(const char*, int);
#define fix_sft(a,as,aw,af, n, os,ow,of)			\
  ({								\
    if (! ((os) == (as) && (ow) == (aw) && (of) == (af)-(n))) {	\
      fix_sft_invalid_error(__FILE__, __LINE__);		\
    }								\
    (a);							\
  })

#define	fix_add(a,as,aw,af,b,bs,bw,bf,os,ow,of)						\
  ({											\
    int fix_add_a_ = (a);								\
    int fix_add_as_ = (as);								\
    int fix_add_aw_ __attribute__((unused)) = (aw);					\
    int fix_add_af_ = (af);								\
    int fix_add_b_ = (b);								\
    int fix_add_bs_ = (bs);								\
    int fix_add_bw_ __attribute__((unused)) = (bw);					\
    int fix_add_bf_ = (bf);								\
    int fix_add_os_ = (os);								\
    int fix_add_ow_ = (ow);								\
    int fix_add_of_ = (of);								\
    int fix_add_o_, fix_add_tf_;							\
    fix_add_a_ = fix_slice_arg(fix_add_a_, fix_add_as_, fix_add_aw_, 0);		\
    fix_add_b_ = fix_slice_arg(fix_add_b_, fix_add_bs_, fix_add_bw_, 0);		\
    fix_checklsb_add(fix_add_af_,fix_add_bf_,fix_add_of_);				\
    if (fix_add_bf_ > fix_add_af_) {							\
      if (fix_add_af_ > fix_add_of_) {							\
	fix_add_tf_ = fix_add_af_;							\
      } else if (fix_add_bf_ > fix_add_of_) {						\
	fix_add_tf_ = fix_add_of_;							\
      } else {										\
	fix_add_tf_ = fix_add_bf_;							\
      }											\
    } else {										\
      if (fix_add_bf_ > fix_add_of_) {							\
	fix_add_tf_ = fix_add_bf_;							\
      } else if (fix_add_af_ > fix_add_of_) {						\
	fix_add_tf_ = fix_add_of_;							\
      } else {										\
	fix_add_tf_ = fix_add_af_;							\
      }											\
    }											\
    fix_add_a_ = fix_lshift(fix_add_a_, fix_add_as_, fix_add_tf_ - fix_add_af_);	\
    fix_add_b_ = fix_lshift(fix_add_b_, fix_add_bs_, fix_add_tf_ - fix_add_bf_);	\
    fix_ovfchk(fix_add_a_,fix_add_as_,31);						\
    fix_ovfchk(fix_add_b_,fix_add_bs_,31);						\
    fix_add_o_ = fix_add_a_ + fix_add_b_;						\
    fix_slice(fix_add_o_, fix_add_os_, fix_add_ow_, fix_add_tf_ - fix_add_of_);		\
  })

#define	fix_sub(a,as,aw,af,b,bs,bw,bf,os,ow,of)						\
  ({											\
    int fix_sub_a_ = (a);								\
    int fix_sub_as_ = (as);								\
    int fix_sub_aw_ __attribute__((unused)) = (aw);					\
    int fix_sub_af_ = (af);								\
    int fix_sub_b_ = (b);								\
    int fix_sub_bs_ = (bs);								\
    int fix_sub_bw_ __attribute__((unused)) = (bw);					\
    int fix_sub_bf_ = (bf);								\
    int fix_sub_os_ = (os);								\
    int fix_sub_ow_ = (ow);								\
    int fix_sub_of_ = (of);								\
    int fix_sub_o_, fix_sub_tf_, fix_sub_ci_;						\
    fix_sub_a_ = fix_slice_arg(fix_sub_a_, fix_sub_as_, fix_sub_aw_, 0);		\
    fix_sub_b_ = fix_slice_arg(fix_sub_b_, fix_sub_bs_, fix_sub_bw_, 0);		\
    fix_checklsb_sub(fix_sub_af_,fix_sub_bf_,fix_sub_of_);				\
    if (fix_sub_bf_ > fix_sub_af_) {							\
      if (fix_sub_af_ > fix_sub_of_) {							\
	fix_sub_tf_ = fix_sub_af_;							\
      } else if (fix_sub_bf_ > fix_sub_of_) {						\
	fix_sub_tf_ = fix_sub_of_;							\
      } else {										\
	fix_sub_tf_ = fix_sub_bf_;							\
      }											\
    } else {										\
      if (fix_sub_bf_ > fix_sub_of_) {							\
	fix_sub_tf_ = fix_sub_bf_;							\
      } else if (fix_sub_af_ > fix_sub_of_) {						\
	fix_sub_tf_ = fix_sub_of_;							\
      } else {										\
	fix_sub_tf_ = fix_sub_af_;							\
      }											\
    }											\
    if (fix_sub_tf_ - fix_sub_bf_ >= 0) {						\
      fix_sub_ci_ = 1;									\
    } else if (fix_sub_tf_ - fix_sub_bf_ >= -31) {					\
      fix_sub_ci_ = ((fix_sub_b_ << (32+fix_sub_tf_-fix_sub_bf_)) == 0) ? 1 : 0;	\
    } else {										\
      fix_sub_ci_ = (fix_sub_b_ == 0) ? 1 : 0;						\
    }											\
    fix_sub_a_ = fix_lshift(fix_sub_a_, fix_sub_as_, fix_sub_tf_ - fix_sub_af_);	\
    fix_sub_b_ = fix_lshift(fix_sub_b_, fix_sub_bs_, fix_sub_tf_ - fix_sub_bf_);	\
    fix_ovfchk(fix_sub_a_,fix_sub_as_,31);						\
    fix_ovfchk(fix_sub_b_,fix_sub_bs_,31);						\
    fix_sub_o_ = fix_sub_a_ + ~fix_sub_b_ + fix_sub_ci_;				\
    fix_slice(fix_sub_o_, fix_sub_os_, fix_sub_ow_, fix_sub_tf_ - fix_sub_of_);		\
  })

#define fix_neg(a,as,aw,af, os,ow,of)						\
  ({										\
    int fix_neg_af_ = (af);							\
    fix_sub(0,U,1,fix_neg_af_,(a),(as),(aw),fix_neg_af_,(os),(ow),(of));	\
  })

#define	fix_addci(a,as,aw,af,b,bs,bw,bf,bci,os,ow,of)					   \
  ({											   \
    int fix_addci_a_ = (a);								   \
    int fix_addci_as_ = (as);								   \
    int fix_addci_aw_ __attribute__((unused)) = (aw);					   \
    int fix_addci_af_ = (af);								   \
    int fix_addci_b_ = (b);								   \
    int fix_addci_bs_ = (bs);								   \
    int fix_addci_bw_ __attribute__((unused)) = (bw);					   \
    int fix_addci_bf_ = (bf);								   \
    int fix_addci_bci_ = (bci);								   \
    int fix_addci_os_ = (os);								   \
    int fix_addci_ow_ = (ow);								   \
    int fix_addci_of_ = (of);								   \
    int fix_addci_o_, fix_addci_tf_;							   \
    fix_addci_a_ = fix_slice_arg(fix_addci_a_, fix_addci_as_, fix_addci_aw_, 0);	   \
    fix_addci_b_ = fix_slice_arg(fix_addci_b_, fix_addci_bs_, fix_addci_bw_, 0);	   \
    fix_checklsb_add(fix_addci_af_,fix_addci_bf_,fix_addci_of_);			   \
    fix_addci_tf_ = (fix_addci_bf_ > fix_addci_af_) ? fix_addci_bf_ : fix_addci_af_;	   \
    fix_addci_a_ = fix_lshift(fix_addci_a_, fix_addci_as_, fix_addci_tf_ - fix_addci_af_); \
    fix_addci_b_ = fix_lshift(fix_addci_b_, fix_addci_bs_, fix_addci_tf_ - fix_addci_bf_); \
    fix_addci_bci_ = fix_lshift(fix_addci_bci_, U, fix_addci_tf_ - fix_addci_bf_);	   \
    fix_ovfchk(fix_addci_a_,fix_addci_as_,31);						   \
    fix_ovfchk(fix_addci_b_,fix_addci_bs_,31);						   \
    fix_addci_o_ = fix_addci_a_ + fix_addci_b_ + fix_addci_bci_;			   \
    fix_slice(fix_addci_o_, fix_addci_os_, fix_addci_ow_, fix_addci_tf_ - fix_addci_of_);  \
  })

#define	fix_mul(a,as,aw,af,b,bs,bw,bf,os,ow,of)					\
  ({										\
    int fix_mul_a_ = (a);							\
    int fix_mul_as_ __attribute__((unused)) = (as);				\
    int fix_mul_aw_ __attribute__((unused)) = (aw);				\
    int fix_mul_af_ = (af);							\
    int fix_mul_b_ = (b);							\
    int fix_mul_bs_ __attribute__((unused)) = (bs);				\
    int fix_mul_bw_ __attribute__((unused)) = (bw);				\
    int fix_mul_bf_ = (bf);							\
    int fix_mul_os_ = (os);							\
    int fix_mul_ow_ = (ow);							\
    int fix_mul_of_ = (of);							\
    long long fix_mul_o_;							\
    fix_mul_a_ = fix_slice_arg(fix_mul_a_, fix_mul_as_, fix_mul_aw_, 0);	\
    fix_mul_b_ = fix_slice_arg(fix_mul_b_, fix_mul_bs_, fix_mul_bw_, 0);	\
    fix_mul_o_ = (long long)fix_mul_a_ * (long long)fix_mul_b_;			\
    fix_mul_o_ = fix_lshift_ll(fix_mul_o_, S,					\
			       fix_mul_of_ - fix_mul_af_ - fix_mul_bf_);	\
    fix_ovfchk_ll(fix_mul_o_, fix_mul_of_, 32);					\
    fix_slice((int)fix_mul_o_, fix_mul_os_, fix_mul_ow_, 0);			\
  })

#define fix_squ(a,as,aw,af,os,ow,of)						\
  ({										\
    int fix_squ_a_ = (a);							\
    int fix_squ_as_ = (as);							\
    int fix_squ_aw_ = (aw);							\
    int fix_squ_af_ = (af);							\
    fix_mul(fix_squ_a_, fix_squ_as_, fix_squ_aw_, fix_squ_af_,			\
	    fix_squ_a_, fix_squ_as_, fix_squ_aw_, fix_squ_af_, (os),(ow),(of));	\
  })

#define	fix_div(a,as,aw,af,b,bs,bw,bf,os,ow,of)						\
  ({											\
    int fix_div_a_ = (a);								\
    int fix_div_as_ __attribute__((unused)) = (as);					\
    int fix_div_aw_ __attribute__((unused)) = (aw);					\
    int fix_div_af_ = (af);								\
    int fix_div_b_ = (b);								\
    int fix_div_bs_ __attribute__((unused)) = (bs);					\
    int fix_div_bw_ __attribute__((unused)) = (bw);					\
    int fix_div_bf_ = (bf);								\
    int fix_div_os_ = (os);								\
    int fix_div_ow_ = (ow);								\
    int fix_div_of_ = (of);								\
    long long fix_div_la_, fix_div_lb_;							\
    int fix_div_o_, fix_div_sft_;							\
    fix_div_la_ = (long long)fix_slice_arg(fix_div_a_, fix_div_as_, fix_div_aw_, 0);	\
    fix_div_lb_ = (long long)fix_slice_arg(fix_div_b_, fix_div_bs_, fix_div_bw_, 0);	\
    if (fix_div_lb_ == 0) {								\
      fix_div_o_ = fix_slice((int)fix_div_la_, fix_div_os_, fix_div_ow_,		\
			     fix_div_af_ - fix_div_of_);				\
      fix_divbyzero_error();								\
    } else {										\
      fix_div_sft_ = fix_div_bf_ - fix_div_af_ + fix_div_of_;				\
      if (fix_div_sft_ >= 0) {								\
	fix_div_la_ = fix_lshift_ll(fix_div_la_, S, fix_div_sft_);			\
      } else {										\
	fix_div_lb_ = fix_lshift_ll(fix_div_lb_, S, -fix_div_sft_);			\
      }											\
      fix_div_la_ /= fix_div_lb_;							\
      fix_ovfchk_ll(fix_div_la_, S, 32);						\
      fix_div_o_ = fix_slice((int)fix_div_la_, fix_div_os_, fix_div_ow_, 0);		\
    }											\
    fix_div_o_;										\
  })

extern int fix_ftoi_body(double,int);
extern double fix_itof_body(int,int);
#define fix_ftoi(a,os,ow,of)  fix_slice(fix_ftoi_body((a),(of)),(os),(ow),0)
#define fix_itof(a,as,aw,af)  fix_itof_body((a),(af))

#define fix_hex(a,as,aw,af)   (((aw)+3)>>2), ((a)&((1<<(aw))-1))

#endif
