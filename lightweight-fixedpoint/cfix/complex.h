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

#ifndef COMPLEX_H_INCLUDED
#define COMPLEX_H_INCLUDED

#define complex_cnv(a,as,aw,af, os,ow,of)						\
  ({											\
    complex complex_cnv_a_ = (a);							\
    int complex_cnv_as_ = (as);								\
    int complex_cnv_aw_ = (aw);								\
    int complex_cnv_af_ = (af);								\
    int complex_cnv_os_ = (os);								\
    int complex_cnv_ow_ = (ow);								\
    int complex_cnv_of_ = (of);								\
    complex complex_cnv_ans_;								\
    complex_cnv_ans_.re = fix_cnv(complex_cnv_a_.re,					\
				  complex_cnv_as_, complex_cnv_aw_, complex_cnv_af_,	\
				  complex_cnv_os_, complex_cnv_ow_, complex_cnv_of_);	\
    complex_cnv_ans_.im = fix_cnv(complex_cnv_a_.im,					\
				  complex_cnv_as_, complex_cnv_aw_, complex_cnv_af_,	\
				  complex_cnv_os_, complex_cnv_ow_, complex_cnv_of_);	\
    complex_cnv_ans_;									\
  })

#define complex_sft	fix_sft

#define complex_add(a,as,aw,af, b,bs,bw,bf, os,ow,of)					\
  ({											\
    complex complex_add_a_ = (a);							\
    int complex_add_as_ = (as);								\
    int complex_add_aw_ = (aw);								\
    int complex_add_af_ = (af);								\
    complex complex_add_b_ = (b);							\
    int complex_add_bs_ = (bs);								\
    int complex_add_bw_ = (bw);								\
    int complex_add_bf_ = (bf);								\
    int complex_add_os_ = (os);								\
    int complex_add_ow_ = (ow);								\
    int complex_add_of_ = (of);								\
    complex complex_add_ans_;								\
    complex_add_ans_.re = fix_add(complex_add_a_.re,					\
				  complex_add_as_, complex_add_aw_, complex_add_af_,	\
				  complex_add_b_.re,					\
				  complex_add_bs_, complex_add_bw_, complex_add_bf_,	\
				  complex_add_os_, complex_add_ow_, complex_add_of_);	\
    complex_add_ans_.im = fix_add(complex_add_a_.im,					\
				  complex_add_as_, complex_add_aw_, complex_add_af_,	\
				  complex_add_b_.im,					\
				  complex_add_bs_, complex_add_bw_, complex_add_bf_,	\
				  complex_add_os_, complex_add_ow_, complex_add_of_);	\
    complex_add_ans_;									\
  })

#define complex_sub(a,as,aw,af, b,bs,bw,bf, os,ow,of)					\
  ({											\
    complex complex_sub_a_ = (a);							\
    int complex_sub_as_ = (as);								\
    int complex_sub_aw_ = (aw);								\
    int complex_sub_af_ = (af);								\
    complex complex_sub_b_ = (b);							\
    int complex_sub_bs_ = (bs);								\
    int complex_sub_bw_ = (bw);								\
    int complex_sub_bf_ = (bf);								\
    int complex_sub_os_ = (os);								\
    int complex_sub_ow_ = (ow);								\
    int complex_sub_of_ = (of);								\
    complex complex_sub_ans_;								\
    complex_sub_ans_.re = fix_sub(complex_sub_a_.re,					\
				  complex_sub_as_, complex_sub_aw_, complex_sub_af_,	\
				  complex_sub_b_.re,					\
				  complex_sub_bs_, complex_sub_bw_, complex_sub_bf_,	\
				  complex_sub_os_, complex_sub_ow_, complex_sub_of_);	\
    complex_sub_ans_.im = fix_sub(complex_sub_a_.im,					\
				  complex_sub_as_, complex_sub_aw_, complex_sub_af_,	\
				  complex_sub_b_.im,					\
				  complex_sub_bs_, complex_sub_bw_, complex_sub_bf_,	\
				  complex_sub_os_, complex_sub_ow_, complex_sub_of_);	\
    complex_sub_ans_;									\
  })

#define complex_neg(a,as,aw,af, os,ow,of)						\
  ({											\
    complex complex_neg_a_ = (a);							\
    int complex_neg_as_ = (as);								\
    int complex_neg_aw_ = (aw);								\
    int complex_neg_af_ = (af);								\
    int complex_neg_os_ = (os);								\
    int complex_neg_ow_ = (ow);								\
    int complex_neg_of_ = (of);								\
    complex complex_neg_ans_;								\
    complex_neg_ans_.re = fix_neg(complex_neg_a_.re,					\
				  complex_neg_as_, complex_neg_aw_, complex_neg_af_,	\
				  complex_neg_os_, complex_neg_ow_, complex_neg_of_);	\
    complex_neg_ans_.im = fix_neg(complex_neg_a_.im,					\
				  complex_neg_as_, complex_neg_aw_, complex_neg_af_,	\
				  complex_neg_os_, complex_neg_ow_, complex_neg_of_);	\
    complex_neg_ans_;									\
  })

