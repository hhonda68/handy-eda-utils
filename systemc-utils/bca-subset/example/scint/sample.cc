#ifdef ORG_SYSTEMC
#include <systemc>
#else
#include "sysc/datatypes/int/sc_int.h"
#endif
using sc_dt::sc_int;
using sc_dt::sc_uint;
using sc_dt::sc_biguint;

typedef bool                  b_t;
typedef   signed char         sc_t;
typedef unsigned char         uc_t;
typedef   signed short        ss_t;
typedef unsigned short        us_t;
typedef   signed int          si_t;
typedef unsigned int          ui_t;
typedef   signed long long    sl_t;
typedef unsigned long long    ul_t;

typedef sc_int< 1> s1_t;     typedef sc_uint< 1> u1_t;
typedef sc_int< 2> s2_t;     typedef sc_uint< 2> u2_t;
typedef sc_int< 3> s3_t;     typedef sc_uint< 3> u3_t;
typedef sc_int< 4> s4_t;     typedef sc_uint< 4> u4_t;
typedef sc_int< 5> s5_t;     typedef sc_uint< 5> u5_t;
typedef sc_int< 6> s6_t;     typedef sc_uint< 6> u6_t;
typedef sc_int< 7> s7_t;     typedef sc_uint< 7> u7_t;
typedef sc_int< 8> s8_t;     typedef sc_uint< 8> u8_t;
typedef sc_int< 9> s9_t;     typedef sc_uint< 9> u9_t;
typedef sc_int<10> s10_t;    typedef sc_uint<10> u10_t;
typedef sc_int<11> s11_t;    typedef sc_uint<11> u11_t;
typedef sc_int<12> s12_t;    typedef sc_uint<12> u12_t;
typedef sc_int<13> s13_t;    typedef sc_uint<13> u13_t;
typedef sc_int<14> s14_t;    typedef sc_uint<14> u14_t;
typedef sc_int<15> s15_t;    typedef sc_uint<15> u15_t;
typedef sc_int<16> s16_t;    typedef sc_uint<16> u16_t;
typedef sc_int<17> s17_t;    typedef sc_uint<17> u17_t;

typedef sc_int<20> s20_t;    typedef sc_uint<20> u20_t;

typedef sc_int<30> s30_t;    typedef sc_uint<30> u30_t;
typedef sc_int<31> s31_t;    typedef sc_uint<31> u31_t;
typedef sc_int<32> s32_t;    typedef sc_uint<32> u32_t;
typedef sc_int<33> s33_t;    typedef sc_uint<33> u33_t;
typedef sc_int<34> s34_t;    typedef sc_uint<34> u34_t;
typedef sc_int<35> s35_t;    typedef sc_uint<35> u35_t;

typedef sc_int<40> s40_t;    typedef sc_uint<40> u40_t;
typedef sc_int<41> s41_t;    typedef sc_uint<41> u41_t;
typedef sc_int<42> s42_t;    typedef sc_uint<42> u42_t;
typedef sc_int<43> s43_t;    typedef sc_uint<43> u43_t;
typedef sc_int<44> s44_t;    typedef sc_uint<44> u44_t;
typedef sc_int<45> s45_t;    typedef sc_uint<45> u45_t;

typedef sc_int<61> s61_t;    typedef sc_uint<61> u61_t;
typedef sc_int<62> s62_t;    typedef sc_uint<62> u62_t;
typedef sc_int<63> s63_t;    typedef sc_uint<63> u63_t;
typedef sc_int<64> s64_t;    typedef sc_uint<64> u64_t;

#define CNV(src,dst)  const dst##_t cnv_##src##_##dst(src##_t a) { return a; }

CNV(si,s13)
CNV(ui,s13)
CNV(sl,s13)
CNV(ul,s13)
CNV(s12,s13)
CNV(s13,s13)
CNV(s14,s13)
CNV(s31,s13)
CNV(s32,s13)
CNV(s33,s13)
CNV(s43,s13)
CNV(s63,s13)
CNV(s64,s13)

CNV(si,u13)
CNV(ui,u13)
CNV(sl,u13)
CNV(ul,u13)
CNV(u12,u13)
CNV(u13,u13)
CNV(u14,u13)
CNV(u31,u13)
CNV(u32,u13)
CNV(u33,u13)
CNV(u43,u13)
CNV(u63,u13)
CNV(u64,u13)

CNV(si,s32)
CNV(ui,s32)
CNV(sl,s32)
CNV(ul,s32)
CNV(s13,s32)
CNV(s31,s32)
CNV(s32,s32)
CNV(s33,s32)
CNV(s43,s32)
CNV(s63,s32)
CNV(s64,s32)

CNV(si,u32)
CNV(ui,u32)
CNV(sl,u32)
CNV(ul,u32)
CNV(u13,u32)
CNV(u31,u32)
CNV(u32,u32)
CNV(u33,u32)
CNV(u43,u32)
CNV(u63,u32)
CNV(u64,u32)

CNV(si,s43)
CNV(ui,s43)
CNV(sl,s43)
CNV(ul,s43)
CNV(s13,s43)
CNV(s31,s43)
CNV(s32,s43)
CNV(s33,s43)
CNV(s42,s43)
CNV(s43,s43)
CNV(s44,s43)
CNV(s63,s43)
CNV(s64,s43)

CNV(si,u43)
CNV(ui,u43)
CNV(sl,u43)
CNV(ul,u43)
CNV(u13,u43)
CNV(u31,u43)
CNV(u32,u43)
CNV(u33,u43)
CNV(u42,u43)
CNV(u43,u43)
CNV(u44,u43)
CNV(u63,u43)
CNV(u64,u43)

CNV(si,s64)
CNV(ui,s64)
CNV(sl,s64)
CNV(ul,s64)
CNV(s13,s64)
CNV(s31,s64)
CNV(s32,s64)
CNV(s33,s64)
CNV(s43,s64)
CNV(s63,s64)
CNV(s64,s64)

CNV(si,u64)
CNV(ui,u64)
CNV(sl,u64)
CNV(ul,u64)
CNV(u13,u64)
CNV(u31,u64)
CNV(u32,u64)
CNV(u33,u64)
CNV(u43,u64)
CNV(u63,u64)
CNV(u64,u64)

CNV(b, s1)
CNV(b, u1)
CNV(b, s2)
CNV(b, u2)
CNV(b, s32)
CNV(b, u32)
CNV(b, s43)
CNV(b, u43)
CNV(b, s64)
CNV(b, u64)

CNV(sc,s7)
CNV(sc,u7)
CNV(sc,s8)
CNV(sc,u8)
CNV(sc,s9)
CNV(sc,u9)
CNV(sc,s32)
CNV(sc,u32)
CNV(sc,s43)
CNV(sc,u43)
CNV(sc,s64)
CNV(sc,u64)

CNV(uc,s7)
CNV(uc,u7)
CNV(uc,s8)
CNV(uc,u8)
CNV(uc,s9)
CNV(uc,u9)
CNV(uc,s32)
CNV(uc,u32)
CNV(uc,s43)
CNV(uc,u43)
CNV(uc,s64)
CNV(uc,u64)

