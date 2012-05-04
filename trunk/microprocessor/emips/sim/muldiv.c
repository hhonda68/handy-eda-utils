/*
 * EMIPS : Embedding MIPS Microprocessor
 *
 * Copyright (c) 2012 the handy-eda-utils developer(s).
 * Distributed under the MIT License.
 * (See accompanying file COPYING or copy at
 *  http://www.opensource.org/licenses/mit-license.php.)
 */

extern void myputc(char);

typedef unsigned int uint;
typedef long long llong;
typedef unsigned long long ullong;

void print_hex(uint x)
{
  int i;
  myputc(' ');
  for (i=7; i>=0; i--)  myputc("0123456789abcdef"[(x>>(i*4))&15]);
}

static inline ullong asm_mult(int a, int b)
{
  uint hi, lo;
  asm("mult %2,%3 ; mflo %0 ; mfhi %1" : "=r" (lo), "=r" (hi) : "r" (a), "r" (b) : "hi", "lo");
  return (((ullong)hi)<<32) | lo;
}

static inline ullong asm_multu(uint a, uint b)
{
  uint hi, lo;
  asm("multu %2,%3 ; mflo %0 ; mfhi %1" : "=r" (lo), "=r" (hi) : "r" (a), "r" (b) : "hi", "lo");
  return (((ullong)hi)<<32) | lo;
}

static inline ullong asm_div(int a, int b)
{
  uint hi, lo;
  asm("div $0,%2,%3 ; mflo %0 ; mfhi %1" : "=r" (lo), "=r" (hi) : "r" (a), "r" (b) : "hi", "lo");
  return (((ullong)hi)<<32) | lo;
}

static inline ullong asm_divu(int a, int b)
{
  uint hi, lo;
  asm("divu $0,%2,%3 ; mflo %0 ; mfhi %1" : "=r" (lo), "=r" (hi) : "r" (a), "r" (b) : "hi", "lo");
  return (((ullong)hi)<<32) | lo;
}

int main(void)
{
  static const int the_tab[11] = {
    0, 1, 123, 0x00123456, 0x12345678, 0x7fffffff, 0x80000000, 0xfedcba98, 0xfffedcba, -123, -1,
  };
  int i, j;
  for (i=0; i<11; i++) {
    int rs = the_tab[i];
    for (j=0; j<11; j++) {
      int rt = the_tab[j];
#if 0
      llong prds = (llong)rs * (llong)rt;
      ullong prdu = (ullong)(uint)rs * (ullong)(uint)rt;
#else
      llong prds = asm_mult(rs, rt);
      ullong prdu = asm_multu(rs, rt);
#endif
      print_hex(rs);
      print_hex(rt);
      print_hex((uint)(prds>>32));
      print_hex((uint)prds);
      print_hex((uint)(prdu>>32));
      print_hex((uint)prdu);
      if (rt != 0) {
#if 0
        int quos = rs / rt;
        int rems = rs % rt;
        uint quou = (uint)rs / (uint)rt;
        uint remu = (uint)rs % (uint)rt;
#else
        ullong quorems = asm_div(rs, rt);
        ullong quoremu = asm_divu(rs, rt);
        int quos = (int)quorems;
        int rems = (int)(quorems>>32);
        uint quou = (uint)quoremu;
        uint remu = (uint)(quoremu>>32);
#endif
        print_hex(quos);
        print_hex(rems);
        print_hex(quou);
        print_hex(remu);
      }
      myputc('\n');
    }
  }
  return 0;
}

