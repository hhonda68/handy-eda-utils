/*
 * Generic EZUSB firmware
 *
 * Copyright (c) 2008 the handy-eda-utils develeper(s).
 * Distributed under the MIT License.
 * (See accompanying file COPYING or copy at
 *  http://www.opensource.org/licenses/mit-license.php.)
 *
 * Last modified on May 2007.
 */

#define ALLOCATE_EXTERN
#include "fx2regs.h"

#define USE_ASM

#ifndef USE_ASM

#define NOP		do { _asm nop _endasm; } while (0)
#define	SYNCDELAY	do { NOP;NOP;NOP;NOP; } while (0)

xdata at 0x4AFC volatile BYTE CURRENTDPTR;
xdata at 0xDEAD volatile BYTE DUMMYDPTR;
xdata at 0xBEEF volatile BYTE INDIRECTPTR;

void main(void)
{
  MPA = 0xe6;
  AUTOPTRSETUP = 0x07;
  APTR1H = 0xe7;
  AUTOPTRH2 = 0xe7;
  while (1) {
    unsigned char cmdend;
    /* arm EP1OUT */
    EP1OUTBC = 0;
    /*SYNCDELAY;*/
    APTR1L = 0x80;
    AUTOPTRL2 = 0xc0;
    /* wait EP1OUT packet */
    while (EP01STAT & bmBIT1)  ;
    /* process commands */
    cmdend = 0x80 | EP1OUTBC;
    do {
      unsigned char cmd;
      cmd = XAUTODAT1;	/* use INDIRECTPTR */
      switch (cmd) {
      case 0x00: /* write internal memory */
	{
	  unsigned char adr, dat;
	  adr = INDIRECTPTR;
	  dat = INDIRECTPTR;
	  *((data volatile unsigned char *)adr) = dat;
	}
	break;
      case 0x01: /* write external memory */
	{
	  unsigned char adrl,adrh,dat;
	  adrl = INDIRECTPTR;
	  adrh = INDIRECTPTR;
	  dat = INDIRECTPTR;
	  DPL = adrl;
	  DPH = adrh;
	  CURRENTDPTR = dat;
	}
	break;
      case 0x02: /* write SFR */
	{
	  unsigned char adr, dat;
	  adr = INDIRECTPTR;
	  dat = INDIRECTPTR;
	  /*BEGIN-DUMMY*/
	  DUMMYDPTR = adr;	/* self-modification address */
	  ACC = dat;
	  /*END-DUMMY*/
	}
	break;
      case 0x03: /* write FPGACONFIG */
	{
	  unsigned char nl,nh,m,autoptr1bak;
	  nl = INDIRECTPTR;
	  nh = INDIRECTPTR;
	  if (nl != 0)  nh ++;
	  autoptr1bak = APTR1L;
	  m = 0;
	  APTR1L = 0x00; /* EP2FIFOBUF low */
	  do {
	    do {
	      APTR1H = 0xF0; /* EP2FIFOBUF high */
	      while (EP2468STAT & bmEP2EMPTY)  ;
	      do {
		SBUF1 = INDIRECTPTR;
		/*NOP-FOR-TIMING-ADJUSTMENT*/
		SBUF1 = INDIRECTPTR;
		/*NOP-FOR-TIMING-ADJUSTMENT*/
	      } while (--m);
	      OUTPKTEND = 0x82;  SYNCDELAY;
	    } while (--nl);
	  } while (--nh);
	  APTR1L = autoptr1bak;
	  APTR1H = 0xe7;
	}
	break;
      case 0x04: /* read internal memory */
	{
	  unsigned char adr, dat;
	  adr = INDIRECTPTR;
	  dat = *((data volatile unsigned char *)adr);
	  XAUTODAT2 = dat;
	}
	break;
      case 0x05: /* read external memory */
	{
	  unsigned char adrl,adrh,dat;
	  adrl = INDIRECTPTR;
	  adrh = INDIRECTPTR;
	  DPL = adrl;
	  DPH = adrh;
	  dat = CURRENTDPTR;
	  XAUTODAT2 = dat;
	}
	break;
      case 0x06: /* read SFR */
	{
	  unsigned char adr, dat;
	  adr = INDIRECTPTR;
	  /*BEGIN-DUMMY*/
	  DUMMYDPTR = adr;	/* self-modification address */
	  dat = ACC;
	  /*END-DUMMY*/
	  XAUTODAT2 = dat;
	}
	break;
      }
    } while (APTR1L != cmdend);
    cmdend = AUTOPTRL2 & 0x3F;
    if (cmdend != 00) {
      while (EP01STAT & bmBIT2)  ;
      EP1INBC = cmdend;
      /*SYNCDELAY;*/
    }
  }
}