CNV(ss,s15)
CNV(ss,u15)
CNV(ss,s16)
CNV(ss,u16)
CNV(ss,s17)
CNV(ss,u17)
CNV(ss,s32)
CNV(ss,u32)
CNV(ss,s43)
CNV(ss,u43)
CNV(ss,s64)
CNV(ss,u64)

CNV(us,s15)
CNV(us,u15)
CNV(us,s16)
CNV(us,u16)
CNV(us,s17)
CNV(us,u17)
CNV(us,s32)
CNV(us,u32)
CNV(us,s43)
CNV(us,u43)
CNV(us,s64)
CNV(us,u64)

#define ASS(src,dst)  void ass_##src##_##dst(const src##_t& a, dst##_t &b) { b = a; }

ASS(si,s13)
ASS(ui,s13)
ASS(sl,s13)
ASS(ul,s13)
ASS(s12,s13)
ASS(s13,s13)
ASS(s14,s13)
ASS(s31,s13)
ASS(s32,s13)
ASS(s33,s13)
ASS(s43,s13)
ASS(s63,s13)
ASS(s64,s13)

ASS(si,u13)
ASS(ui,u13)
ASS(sl,u13)
ASS(ul,u13)
ASS(u12,u13)
ASS(u13,u13)
ASS(u14,u13)
ASS(u31,u13)
ASS(u32,u13)
ASS(u33,u13)
ASS(u43,u13)
ASS(u63,u13)
ASS(u64,u13)

ASS(si,s32)
ASS(ui,s32)
ASS(sl,s32)
ASS(ul,s32)
ASS(s13,s32)
ASS(s31,s32)
ASS(s32,s32)
ASS(s33,s32)
ASS(s43,s32)
ASS(s63,s32)
ASS(s64,s32)

ASS(si,u32)
ASS(ui,u32)
ASS(sl,u32)
ASS(ul,u32)
ASS(u13,u32)
ASS(u31,u32)
ASS(u32,u32)
ASS(u33,u32)
ASS(u43,u32)
ASS(u63,u32)
ASS(u64,u32)

ASS(si,s43)
ASS(ui,s43)
ASS(sl,s43)
ASS(ul,s43)
ASS(s13,s43)
ASS(s31,s43)
ASS(s32,s43)
ASS(s33,s43)
ASS(s42,s43)
ASS(s43,s43)
ASS(s44,s43)
ASS(s63,s43)
ASS(s64,s43)

ASS(si,u43)
ASS(ui,u43)
ASS(sl,u43)
ASS(ul,u43)
ASS(u13,u43)
ASS(u31,u43)
ASS(u32,u43)
ASS(u33,u43)
ASS(u42,u43)
ASS(u43,u43)
ASS(u44,u43)
ASS(u63,u43)
ASS(u64,u43)

ASS(si,s64)
ASS(ui,s64)
ASS(sl,s64)
ASS(ul,s64)
ASS(s13,s64)
ASS(s31,s64)
ASS(s32,s64)
ASS(s33,s64)
ASS(s43,s64)
ASS(s63,s64)
ASS(s64,s64)

ASS(si,u64)
ASS(ui,u64)
ASS(sl,u64)
ASS(ul,u64)
ASS(u13,u64)
ASS(u31,u64)
ASS(u32,u64)
ASS(u33,u64)
ASS(u43,u64)
ASS(u63,u64)
ASS(u64,u64)

ASS(b, s1)
ASS(b, u1)
ASS(b, s2)
ASS(b, u2)
ASS(b, s32)
ASS(b, u32)
ASS(b, s43)
ASS(b, u43)
ASS(b, s64)
ASS(b, u64)

ASS(sc,s7)
ASS(sc,u7)
ASS(sc,s8)
ASS(sc,u8)
ASS(sc,s9)
ASS(sc,u9)
ASS(sc,s32)
ASS(sc,u32)
ASS(sc,s43)
ASS(sc,u43)
ASS(sc,s64)
ASS(sc,u64)

ASS(uc,s7)
ASS(uc,u7)
ASS(uc,s8)
ASS(uc,u8)
ASS(uc,s9)
ASS(uc,u9)
ASS(uc,s32)
ASS(uc,u32)
ASS(uc,s43)
ASS(uc,u43)
ASS(uc,s64)
ASS(uc,u64)

ASS(ss,s15)
ASS(ss,u15)
ASS(ss,s16)
ASS(ss,u16)
ASS(ss,s17)
ASS(ss,u17)
ASS(ss,s32)
ASS(ss,u32)
ASS(ss,s43)
ASS(ss,u43)
ASS(ss,s64)
ASS(ss,u64)

ASS(us,s15)
ASS(us,u15)
ASS(us,s16)
ASS(us,u16)
ASS(us,s17)
ASS(us,u17)
ASS(us,s32)
ASS(us,u32)
ASS(us,s43)
ASS(us,u43)
ASS(us,s64)
ASS(us,u64)

#define ADD(lhs,rhs,dst)  const dst##_t add_##lhs##_##rhs##_##dst(lhs##_t a, rhs##_t b) { return a + b; }

ADD(s13,s13,s13)
ADD(s13,s13,s14)
ADD(s13,s12,s13)
ADD(s13,s12,s14)
ADD(s12,s13,s13)
ADD(s12,s13,s14)
ADD(s12,s12,s13)
ADD(s12,s12,s14)

ADD(s13,u13,s13)
ADD(s13,u13,s14)
ADD(s13,u12,s13)
ADD(s13,u12,s14)
ADD(s12,u13,s13)
ADD(s12,u13,s14)
ADD(s12,u12,s13)
ADD(s12,u12,s14)

ADD(u13,s13,s13)
ADD(u13,s13,s14)
ADD(u13,s12,s13)
ADD(u13,s12,s14)
ADD(u12,s13,s13)
ADD(u12,s13,s14)
ADD(u12,s12,s13)
ADD(u12,s12,s14)

ADD(u13,u13,s13)
ADD(u13,u13,s14)
ADD(u13,u12,s13)
ADD(u13,u12,s14)
ADD(u12,u13,s13)
ADD(u12,u13,s14)
ADD(u12,u12,s13)
ADD(u12,u12,s14)

ADD(s13,s13,u13)
ADD(s13,s13,u14)
ADD(s13,s12,u13)
ADD(s13,s12,u14)
ADD(s12,s13,u13)
ADD(s12,s13,u14)
ADD(s12,s12,u13)
ADD(s12,s12,u14)

ADD(s13,u13,u13)
ADD(s13,u13,u14)
ADD(s13,u12,u13)
ADD(s13,u12,u14)
ADD(s12,u13,u13)
ADD(s12,u13,u14)
ADD(s12,u12,u13)
ADD(s12,u12,u14)

ADD(u13,s13,u13)
ADD(u13,s13,u14)
ADD(u13,s12,u13)
ADD(u13,s12,u14)
ADD(u12,s13,u13)
ADD(u12,s13,u14)
ADD(u12,s12,u13)
ADD(u12,s12,u14)

ADD(u13,u13,u13)
ADD(u13,u13,u14)
ADD(u13,u12,u13)
ADD(u13,u12,u14)
ADD(u12,u13,u13)
ADD(u12,u13,u14)
ADD(u12,u12,u13)
ADD(u12,u12,u14)

ADD(s32,s32,s32)
ADD(s32,s32,s33)  // produces wrong result (limitation of the BCA subset)
ADD(u32,u32,u32)
ADD(u32,u32,u33)  // produces wrong result (limitation of the BCA subset)

