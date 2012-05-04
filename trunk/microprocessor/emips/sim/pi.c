/*
 * EMIPS : Embedding MIPS Microprocessor
 *
 * Copyright (c) 2012 the handy-eda-utils developer(s).
 * Distributed under the MIT License.
 * (See accompanying file COPYING or copy at
 *  http://www.opensource.org/licenses/mit-license.php.)
 */

/* From opencores.org plasma/trunk/tools/pi.c */

extern void myputc(char);

void print_num(unsigned long num)
{
   unsigned long digit,offset;
   for(offset=1000;offset;offset/=10) {
      digit=num/offset;
      myputc(digit+'0');
      num-=digit*offset;
   }
}
 
int main()
{
   long a=10000,b=0,c=56,d=0,e=0,f[57],g=0;
   long a5=a/5;
   for(;b<=c;) f[b++]=a5;
   for(;d=0,g=c*2;c-=14,print_num(e+d/a),e=d%a)for(b=c;d+=f[b]*a,
     f[b]=d%--g,d/=g--,--b;d*=b);
   myputc('\n');
   return 0;
}