#else

void main(void) __naked
{
_asm
	ar2 = 0x02
	ar3 = 0x03
	ar4 = 0x04
	ar5 = 0x05
	ar6 = 0x06
	ar7 = 0x07
	ar0 = 0x00
	ar1 = 0x01
;       firm.c    MPA = 0xe6;
;	firm.c 30 AUTOPTRSETUP = 0x07;
;	firm.c 31 APTR1H = 0xe7;
;	firm.c 32 AUTOPTRH2 = 0xe7;
	mov	_MPA,#>_XAUTODAT1
	mov	r1,#<_XAUTODAT1
	mov	_AUTOPTRSETUP,#0x07
	mov	_APTR1H,#>_EP1OUTBUF
	mov	_AUTOPTRH2,#>_EP1INBUF
;	firm.c 33 while (1) {
00159$:
;	firm.c 36 EP1OUTBC = 0;
	mov	r0,#<_EP1OUTBC
	clr	a
	movx	@r0,a
;	firm.c 42 APTR1L = 0x80;
	mov	r2,#0x80
	mov	_APTR1L,r2
;	firm.c 43 AUTOPTRL2 = 0xc0;
	mov	_AUTOPTRL2,#0xC0
;	firm.c 39 while (EP01STAT & bmBIT1)  ;
00113$:
	mov	a,_EP01STAT
	jb	acc.1,00113$
;	firm.c 41 cmdend = 0x80 | EP1OUTBC;
	movx	a,@r0
	orl	ar2,a
;	firm.c 44 do {
00138$:
;	firm.c 46 cmd = XAUTODAT1;
	movx	a,@r1
;	firm.c 47 switch (cmd) {
	rl	a
	mov	dptr,#00180$
	jmp	@a+dptr
00180$:
	sjmp	00116$
	sjmp	00117$
	sjmp	00118$
	sjmp	00119$
	sjmp	00134$
	sjmp	00135$
	sjmp	00136$
	.byte	0x51,0x00	/* acall 0x200 (user extension command at address 0x0200) */
	sjmp	00139$
;	firm.c 48 case 0x00 /* write internal memory */
00116$:
;	firm.c 51 adr = INDIRECTPTR;
	movx	a,@r1
	mov	r0,a
;	firm.c 52 dat = INDIRECTPTR;
	movx	a,@r1
;	firm.c 53 *((data volatile unsigned char *)adr) = dat;
	mov	@r0,a
;	firm.c 55 break;
	sjmp	00139$
;	firm.c 56 case 0x01 /* write external memory */
00117$:
;	firm.c 59 adrl = INDIRECTPTR;
	movx	a,@r1
;	firm.c 62 DPL = adrl;
	mov	_DPL,a
;	firm.c 60 adrh = INDIRECTPTR;
	movx	a,@r1
;	firm.c 63 DPH = adrh;
	mov	_DPH,a
;	firm.c 61 dat = INDIRECTPTR;
	movx	a,@r1
;	firm.c 64 CURRENTDPTR = dat;
	movx	@dptr,a
;	firm.c 66 break;
	sjmp	00139$
;	firm.c 67 case 0x02 /* write SFR */
00118$:
;	firm.c 70 adr = INDIRECTPTR;
	movx	a,@r1
	mov	r3,a
;	firm.c 71 dat = INDIRECTPTR;
	movx	a,@r1
	xch	a,r3
;	firm.c 73 DUMMYDPTR = adr;	/* self-modification address */
	mov	dptr,#00198$+1
	movx	@dptr,a
;	firm.c 74 ACC = dat;
00198$:
	mov	_ACC,r3
;	firm.c 77 break;
	sjmp	00139$

;	firm.c 116 case 0x04 /* read internal memory */
00134$:
;	firm.c 119 adr = INDIRECTPTR;
	movx	a,@r1
	mov	r0,a
;	firm.c 120 dat = *((data volatile unsigned char *)adr);
	mov	a,@r0
;	goto write_EP1IN_and_break;
;	sjmp	00140$	/*FALLTHROUGH*/

;;;;;;;;;;;;;;;; Insert loop-epilogue here to use short jump ;;;;;;;;;;;;;;;;

00140$:
;	firm.c 122 XAUTODAT2 = dat;
	mov	r0,#<_XAUTODAT2
	movx	@r0,a
00139$:
;	firm.c 148 } while (APTR1L != cmdend);
	mov	a,r2
	cjne	a,_APTR1L,00138$
;	firm.c 149 cmdend = AUTOPTRL2 & 0x3F;
	mov	a,#0x3F
	anl	a,_AUTOPTRL2
;	firm.c 150 if (cmdend != 00) {
	jz	00159$
;	firm.c 151 while (EP01STAT & bmBIT2)  ;
00141$:
	mov	0x20,_EP01STAT
	jb	2,00141$
;	firm.c 152 EP1INBC = cmdend;
	mov	r0,#<_EP1INBC
	movx	@r0,a
	sjmp	00159$

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;	firm.c 125 case 0x05 /* read external memory */
00135$:
;	firm.c 128 adrl = INDIRECTPTR;
	movx	a,@r1
;	firm.c 130 DPL = adrl;
	mov	_DPL,a
;	firm.c 129 adrh = INDIRECTPTR;
	movx	a,@r1
;	firm.c 131 DPH = adrh;
	mov	_DPH,a
;	firm.c 132 dat = CURRENTDPTR;
	movx	a,@dptr
;	goto write_EP1IN_and_break;
	sjmp	00140$
;	firm.c 136 case 0x06 /* read SFR */
00136$:
;	firm.c 139 adr = INDIRECTPTR;
	movx	a,@r1
;	firm.c 141 DUMMYDPTR = adr;	/* self-modification address */
	mov	dptr,#00199$+1
	movx	@dptr,a
;	firm.c 142 dat = ACC;
00199$:
	mov	a,_ACC
;	goto write_EP1IN_and_break;
	sjmp	00140$

;	firm.c 78 case 0x03 /* write FPGACONFIG */
00119$:
;	firm.c 81 nl = INDIRECTPTR;
	movx	a,@r1
	mov	r3,a
;	firm.c 82 nh = INDIRECTPTR;
	movx	a,@r1
	mov	r4,a
;	firm.c 83 if (nl != 0)  nh ++;
	mov	a,r3
	jz	00160$
	inc	r4
00160$:
;	firm.c 84 autoptr1bak = APTR1L
;	firm.c 89 APTR1L = 0x00; /* EP2FIFOBUF low */
	clr	a
	mov	r6,a
	xch	a,_APTR1L
	mov	r5,a
	mov	r7,#>_EP2FIFOBUF
	mov	r0,#<_OUTPKTEND
;	firm.c 86 do {
00162$:
;	firm.c 90 APTR1H = 0xF0; /* EP2FIFOBUF high */
	mov	_APTR1H,r7
;	firm.c 88 while (EP2468STAT & bmEP2EMPTY)  ;
00161$:
	mov	a,_EP2468STAT
	jb	acc.0,00161$
;	firm.c 91 do {
00163$:
	movx	a,@r1
	mov	_SBUF1,a
	mul	ab
	nop
	movx	a,@r1
	mov	_SBUF1,a
	mov	a,#0x82
	nop
;	firm.c 108 } while (--m);
	djnz	r6,00163$
;	firm.c 109 OUTPKTEND = 0x82;  SYNCDELAY;
	movx	@r0,a
;	firm.c 110 } while (--nl);
	djnz	r3,00162$
;	firm.c 111 } while (--nh);
	djnz	r4,00162$
;	firm.c 112 APTR1L = autoptr1bak;
	mov	_APTR1L,r5
;	firm.c 113 APTR1H = 0xe7;
	mov	_APTR1H,#>_EP1OUTBUF
;	firm.c 115 break;
	sjmp	00139$

_endasm;
}

#endif