ADD(s43,s43,s43)
ADD(s43,s43,s44)
ADD(u43,u43,u43)
ADD(u43,u43,u44)

ADD(s64,s64,s64)
ADD(u64,u64,u64)

const s13_t add_s13_s13_1_s13 (s13_t a, s13_t b)         { return a + b + 1; }
const s14_t add_s13_s13_1_s14 (s13_t a, s13_t b)         { return a + b + 1; }
const s13_t add_s13_s13_b_s13 (s13_t a, s13_t b, bool c) { return a + b + c; }
const s14_t add_s13_s13_b_s14 (s13_t a, s13_t b, bool c) { return a + b + c; }
const s13_t add_s13_s13_u1_s13(s13_t a, s13_t b, u1_t c) { return a + b + c; }
const s14_t add_s13_s13_u1_s14(s13_t a, s13_t b, u1_t c) { return a + b + c; }
const u13_t add_u13_u13_1_u13 (u13_t a, u13_t b)         { return a + b + 1; }
const u14_t add_u13_u13_1_u14 (u13_t a, u13_t b)         { return a + b + 1; }
const u13_t add_u13_u13_b_u13 (u13_t a, u13_t b, bool c) { return a + b + c; }
const u14_t add_u13_u13_b_u14 (u13_t a, u13_t b, bool c) { return a + b + c; }
const u13_t add_u13_u13_u1_u13(u13_t a, u13_t b, u1_t c) { return a + b + c; }
const u14_t add_u13_u13_u1_u14(u13_t a, u13_t b, u1_t c) { return a + b + c; }

#define SUB(lhs,rhs,dst)  const dst##_t sub_##lhs##_##rhs##_##dst(lhs##_t a, rhs##_t b) { return a - b; }

SUB(s13,s13,s13)
SUB(s13,s13,s14)
SUB(s13,s12,s13)
SUB(s13,s12,s14)
SUB(s12,s13,s13)
SUB(s12,s13,s14)
SUB(s12,s12,s13)
SUB(s12,s12,s14)

SUB(s13,u13,s13)
SUB(s13,u13,s14)
SUB(s13,u12,s13)
SUB(s13,u12,s14)
SUB(s12,u13,s13)
SUB(s12,u13,s14)
SUB(s12,u12,s13)
SUB(s12,u12,s14)

SUB(u13,s13,s13)
SUB(u13,s13,s14)
SUB(u13,s12,s13)
SUB(u13,s12,s14)
SUB(u12,s13,s13)
SUB(u12,s13,s14)
SUB(u12,s12,s13)
SUB(u12,s12,s14)

SUB(u13,u13,s13)
SUB(u13,u13,s14)
SUB(u13,u12,s13)
SUB(u13,u12,s14)
SUB(u12,u13,s13)
SUB(u12,u13,s14)
SUB(u12,u12,s13)
SUB(u12,u12,s14)

SUB(s13,s13,u13)
SUB(s13,s13,u14)
SUB(s13,s12,u13)
SUB(s13,s12,u14)
SUB(s12,s13,u13)
SUB(s12,s13,u14)
SUB(s12,s12,u13)
SUB(s12,s12,u14)

SUB(s13,u13,u13)
SUB(s13,u13,u14)
SUB(s13,u12,u13)
SUB(s13,u12,u14)
SUB(s12,u13,u13)
SUB(s12,u13,u14)
SUB(s12,u12,u13)
SUB(s12,u12,u14)

SUB(u13,s13,u13)
SUB(u13,s13,u14)
SUB(u13,s12,u13)
SUB(u13,s12,u14)
SUB(u12,s13,u13)
SUB(u12,s13,u14)
SUB(u12,s12,u13)
SUB(u12,s12,u14)

SUB(u13,u13,u13)
SUB(u13,u13,u14)
SUB(u13,u12,u13)
SUB(u13,u12,u14)
SUB(u12,u13,u13)
SUB(u12,u13,u14)
SUB(u12,u12,u13)
SUB(u12,u12,u14)

SUB(s32,s32,s32)
SUB(s32,s32,s33)  // produces wrong result (limitation of the BCA subset)
SUB(u32,u32,u32)
SUB(u32,u32,u33)  // produces wrong result (limitation of the BCA subset)

SUB(s43,s43,s43)
SUB(s43,s43,s44)
SUB(u43,u43,u43)
SUB(u43,u43,u44)

SUB(s64,s64,s64)
SUB(u64,u64,u64)

const s13_t sub_s13_s13_1_s13 (s13_t a, s13_t b)         { return a - b - 1; }
const s14_t sub_s13_s13_1_s14 (s13_t a, s13_t b)         { return a - b - 1; }
const s13_t sub_s13_s13_b_s13 (s13_t a, s13_t b, bool c) { return a - b - c; }
const s14_t sub_s13_s13_b_s14 (s13_t a, s13_t b, bool c) { return a - b - c; }
const s13_t sub_s13_s13_u1_s13(s13_t a, s13_t b, u1_t c) { return a - b - c; }
const s14_t sub_s13_s13_u1_s14(s13_t a, s13_t b, u1_t c) { return a - b - c; }
const u13_t sub_u13_u13_1_u13 (u13_t a, u13_t b)         { return a - b - 1; }
const u14_t sub_u13_u13_1_u14 (u13_t a, u13_t b)         { return a - b - 1; }
const u13_t sub_u13_u13_b_u13 (u13_t a, u13_t b, bool c) { return a - b - c; }
const u14_t sub_u13_u13_b_u14 (u13_t a, u13_t b, bool c) { return a - b - c; }
const u13_t sub_u13_u13_u1_u13(u13_t a, u13_t b, u1_t c) { return a - b - c; }
const u14_t sub_u13_u13_u1_u14(u13_t a, u13_t b, u1_t c) { return a - b - c; }

const s13_t inc_post_s13(s13_t& a) { return a++; }
const s13_t inc_pre_s13 (s13_t& a) { return ++a; }
const u13_t inc_post_u13(u13_t& a) { return a++; }
const u13_t inc_pre_u13 (u13_t& a) { return ++a; }
const s13_t dec_post_s13(s13_t& a) { return a--; }
const s13_t dec_pre_s13 (s13_t& a) { return --a; }
const u13_t dec_post_u13(u13_t& a) { return a--; }
const u13_t dec_pre_u13 (u13_t& a) { return --a; }

#define LSH(lhs,rhs,dst)  const dst##_t lsh_##lhs##_##rhs##_##dst(lhs##_t a, rhs##_t b) { return a << b; }

const u14_t lsh_u11_2_u14(u11_t a) { return a << 2; }
const u14_t lsh_u11_3_u14(u11_t a) { return a << 3; }
const u14_t lsh_u11_4_u14(u11_t a) { return a << 4; }

LSH(u11,u1,u14)
LSH(u11,u2,u14)
LSH(u11,u3,u14)
LSH(u11,s1,u14)
LSH(u11,s2,u14)
LSH(u11,s3,u14)
LSH(u11,s4,u14)
LSH(u11,b,u14)
LSH(u11,sc,u14)
LSH(u11,uc,u14)
LSH(u11,ss,u14)
LSH(u11,us,u14)
LSH(u11,si,u14)
LSH(u11,ui,u14)
LSH(u11,sl,u14)
LSH(u11,ul,u14)

