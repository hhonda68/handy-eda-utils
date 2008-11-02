/*
 * Lightweight restricted-width integer library for C.
 *
 * Copyright (c) 2008 the handy-eda-utils develeper(s).
 * Distributed under the MIT License.
 * (See accompanying file COPYING or copy at
 *  http://www.opensource.org/licenses/mit-license.php.)
 *
 * Last modified on November 2007.
 */

#ifndef FX_H_INCLUDED
#define FX_H_INCLUDED

/* Experimental definitions */

//#define FX_CHECK_OVERFLOW

/****************************************************************
 *
 * Usage example:
 *   y = fx_cnv(a,12, 11);
 *   y = fx_add(a,12, b,12, 13);
 *   y = fx_sub(a,12, b,12, 13);
 *   y = fx_clamp(a,12, 11);
 *   printf("%0*x", fix_hex(a,12));  ... for debug-printf
 *
 * fix_cnv   : format conversion
 *
 * fix_cnv() always sign-extends upper bits of the output.
 * If you want simple fast assignments,
 * use "y = a;" with a comment describing the fixed-point format.
 *
 ****************************************************************/

#ifdef FX_CHECK_OVERFLOW

extern void fx_overflow_error(const char *file, int line);
extern bool fx_overflow_flag;

#define fx_ovfchk(in,size)							\
  do {										\
    int fx_ovfchk_in_ = (in);							\
    int fx_ovfchk_size_ = (size);						\
    if ((fx_ovfchk_in_ >> (fx_ovfchk_size_-1)) != (fx_ovfchk_in_ >> 31)) {	\
      fx_overflow_error(__FILE__, __LINE__);					\
    }										\
  } while (0)

#else

#define	fx_ovfchk(in,size)     do { (void)(in);(void)(size); } while (0)

#endif

#define	fx_slice(in, size)						\
  ({									\
    int fx_slice_in_ = (in);						\
    int fx_slice_size_ = (size);					\
    fx_ovfchk(fx_slice_in_, fx_slice_size_);				\
    (fx_slice_in_ << (32-fx_slice_size_)) >> (32-fx_slice_size_);	\
  })

/****************************************************************/

#define	fx_cnv(a,aw, ow)		\
  ({					\
    (void)(aw);				\
    fx_slice((a),(ow));			\
  })

#define fx_add(a,aw,b,bw,ow)			\
  ({						\
    (void)(aw);					\
    (void)(bw);					\
    fx_slice((a)+(b), (ow));			\
  })

#define fx_sub(a,aw,b,bw,ow)			\
  ({						\
    (void)(aw);					\
    (void)(bw);					\
    fx_slice((a)-(b), (ow));			\
  })

#define fx_clamp(a,aw,ow)								\
  ({											\
    int fx_clamp_a_ = (a);								\
    int fx_clamp_ow_ = (ow);								\
    int fx_clamp_signmask_ = (fx_clamp_a_ >> 31);					\
    unsigned int fx_clamp_tmp_ = fx_clamp_a_ ^ fx_clamp_signmask_;			\
    unsigned int fx_clamp_owmask_ = (1<<(fx_clamp_ow_-1)) - 1;				\
    bool fx_clamp_overflow_ = ((fx_clamp_tmp_ & (~fx_clamp_owmask_)) != 0);		\
    unsigned int fx_clamp_abs_ = fx_clamp_overflow_ ? fx_clamp_owmask_ : fx_clamp_tmp_;	\
    int fx_clamp_o_ = fx_clamp_abs_ ^ fx_clamp_signmask_;				\
    (void)(aw);										\
    fx_clamp_o_;									\
  })

#define fx_hex(a,aw)   (((aw)+3)>>2), ((a)&((1<<(aw))-1))

#endif
