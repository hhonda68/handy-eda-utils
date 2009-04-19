// -*- c++ -*-
#ifndef COMMON_H
#define COMMON_H

#include "../../include/streamflowc.h"
#include "../../include/sfcut.h"
#include <systemc>
using sc_dt::sc_uint;

typedef sc_uint<1>  u1_t;
typedef sc_uint<2>  u2_t;
typedef sc_uint<3>  u3_t;
typedef sc_uint<4>  u4_t;
typedef sc_uint<5>  u5_t;
typedef sc_uint<6>  u6_t;
typedef sc_uint<7>  u7_t;
typedef sc_uint<8>  u8_t;
typedef sc_uint<9>  u9_t;
typedef sc_uint<10> u10_t;
typedef sc_uint<11> u11_t;
typedef sc_uint<12> u12_t;
typedef sc_uint<13> u13_t;
typedef sc_uint<14> u14_t;
typedef sc_uint<15> u15_t;
typedef sc_uint<16> u16_t;
typedef sc_uint<17> u17_t;
typedef sc_uint<18> u18_t;
typedef sc_uint<19> u19_t;
typedef sc_uint<20> u20_t;
typedef sc_uint<21> u21_t;
typedef sc_uint<22> u22_t;
typedef sc_uint<23> u23_t;
typedef sc_uint<24> u24_t;
typedef sc_uint<25> u25_t;
typedef sc_uint<26> u26_t;
typedef sc_uint<27> u27_t;
typedef sc_uint<28> u28_t;
typedef sc_uint<29> u29_t;
typedef sc_uint<30> u30_t;
typedef sc_uint<31> u31_t;
typedef sc_uint<32> u32_t;

const int BPP = 8;              // bits per pixel
const int MAX_WIDTH = 1920;
const int LG_MAX_WIDTH = 11;    // ceil(log2(MAX_WIDTH))
const int MAX_HEIGHT = 1080;
const int LG_MAX_HEIGHT = 11;   // ceil(log2(MAX_HEIGHT))

typedef sc_uint<BPP>    pel_t;
typedef sc_uint<BPP+1>  pel1_t;
typedef sc_uint<BPP+2>  pel2_t;
typedef sc_uint<BPP+3>  pel3_t;
typedef sc_uint<BPP+4>  pel4_t;

#endif
