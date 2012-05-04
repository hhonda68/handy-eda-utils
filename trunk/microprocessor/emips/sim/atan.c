/*
 * EMIPS : Embedding MIPS Microprocessor
 *
 * Copyright (c) 2012 the handy-eda-utils developer(s).
 * Distributed under the MIT License.
 * (See accompanying file COPYING or copy at
 *  http://www.opensource.org/licenses/mit-license.php.)
 */

/* ftp://ftp.ics.es.osaka-u.ac.jp/mirrors/UnixArchive/PDP-11/Trees/V7/usr/src/libm/atan.c */

/****************************************************************/

/*
	floating-point arctangent

	atan returns the value of the arctangent of its
	argument in the range [-pi/2,pi/2].

	atan2 returns the arctangent of arg1/arg2
	in the range [-pi,pi].

	there are no error returns.

	coefficients are #5077 from Hart & Cheney. (19.56D)
*/

static const double sq2p1 =2.414213562373095048802e0;
static const double sq2m1 = .414213562373095048802e0;
static const double pio2  =1.570796326794896619231e0;
static const double pio4  = .785398163397448309615e0;
static const double p4    = .161536412982230228262e2;
static const double p3    = .26842548195503973794141e3;
static const double p2    = .11530293515404850115428136e4;
static const double p1    = .178040631643319697105464587e4;
static const double p0    = .89678597403663861959987488e3;
static const double q4    = .5895697050844462222791e2;
static const double q3    = .536265374031215315104235e3;
static const double q2    = .16667838148816337184521798e4;
static const double q1    = .207933497444540981287275926e4;
static const double q0    = .89678597403663861962481162e3;


/*
	xatan evaluates a series valid in the
	range [-0.414...,+0.414...].
*/

static double
xatan(arg)
double arg;
{
	double argsq;
	double value;

	argsq = arg*arg;
	value = ((((p4*argsq + p3)*argsq + p2)*argsq + p1)*argsq + p0);
	value = value/(((((argsq + q4)*argsq + q3)*argsq + q2)*argsq + q1)*argsq + q0);
	return(value*arg);
}

/*
	satan reduces its argument (known to be positive)
	to the range [0,0.414...] and calls xatan.
*/

static double
satan(arg)
double arg;
{
	if(arg < sq2m1)
		return(xatan(arg));
	else if(arg > sq2p1)
		return(pio2 - xatan(1.0/arg));
	else
		return(pio4 + xatan((arg-1.0)/(arg+1.0)));
}

/*
	atan makes its argument positive and
	calls the inner routine satan.
*/

double
atan(arg)
double arg;
{
	if(arg>0)
		return(satan(arg));
	else
		return(-satan(-arg));
}


/*
	atan2 discovers what quadrant the angle
	is in and calls atan.
*/

double
atan2(arg1,arg2)
double arg1,arg2;
{
	if((arg1+arg2)==arg1)
		if(arg1 >= 0.) return(pio2);
		else return(-pio2);
	else if(arg2 <0.)
		if(arg1 >= 0.)
			return(pio2+pio2 - satan(-arg1/arg2));
		else
			return(-pio2-pio2 + satan(arg1/arg2));
	else if(arg1>0)
		return(satan(arg1/arg2));
	else
		return(-satan(-arg1/arg2));
}

/****************************************************************/

#if 0
#include <stdio.h>
#define myputc putchar
#else
extern void myputc(char);
#endif

void print_double(double x)
{
  unsigned int xi;
  char buf[16];
  int i;
  if (x < 0) {
    myputc('-');
    x = -x;
  }
  xi = (unsigned int)x;
  x -= (double)xi;
  for (i=0; i<12; i++) {
    buf[i] = xi % 10;
    xi /= 10;
  }
  for (i--; i>=0; i--)  if (buf[i] != 0)  break;
  if (i < 0)  i = 0;
  for ( ; i>=0; i--)  myputc('0'+buf[i]);
  myputc('.');
  for (i=0; i<14; i++) {
    x *= 10.0;
    xi = (unsigned int)x;
    myputc('0'+xi);
    x -= (double)xi;
  }
  myputc('\n');
}

void print_hex(unsigned int x)
{
  int i;
  for (i=7; i>=0; i--)  myputc("0123456789abcdef"[(x>>(i*4))&15]);
  myputc('\n');
}

int main()
{
#if 1
  double atan_pi8 = atan(sq2m1);
  double atan_pi = atan_pi8 * 8.0;
  print_double(atan_pi);
#endif
#if 0
  print_double(pio2);
#endif
#if 0
  typedef union { float f; unsigned int ui; } fui_t;
  fui_t f1, f2;
  f1.f = 3.14f;
  f2.f = 2.0f;
  print_hex(f1.ui);	// 4048f5c3
  print_hex(f2.ui);	// 40000000
  f1.f *= f2.f;
  print_hex(f1.ui);	// 40c8f5c3
#endif
  return 0;
}