const s14_t lsh_s11_2_s14(s11_t a) { return a << 2; }
const s14_t lsh_s11_3_s14(s11_t a) { return a << 3; }
const s14_t lsh_s11_4_s14(s11_t a) { return a << 4; }

LSH(s11,u1,s14)
LSH(s11,u2,s14)
LSH(s11,u3,s14)
LSH(s11,s1,s14)
LSH(s11,s2,s14)
LSH(s11,s3,s14)
LSH(s11,s4,s14)
LSH(s11,b,s14)
LSH(s11,sc,s14)
LSH(s11,uc,s14)
LSH(s11,ss,s14)
LSH(s11,us,s14)
LSH(s11,si,s14)
LSH(s11,ui,s14)
LSH(s11,sl,s14)
LSH(s11,ul,s14)

#define RSH(lhs,rhs,dst)  const dst##_t rsh_##lhs##_##rhs##_##dst(lhs##_t a, rhs##_t b) { return a >> b; }

const u11_t rsh_u14_2_u11(u14_t a) { return a >> 2; }
const u11_t rsh_u14_3_u11(u14_t a) { return a >> 3; }
const u11_t rsh_u14_4_u11(u14_t a) { return a >> 4; }

RSH(u14,u1,u14)
RSH(u14,u2,u14)
RSH(u14,u3,u14)
RSH(u14,s1,u14)
RSH(u14,s2,u14)
RSH(u14,s3,u14)
RSH(u14,s4,u14)
RSH(u14,b,u14)
RSH(u14,sc,u14)
RSH(u14,uc,u14)
RSH(u14,ss,u14)
RSH(u14,us,u14)
RSH(u14,si,u14)
RSH(u14,ui,u14)
RSH(u14,sl,u14)
RSH(u14,ul,u14)

RSH(u14,u1,u13)
RSH(u14,u2,u13)
RSH(u14,u3,u13)
RSH(u14,s1,u13)
RSH(u14,s2,u13)
RSH(u14,s3,u13)
RSH(u14,s4,u13)
RSH(u14,b,u13)
RSH(u14,sc,u13)
RSH(u14,uc,u13)
RSH(u14,ss,u13)
RSH(u14,us,u13)
RSH(u14,si,u13)
RSH(u14,ui,u13)
RSH(u14,sl,u13)
RSH(u14,ul,u13)

const s11_t rsh_s14_2_s11(s14_t a) { return a >> 2; }
const s11_t rsh_s14_3_s11(s14_t a) { return a >> 3; }
const s11_t rsh_s14_4_s11(s14_t a) { return a >> 4; }

RSH(s14,u1,s14)
RSH(s14,u2,s14)
RSH(s14,u3,s14)
RSH(s14,s1,s14)
RSH(s14,s2,s14)
RSH(s14,s3,s14)
RSH(s14,s4,s14)
RSH(s14,b,s14)
RSH(s14,sc,s14)
RSH(s14,uc,s14)
RSH(s14,ss,s14)
RSH(s14,us,s14)
RSH(s14,si,s14)
RSH(s14,ui,s14)
RSH(s14,sl,s14)
RSH(s14,ul,s14)

RSH(s14,u1,s13)
RSH(s14,u2,s13)
RSH(s14,u3,s13)
RSH(s14,s1,s13)
RSH(s14,s2,s13)
RSH(s14,s3,s13)
RSH(s14,s4,s13)
RSH(s14,b,s13)
RSH(s14,sc,s13)
RSH(s14,uc,s13)
RSH(s14,ss,s13)
RSH(s14,us,s13)
RSH(s14,si,s13)
RSH(s14,ui,s13)
RSH(s14,sl,s13)
RSH(s14,ul,s13)

bool bitref_s13c_5_b (const s13_t& a) { return a[5]; }
bool bitref_s13_5_b  (      s13_t& a) { return a[5]; }
bool bitref_s13v_5_b (      s13_t  a) { return a[5]; }
int  bitref_s13c_5_si(const s13_t& a) { return a[5]; }
int  bitref_s13_5_si (      s13_t& a) { return a[5]; }
int  bitref_s13v_5_si(      s13_t  a) { return a[5]; }
bool bitref_s13c_si_b(const s13_t& a, int n) { return a[n]; }
bool bitref_s13_si_b (      s13_t& a, int n) { return a[n]; }
bool bitref_s13v_si_b(      s13_t  a, int n) { return a[n]; }
bool bitref_u13c_5_b (const u13_t& a) { return a[5]; }
bool bitref_u13_5_b  (      u13_t& a) { return a[5]; }
bool bitref_u13v_5_b (      u13_t  a) { return a[5]; }
int  bitref_u13c_5_si(const u13_t& a) { return a[5]; }
int  bitref_u13_5_si (      u13_t& a) { return a[5]; }
int  bitref_u13v_5_si(      u13_t  a) { return a[5]; }
bool bitref_u13c_si_b(const u13_t& a, int n) { return a[n]; }
bool bitref_u13_si_b (      u13_t& a, int n) { return a[n]; }
bool bitref_u13v_si_b(      u13_t  a, int n) { return a[n]; }

bool bitref_s43c_5_b (const s43_t& a) { return a[5]; }
bool bitref_s43_5_b  (      s43_t& a) { return a[5]; }
bool bitref_s43v_5_b (      s43_t  a) { return a[5]; }
int  bitref_s43c_5_si(const s43_t& a) { return a[5]; }
int  bitref_s43_5_si (      s43_t& a) { return a[5]; }
int  bitref_s43v_5_si(      s43_t  a) { return a[5]; }
bool bitref_s43c_si_b(const s43_t& a, int n) { return a[n]; }
bool bitref_s43_si_b (      s43_t& a, int n) { return a[n]; }
bool bitref_s43v_si_b(      s43_t  a, int n) { return a[n]; }
bool bitref_u43c_5_b (const u43_t& a) { return a[5]; }
bool bitref_u43_5_b  (      u43_t& a) { return a[5]; }
bool bitref_u43v_5_b (      u43_t  a) { return a[5]; }
int  bitref_u43c_5_si(const u43_t& a) { return a[5]; }
int  bitref_u43_5_si (      u43_t& a) { return a[5]; }
int  bitref_u43v_5_si(      u43_t  a) { return a[5]; }
bool bitref_u43c_si_b(const u43_t& a, int n) { return a[n]; }
bool bitref_u43_si_b (      u43_t& a, int n) { return a[n]; }
bool bitref_u43v_si_b(      u43_t  a, int n) { return a[n]; }