#define complex_cnj(a,as,aw,af, os,ow,of)						\
  ({											\
    complex complex_cnj_a_ = (a);							\
    int complex_cnj_as_ = (as);								\
    int complex_cnj_aw_ = (aw);								\
    int complex_cnj_af_ = (af);								\
    int complex_cnj_os_ = (os);								\
    int complex_cnj_ow_ = (ow);								\
    int complex_cnj_of_ = (of);								\
    complex complex_cnj_ans_;								\
    complex_cnj_ans_.re = fix_cnv(complex_cnj_a_.re,					\
				  complex_cnj_as_, complex_cnj_aw_, complex_cnj_af_,	\
				  complex_cnj_os_, complex_cnj_ow_, complex_cnj_of_);	\
    complex_cnj_ans_.im = fix_neg(complex_cnj_a_.im,					\
				  complex_cnj_as_, complex_cnj_aw_, complex_cnj_af_,	\
				  complex_cnj_os_, complex_cnj_ow_, complex_cnj_of_);	\
    complex_cnj_ans_;									\
  })

#define complex_scale(a,as,aw,af, b,bs,bw,bf, os,ow,of)					      \
  ({											      \
    complex complex_scale_a_ = (a);							      \
    int complex_scale_as_ = (as);							      \
    int complex_scale_aw_ = (aw);							      \
    int complex_scale_af_ = (af);							      \
    int complex_scale_b_ = (b);								      \
    int complex_scale_bs_ = (bs);							      \
    int complex_scale_bw_ = (bw);							      \
    int complex_scale_bf_ = (bf);							      \
    int complex_scale_os_ = (os);							      \
    int complex_scale_ow_ = (ow);							      \
    int complex_scale_of_ = (of);							      \
    complex complex_scale_ans_;								      \
    complex_scale_ans_.re = fix_mul(complex_scale_a_.re,				      \
				    complex_scale_as_, complex_scale_aw_, complex_scale_af_,  \
				    complex_scale_b_,					      \
				    complex_scale_bs_, complex_scale_bw_, complex_scale_bf_,  \
				    complex_scale_os_, complex_scale_ow_, complex_scale_of_); \
    complex_scale_ans_.im = fix_mul(complex_scale_a_.im,				      \
				    complex_scale_as_, complex_scale_aw_, complex_scale_af_,  \
				    complex_scale_b_,					      \
				    complex_scale_bs_, complex_scale_bw_, complex_scale_bf_,  \
				    complex_scale_os_, complex_scale_ow_, complex_scale_of_); \
    complex_scale_ans_;									      \
  })

#define complex_sqmag(a,as,aw,af, ps,pw,pf, os,ow,of)					  \
  ({											  \
    complex complex_sqmag_a_ = (a);							  \
    int complex_sqmag_as_ = (as);							  \
    int complex_sqmag_aw_ = (aw);							  \
    int complex_sqmag_af_ = (af);							  \
    int complex_sqmag_ps_ = (ps);							  \
    int complex_sqmag_pw_ = (pw);							  \
    int complex_sqmag_pf_ = (pf);							  \
    int complex_sqmag_os_ = (os);							  \
    int complex_sqmag_ow_ = (ow);							  \
    int complex_sqmag_of_ = (of);							  \
    int complex_sqmag_rr_, complex_sqmag_ii_;						  \
    complex_sqmag_rr_ = fix_squ(complex_sqmag_a_.re,					  \
				complex_sqmag_as_, complex_sqmag_aw_, complex_sqmag_af_,  \
				complex_sqmag_ps_, complex_sqmag_pw_, complex_sqmag_pf_); \
    complex_sqmag_ii_ = fix_squ(complex_sqmag_a_.im,					  \
				complex_sqmag_as_, complex_sqmag_aw_, complex_sqmag_af_,  \
				complex_sqmag_ps_, complex_sqmag_pw_, complex_sqmag_pf_); \
    fix_add(complex_sqmag_rr_,								  \
	    complex_sqmag_ps_, complex_sqmag_pw_, complex_sqmag_pf_,			  \
	    complex_sqmag_ii_,								  \
	    complex_sqmag_ps_, complex_sqmag_pw_, complex_sqmag_pf_,			  \
	    complex_sqmag_os_, complex_sqmag_ow_, complex_sqmag_of_);			  \
  })

typedef struct { double re, im; } complex_dbl;

#define complex_ftoi(a,os,ow,of)							   \
  ({											   \
    complex_dbl complex_ftoi_a_ = (a);							   \
    int complex_ftoi_os_ = (os);							   \
    int complex_ftoi_ow_ = (ow);							   \
    int complex_ftoi_of_ = (of);							   \
    complex complex_ftoi_ans_;								   \
    complex_ftoi_ans_.re = fix_ftoi(complex_ftoi_a_.re,					   \
				    complex_ftoi_os_, complex_ftoi_ow_, complex_ftoi_of_); \
    complex_ftoi_ans_.im = fix_ftoi(complex_ftoi_a_.im,					   \
				    complex_ftoi_os_, complex_ftoi_ow_, complex_ftoi_of_); \
    complex_ftoi_ans_;									   \
  })

#define complex_itof(a,as,aw,af)							   \
  ({											   \
    complex complex_itof_a_ = (a);							   \
    int complex_itof_as_ = (as);							   \
    int complex_itof_aw_ = (aw);							   \
    int complex_itof_af_ = (af);							   \
    complex_dbl complex_itof_ans_;							   \
    complex_itof_ans_.re = fix_itof(complex_itof_a_.re,					   \
				    complex_itof_as_, complex_itof_aw_, complex_itof_af_); \
    complex_itof_ans_.im = fix_itof(complex_itof_a_.im,					   \
				    complex_itof_as_, complex_itof_aw_, complex_itof_af_); \
    complex_itof_ans_;									   \
  })
    
#endif