bool bitand_s13c_5_b (const s13_t& a,        bool b) { return a[5] & b; }
bool bitand_s13_5_b  (      s13_t& a,        bool b) { return a[5] & b; }
bool bitand_s13c_si_b(const s13_t& a, int n, bool b) { return a[n] & b; }
bool bitand_s13_si_b (      s13_t& a, int n, bool b) { return a[n] & b; }
bool bitand_b_s13c_5 (bool a, const s13_t& b       ) { return a & b[5]; }
bool bitand_b_s13_5  (bool a,       s13_t& b       ) { return a & b[5]; }
bool bitand_b_s13c_si(bool a, const s13_t& b, int n) { return a & b[n]; }
bool bitand_b_s13_si (bool a,       s13_t& b, int n) { return a & b[n]; }
bool bitand_s13c_5_u11c_7 (const s13_t& a,        const u11_t& b) { return a[5] & b[7]; }
bool bitand_s13_5_u11c_7  (      s13_t& a,        const u11_t& b) { return a[5] & b[7]; }
bool bitand_s13c_si_u11c_7(const s13_t& a, int n, const u11_t& b) { return a[n] & b[7]; }
bool bitand_s13_si_u11c_7 (      s13_t& a, int n, const u11_t& b) { return a[n] & b[7]; }
bool bitand_s13c_5_u11_7  (const s13_t& a,              u11_t& b) { return a[5] & b[7]; }
bool bitand_s13_5_u11_7   (      s13_t& a,              u11_t& b) { return a[5] & b[7]; }
bool bitand_s13c_si_u11_7 (const s13_t& a, int n,       u11_t& b) { return a[n] & b[7]; }
bool bitand_s13_si_u11_7  (      s13_t& a, int n,       u11_t& b) { return a[n] & b[7]; }
bool bitand_s13c_5_u11c_si (const s13_t& a,        const u11_t& b, int m) { return a[5] & b[m]; }
bool bitand_s13_5_u11c_si  (      s13_t& a,        const u11_t& b, int m) { return a[5] & b[m]; }
bool bitand_s13c_si_u11c_si(const s13_t& a, int n, const u11_t& b, int m) { return a[n] & b[m]; }
bool bitand_s13_si_u11c_si (      s13_t& a, int n, const u11_t& b, int m) { return a[n] & b[m]; }
bool bitand_s13c_5_u11_si  (const s13_t& a,              u11_t& b, int m) { return a[5] & b[m]; }
bool bitand_s13_5_u11_si   (      s13_t& a,              u11_t& b, int m) { return a[5] & b[m]; }
bool bitand_s13c_si_u11_si (const s13_t& a, int n,       u11_t& b, int m) { return a[n] & b[m]; }
bool bitand_s13_si_u11_si  (      s13_t& a, int n,       u11_t& b, int m) { return a[n] & b[m]; }

bool bitor_s13c_5_b (const s13_t& a,        bool b) { return a[5] | b; }
bool bitor_s13_5_b  (      s13_t& a,        bool b) { return a[5] | b; }
bool bitor_s13c_si_b(const s13_t& a, int n, bool b) { return a[n] | b; }
bool bitor_s13_si_b (      s13_t& a, int n, bool b) { return a[n] | b; }
bool bitor_b_s13c_5 (bool a, const s13_t& b       ) { return a | b[5]; }
bool bitor_b_s13_5  (bool a,       s13_t& b       ) { return a | b[5]; }
bool bitor_b_s13c_si(bool a, const s13_t& b, int n) { return a | b[n]; }
bool bitor_b_s13_si (bool a,       s13_t& b, int n) { return a | b[n]; }
bool bitor_s13c_5_u11c_7 (const s13_t& a,        const u11_t& b) { return a[5] | b[7]; }
bool bitor_s13_5_u11c_7  (      s13_t& a,        const u11_t& b) { return a[5] | b[7]; }
bool bitor_s13c_si_u11c_7(const s13_t& a, int n, const u11_t& b) { return a[n] | b[7]; }
bool bitor_s13_si_u11c_7 (      s13_t& a, int n, const u11_t& b) { return a[n] | b[7]; }
bool bitor_s13c_5_u11_7  (const s13_t& a,              u11_t& b) { return a[5] | b[7]; }
bool bitor_s13_5_u11_7   (      s13_t& a,              u11_t& b) { return a[5] | b[7]; }
bool bitor_s13c_si_u11_7 (const s13_t& a, int n,       u11_t& b) { return a[n] | b[7]; }
bool bitor_s13_si_u11_7  (      s13_t& a, int n,       u11_t& b) { return a[n] | b[7]; }
bool bitor_s13c_5_u11c_si (const s13_t& a,        const u11_t& b, int m) { return a[5] | b[m]; }
bool bitor_s13_5_u11c_si  (      s13_t& a,        const u11_t& b, int m) { return a[5] | b[m]; }
bool bitor_s13c_si_u11c_si(const s13_t& a, int n, const u11_t& b, int m) { return a[n] | b[m]; }
bool bitor_s13_si_u11c_si (      s13_t& a, int n, const u11_t& b, int m) { return a[n] | b[m]; }
bool bitor_s13c_5_u11_si  (const s13_t& a,              u11_t& b, int m) { return a[5] | b[m]; }
bool bitor_s13_5_u11_si   (      s13_t& a,              u11_t& b, int m) { return a[5] | b[m]; }
bool bitor_s13c_si_u11_si (const s13_t& a, int n,       u11_t& b, int m) { return a[n] | b[m]; }
bool bitor_s13_si_u11_si  (      s13_t& a, int n,       u11_t& b, int m) { return a[n] | b[m]; }

bool bitxor_s13c_5_b (const s13_t& a,        bool b) { return a[5] ^ b; }
bool bitxor_s13_5_b  (      s13_t& a,        bool b) { return a[5] ^ b; }
bool bitxor_s13c_si_b(const s13_t& a, int n, bool b) { return a[n] ^ b; }
bool bitxor_s13_si_b (      s13_t& a, int n, bool b) { return a[n] ^ b; }
bool bitxor_b_s13c_5 (bool a, const s13_t& b       ) { return a ^ b[5]; }
bool bitxor_b_s13_5  (bool a,       s13_t& b       ) { return a ^ b[5]; }
bool bitxor_b_s13c_si(bool a, const s13_t& b, int n) { return a ^ b[n]; }
bool bitxor_b_s13_si (bool a,       s13_t& b, int n) { return a ^ b[n]; }
bool bitxor_s13c_5_u11c_7 (const s13_t& a,        const u11_t& b) { return a[5] ^ b[7]; }
bool bitxor_s13_5_u11c_7  (      s13_t& a,        const u11_t& b) { return a[5] ^ b[7]; }
bool bitxor_s13c_si_u11c_7(const s13_t& a, int n, const u11_t& b) { return a[n] ^ b[7]; }
bool bitxor_s13_si_u11c_7 (      s13_t& a, int n, const u11_t& b) { return a[n] ^ b[7]; }
bool bitxor_s13c_5_u11_7  (const s13_t& a,              u11_t& b) { return a[5] ^ b[7]; }
bool bitxor_s13_5_u11_7   (      s13_t& a,              u11_t& b) { return a[5] ^ b[7]; }
bool bitxor_s13c_si_u11_7 (const s13_t& a, int n,       u11_t& b) { return a[n] ^ b[7]; }
bool bitxor_s13_si_u11_7  (      s13_t& a, int n,       u11_t& b) { return a[n] ^ b[7]; }
bool bitxor_s13c_5_u11c_si (const s13_t& a,        const u11_t& b, int m) { return a[5] ^ b[m]; }
bool bitxor_s13_5_u11c_si  (      s13_t& a,        const u11_t& b, int m) { return a[5] ^ b[m]; }
bool bitxor_s13c_si_u11c_si(const s13_t& a, int n, const u11_t& b, int m) { return a[n] ^ b[m]; }
bool bitxor_s13_si_u11c_si (      s13_t& a, int n, const u11_t& b, int m) { return a[n] ^ b[m]; }
bool bitxor_s13c_5_u11_si  (const s13_t& a,              u11_t& b, int m) { return a[5] ^ b[m]; }
bool bitxor_s13_5_u11_si   (      s13_t& a,              u11_t& b, int m) { return a[5] ^ b[m]; }
bool bitxor_s13c_si_u11_si (const s13_t& a, int n,       u11_t& b, int m) { return a[n] ^ b[m]; }
bool bitxor_s13_si_u11_si  (      s13_t& a, int n,       u11_t& b, int m) { return a[n] ^ b[m]; }

void bitass_u1_s13c_5(u1_t& a, const s13_t& b) { a = b[5]; }
void bitass_u1_s13_5 (u1_t& a,       s13_t& b) { a = b[5]; }

void bitass_s13_5_0 (s13_t& a               ) { a[5] = 0; }
void bitass_s13_5_f (s13_t& a               ) { a[5] = false; }
void bitass_s13_5_1 (s13_t& a               ) { a[5] = 1; }
void bitass_s13_5_t (s13_t& a               ) { a[5] = true; }
void bitass_s13_5_b (s13_t& a,        bool b) { a[5] = b; }
void bitass_s13_12_0(s13_t& a               ) { a[12] = 0; }
void bitass_s13_12_f(s13_t& a               ) { a[12] = false; }
void bitass_s13_12_1(s13_t& a               ) { a[12] = 1; }
void bitass_s13_12_t(s13_t& a               ) { a[12] = true; }
void bitass_s13_12_b(s13_t& a,        bool b) { a[12] = b; }
void bitass_s13_si_0(s13_t& a, int n        ) { a[n] = 0; }
void bitass_s13_si_f(s13_t& a, int n        ) { a[n] = false; }
void bitass_s13_si_1(s13_t& a, int n        ) { a[n] = 1; }
void bitass_s13_si_t(s13_t& a, int n        ) { a[n] = true; }
void bitass_s13_si_b(s13_t& a, int n, bool b) { a[n] = b; }

void bitass_u13_5_0 (u13_t& a               ) { a[5] = 0; }
void bitass_u13_5_f (u13_t& a               ) { a[5] = false; }
void bitass_u13_5_1 (u13_t& a               ) { a[5] = 1; }
void bitass_u13_5_t (u13_t& a               ) { a[5] = true; }
void bitass_u13_5_b (u13_t& a,        bool b) { a[5] = b; }
void bitass_u13_12_0(u13_t& a               ) { a[12] = 0; }
void bitass_u13_12_f(u13_t& a               ) { a[12] = false; }
void bitass_u13_12_1(u13_t& a               ) { a[12] = 1; }
void bitass_u13_12_t(u13_t& a               ) { a[12] = true; }
void bitass_u13_12_b(u13_t& a,        bool b) { a[12] = b; }
void bitass_u13_si_0(u13_t& a, int n        ) { a[n] = 0; }
void bitass_u13_si_f(u13_t& a, int n        ) { a[n] = false; }
void bitass_u13_si_1(u13_t& a, int n        ) { a[n] = 1; }
void bitass_u13_si_t(u13_t& a, int n        ) { a[n] = true; }
void bitass_u13_si_b(u13_t& a, int n, bool b) { a[n] = b; }

void bitass_s43_5_0 (s43_t& a               ) { a[5] = 0; }
void bitass_s43_5_f (s43_t& a               ) { a[5] = false; }
void bitass_s43_5_1 (s43_t& a               ) { a[5] = 1; }
void bitass_s43_5_t (s43_t& a               ) { a[5] = true; }
void bitass_s43_5_b (s43_t& a,        bool b) { a[5] = b; }
void bitass_s43_42_0(s43_t& a               ) { a[42] = 0; }
void bitass_s43_42_f(s43_t& a               ) { a[42] = false; }
void bitass_s43_42_1(s43_t& a               ) { a[42] = 1; }
void bitass_s43_42_t(s43_t& a               ) { a[42] = true; }
void bitass_s43_42_b(s43_t& a,        bool b) { a[42] = b; }
void bitass_s43_si_0(s43_t& a, int n        ) { a[n] = 0; }
void bitass_s43_si_f(s43_t& a, int n        ) { a[n] = false; }
void bitass_s43_si_1(s43_t& a, int n        ) { a[n] = 1; }
void bitass_s43_si_t(s43_t& a, int n        ) { a[n] = true; }
void bitass_s43_si_b(s43_t& a, int n, bool b) { a[n] = b; }

void bitass_u43_5_0 (u43_t& a               ) { a[5] = 0; }
void bitass_u43_5_f (u43_t& a               ) { a[5] = false; }
void bitass_u43_5_1 (u43_t& a               ) { a[5] = 1; }
void bitass_u43_5_t (u43_t& a               ) { a[5] = true; }
void bitass_u43_5_b (u43_t& a,        bool b) { a[5] = b; }
void bitass_u43_42_0(u43_t& a               ) { a[42] = 0; }
void bitass_u43_42_f(u43_t& a               ) { a[42] = false; }
void bitass_u43_42_1(u43_t& a               ) { a[42] = 1; }
void bitass_u43_42_t(u43_t& a               ) { a[42] = true; }
void bitass_u43_42_b(u43_t& a,        bool b) { a[42] = b; }
void bitass_u43_si_0(u43_t& a, int n        ) { a[n] = 0; }
void bitass_u43_si_f(u43_t& a, int n        ) { a[n] = false; }
void bitass_u43_si_1(u43_t& a, int n        ) { a[n] = 1; }
void bitass_u43_si_t(u43_t& a, int n        ) { a[n] = true; }
void bitass_u43_si_b(u43_t& a, int n, bool b) { a[n] = b; }

void bitass_s13_5_s11c_7(s13_t& a, const s11_t& b) { a[5] = b[7]; }
void bitass_s13_5_s11_7 (s13_t& a,       s11_t& b) { a[5] = b[7]; }
void bitass_s13_5_s13c_5(s13_t& a, const s13_t& b) { a[5] = b[5]; }
void bitass_s13_5_s13_5 (s13_t& a,       s13_t& b) { a[5] = b[5]; }

void bitassand_s13_5_b     (s13_t& a, bool         b) { a[5] &= b; }     // not optimal
void bitassand_s13_5_s11c_7(s13_t& a, const s11_t& b) { a[5] &= b[7]; }  // not optimal
void bitassand_s13_5_s11_7 (s13_t& a,       s11_t& b) { a[5] &= b[7]; }  // not optimal

void bitassor_s13_5_b     (s13_t& a, bool         b) { a[5] |= b; }      // not optimal
void bitassor_s13_5_s11c_7(s13_t& a, const s11_t& b) { a[5] |= b[7]; }   // not optimal
void bitassor_s13_5_s11_7 (s13_t& a,       s11_t& b) { a[5] |= b[7]; }   // not optimal

void bitassxor_s13_5_b     (s13_t& a, bool         b) { a[5] ^= b; }     // not optimal
void bitassxor_s13_5_s11c_7(s13_t& a, const s11_t& b) { a[5] ^= b[7]; }  // not optimal
void bitassxor_s13_5_s11_7 (s13_t& a,       s11_t& b) { a[5] ^= b[7]; }  // not optimal

int subref_s13c_7_3_si(const s13_t& a) { return a(7,3); }
int subref_s13_7_3_si (      s13_t& a) { return a(7,3); }

int subref_s43c_7_3_si(const s43_t& a) { return a(7,3); }
int subref_s43_7_3_si (      s43_t& a) { return a(7,3); }

void subass_u4_s13c_7_3(u4_t& a, const s13_t& b) { a = b(7,3); }
void subass_u4_s13_7_3 (u4_t& a,       s13_t& b) { a = b(7,3); }
void subass_u5_s13c_7_3(u5_t& a, const s13_t& b) { a = b(7,3); }
void subass_u5_s13_7_3 (u5_t& a,       s13_t& b) { a = b(7,3); }
void subass_u6_s13c_7_3(u6_t& a, const s13_t& b) { a = b(7,3); }
void subass_u6_s13_7_3 (u6_t& a,       s13_t& b) { a = b(7,3); }

void subass_s13_7_3_10      (s13_t& a                ) { a(7,3) = 10;     }
void subass_s13_7_3_si      (s13_t& a,       si_t   b) { a(7,3) = b;      }
void subass_s13_7_3_s11c_1  (s13_t& a, const s11_t& b) { a(7,3) = b[1];   }
void subass_s13_7_3_s11_1   (s13_t& a,       s11_t& b) { a(7,3) = b[1];   }
void subass_s13_7_3_s11c_8_4(s13_t& a, const s11_t& b) { a(7,3) = b(8,4); }
void subass_s13_7_3_s11_8_4 (s13_t& a,       s11_t& b) { a(7,3) = b(8,4); }
void subass_s11_8_4_s11c_8_4(s11_t& a, const s11_t& b) { a(8,4) = b(8,4); }
void subass_s11_8_4_s11_8_4 (s11_t& a,       s11_t& b) { a(8,4) = b(8,4); }
void subass_s13_7_3_u4c     (s13_t& a, const u4_t&  b) { a(7,3) = b; }  // not optimal
void subass_s13_7_3_u5c     (s13_t& a, const u5_t&  b) { a(7,3) = b; }  // not optimal
void subass_s13_7_3_u6c     (s13_t& a, const u6_t&  b) { a(7,3) = b; }

int concat_bool_bitr(bool              a, const sc_int<15>& b) { return (a,      b[1]  ); }
int concat_bool_subr(bool              a, const sc_int<15>& b) { return (a,      b(8,5)); }
int concat_bool_intc(bool              a, const sc_int<15>& b) { return (a,      b     ); }
int concat_bitr_bool(const sc_int<13>& a, bool b)              { return (a[2],   b     ); }
int concat_bitr_bitr(const sc_int<13>& a, const sc_int<15>& b) { return (a[2],   b[1]  ); }
int concat_bitr_subr(const sc_int<13>& a, const sc_int<15>& b) { return (a[2],   b(8,5)); }
int concat_bitr_intc(const sc_int<13>& a, const sc_int<15>& b) { return (a[2],   b     ); }
int concat_subr_bool(const sc_int<13>& a, bool b)              { return (a(7,3), b     ); }
int concat_subr_bitr(const sc_int<13>& a, const sc_int<15>& b) { return (a(7,3), b[1]  ); }
int concat_subr_subr(const sc_int<13>& a, const sc_int<15>& b) { return (a(7,3), b(8,5)); }
int concat_subr_intc(const sc_int<13>& a, const sc_int<15>& b) { return (a(7,3), b     ); }
int concat_intc_bool(const sc_int<13>& a, bool b)              { return (a,      b     ); }
int concat_intc_bitr(const sc_int<13>& a, const sc_int<15>& b) { return (a,      b[1]  ); }
int concat_intc_subr(const sc_int<13>& a, const sc_int<15>& b) { return (a,      b(8,5)); }
int concat_intc_intc(const sc_int<13>& a, const sc_int<15>& b) { return (a,      b     ); }
int concat_intc_intc(const sc_int<13>& a, const sc_int<13>& b) { return (a,      b     ); }

int concat_bool_bit(bool              a,        sc_int<15>& b) { return (a,      b[1]  ); }
int concat_bool_sub(bool              a,        sc_int<15>& b) { return (a,      b(8,5)); }
int concat_bool_int(bool              a,        sc_int<15>& b) { return (a,      b     ); }
int concat_bitr_bit(const sc_int<13>& a,        sc_int<15>& b) { return (a[2],   b[1]  ); }
int concat_bitr_sub(const sc_int<13>& a,        sc_int<15>& b) { return (a[2],   b(8,5)); }
int concat_bitr_int(const sc_int<13>& a,        sc_int<15>& b) { return (a[2],   b     ); }
int concat_subr_bit(const sc_int<13>& a,        sc_int<15>& b) { return (a(7,3), b[1]  ); }
int concat_subr_sub(const sc_int<13>& a,        sc_int<15>& b) { return (a(7,3), b(8,5)); }
int concat_subr_int(const sc_int<13>& a,        sc_int<15>& b) { return (a(7,3), b     ); }
int concat_intc_bit(const sc_int<13>& a,        sc_int<15>& b) { return (a,      b[1]  ); }
int concat_intc_sub(const sc_int<13>& a,        sc_int<15>& b) { return (a,      b(8,5)); }
int concat_intc_int(const sc_int<13>& a,        sc_int<15>& b) { return (a,      b     ); }

int concat_bit_bool(      sc_int<13>& a, bool b)              { return (a[2],   b     ); }
int concat_bit_bitr(      sc_int<13>& a, const sc_int<15>& b) { return (a[2],   b[1]  ); }
int concat_bit_subr(      sc_int<13>& a, const sc_int<15>& b) { return (a[2],   b(8,5)); }
int concat_bit_intc(      sc_int<13>& a, const sc_int<15>& b) { return (a[2],   b     ); }
int concat_sub_bool(      sc_int<13>& a, bool b)              { return (a(7,3), b     ); }
int concat_sub_bitr(      sc_int<13>& a, const sc_int<15>& b) { return (a(7,3), b[1]  ); }
int concat_sub_subr(      sc_int<13>& a, const sc_int<15>& b) { return (a(7,3), b(8,5)); }
int concat_sub_intc(      sc_int<13>& a, const sc_int<15>& b) { return (a(7,3), b     ); }
int concat_int_bool(      sc_int<13>& a, bool b)              { return (a     , b     ); }
int concat_int_bitr(      sc_int<13>& a, const sc_int<15>& b) { return (a     , b[1]  ); }
int concat_int_subr(      sc_int<13>& a, const sc_int<15>& b) { return (a     , b(8,5)); }
int concat_int_intc(      sc_int<13>& a, const sc_int<15>& b) { return (a     , b     ); }

int concat_bool_cat(bool              a, sc_int<10>& b0, sc_int<11>& b1) { return (a,      (b0(9,2),b1(5,3))); }
int concat_bitr_cat(const sc_int<13>& a, sc_int<10>& b0, sc_int<11>& b1) { return (a[2],   (b0(9,2),b1(5,3))); }
int concat_subr_cat(const sc_int<13>& a, sc_int<10>& b0, sc_int<11>& b1) { return (a(7,3), (b0(9,2),b1(5,3))); }
int concat_intc_cat(const sc_int<13>& a, sc_int<10>& b0, sc_int<11>& b1) { return (a,      (b0(9,2),b1(5,3))); }
int concat_cat_bool(sc_int<10>& a0, sc_int<11>& a1, const sc_int<15>& b) { return (a0(9,2),a1(5,3), b     ); }
int concat_cat_bitr(sc_int<10>& a0, sc_int<11>& a1, const sc_int<15>& b) { return (a0(9,2),a1(5,3), b[1]  ); }
int concat_cat_subr(sc_int<10>& a0, sc_int<11>& a1, const sc_int<15>& b) { return (a0(9,2),a1(5,3), b(8,5)); }
int concat_cat_intc(sc_int<10>& a0, sc_int<11>& a1, const sc_int<15>& b) { return (a0(9,2),a1(5,3), b     ); }

void concatl_bit_bit(sc_int<13>& a, sc_int<15>& b, int x) { (a[2],   b[1]  ) = x; }
void concatl_bit_sub(sc_int<13>& a, sc_int<15>& b, int x) { (a[2],   b(8,5)) = x; }
void concatl_bit_int(sc_int<13>& a, sc_int<15>& b, int x) { (a[2],   b     ) = x; }
void concatl_sub_bit(sc_int<13>& a, sc_int<15>& b, int x) { (a(7,3), b[1]  ) = x; }
void concatl_sub_sub(sc_int<13>& a, sc_int<15>& b, int x) { (a(7,3), b(8,5)) = x; }
void concatl_sub_int(sc_int<13>& a, sc_int<15>& b, int x) { (a(7,3), b     ) = x; }
void concatl_int_bit(sc_int<13>& a, sc_int<15>& b, int x) { (a,      b[1]  ) = x; }
void concatl_int_sub(sc_int<13>& a, sc_int<15>& b, int x) { (a,      b(8,5)) = x; }
void concatl_int_int(sc_int<13>& a, sc_int<15>& b, int x) { (a,      b     ) = x; }

void concatl_bit_cat(sc_int<13>& a, sc_int<10>& b0, sc_int<11>& b1, int x) { (a[2],   (b0(9,2),b1(5,3))) = x; }
void concatl_sub_cat(sc_int<13>& a, sc_int<10>& b0, sc_int<11>& b1, int x) { (a(7,3), (b0(9,2),b1(5,3))) = x; }
void concatl_int_cat(sc_int<13>& a, sc_int<10>& b0, sc_int<11>& b1, int x) { (a,      (b0(9,2),b1(5,3))) = x; }

void concatl_cat_bit(sc_int<10>& a0, sc_int<11>& a1, sc_int<15>& b, int x) { (a0(9,2),a1(5,3), b[1]   ) = x; }
void concatl_cat_sub(sc_int<10>& a0, sc_int<11>& a1, sc_int<15>& b, int x) { (a0(9,2),a1(5,3), b(8,5) ) = x; }
void concatl_cat_int(sc_int<10>& a0, sc_int<11>& a1, sc_int<15>& b, int x) { (a0(9,2),a1(5,3), b      ) = x; }

void concatl_cat_cat(sc_int<10>& a0, sc_int<11>& a1, sc_int<12>& b0, sc_int<13>& b1, int x) {
  (a0(9,2),a1(5,3),(b0(9,2),b1(5,3))) = x;
}

int concat_bit_bit(sc_int<13>& a, sc_int<15>& b) { return (a[2],   b[1]  ); }
int concat_bit_sub(sc_int<13>& a, sc_int<15>& b) { return (a[2],   b(8,5)); }
int concat_bit_int(sc_int<13>& a, sc_int<15>& b) { return (a[2],   b     ); }
int concat_sub_bit(sc_int<13>& a, sc_int<15>& b) { return (a(7,3), b[1]  ); }
int concat_sub_sub(sc_int<13>& a, sc_int<15>& b) { return (a(7,3), b(8,5)); }
int concat_sub_int(sc_int<13>& a, sc_int<15>& b) { return (a(7,3), b     ); }
int concat_int_bit(sc_int<13>& a, sc_int<15>& b) { return (a,      b[1]  ); }
int concat_int_sub(sc_int<13>& a, sc_int<15>& b) { return (a,      b(8,5)); }
int concat_int_int(sc_int<13>& a, sc_int<15>& b) { return (a,      b     ); }

int concat_bit_cat(sc_int<13>& a, sc_int<10>& b0, sc_int<11>& b1) { return (a[2],   (b0(9,2),b1(5,3))); }
int concat_sub_cat(sc_int<13>& a, sc_int<10>& b0, sc_int<11>& b1) { return (a(7,3), (b0(9,2),b1(5,3))); }
int concat_int_cat(sc_int<13>& a, sc_int<10>& b0, sc_int<11>& b1) { return (a,      (b0(9,2),b1(5,3))); }

int concat_cat_bit(sc_int<10>& a0, sc_int<11>& a1, sc_int<15>& b) { return (a0(9,2),a1(5,3), b[1]   ); }
int concat_cat_sub(sc_int<10>& a0, sc_int<11>& a1, sc_int<15>& b) { return (a0(9,2),a1(5,3), b(8,5) ); }
int concat_cat_int(sc_int<10>& a0, sc_int<11>& a1, sc_int<15>& b) { return (a0(9,2),a1(5,3), b      ); }

int concat_cat_cat(sc_int<10>& a0, sc_int<11>& a1, sc_int<12>& b0, sc_int<13>& b1) {
  return (a0(9,2),a1(5,3),(b0(9,2),b1(5,3)));
}

void bigmerge1(sc_biguint<100>& x, u40_t& a, u20_t& b, u30_t& c, u10_t& d) { x = (a,b,c,d); }
void bigmerge2(sc_biguint<100>& x, u40_t& a, u20_t& b, u30_t& c, u10_t& d) { x = (d,c,b,a); }
void bigmerge1a(sc_biguint<100>& x, const u40_t& a, u20_t& b, u30_t& c, u10_t& d) { x = (a,b,c,d); }
void bigmerge1b(sc_biguint<100>& x, u40_t& a, const u20_t& b, u30_t& c, u10_t& d) { x = (a,b,c,d); }
void bigmerge1c(sc_biguint<100>& x, u40_t& a, u20_t& b, const u30_t& c, u10_t& d) { x = (a,b,c,d); }
void bigmerge1d(sc_biguint<100>& x, u40_t& a, u20_t& b, u30_t& c, const u10_t& d) { x = (a,b,c,d); }
void bigmerge1ab(sc_biguint<100>& x, const u40_t& a, const u20_t& b, u30_t& c, u10_t& d) { x = (a,b,c,d); }
void bigsplit1(sc_biguint<100>& x, u40_t& a, u20_t& b, u30_t& c, u10_t& d) { (a,b,c,d) = x; }
void bigsplit2(sc_biguint<100>& x, u40_t& a, u20_t& b, u30_t& c, u10_t& d) { (d,c,b,a) = x; }
void bigsplit1c(const sc_biguint<100>& x, u40_t& a, u20_t& b, u30_t& c, u10_t& d) { (a,b,c,d) = x; }
void bigsplit2c(const sc_biguint<100>& x, u40_t& a, u20_t& b, u30_t& c, u10_t& d) { (d,c,b,a) = x; }
