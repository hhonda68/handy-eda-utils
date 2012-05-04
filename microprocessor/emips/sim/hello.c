/*
 * EMIPS : Embedding MIPS Microprocessor
 *
 * Copyright (c) 2012 the handy-eda-utils developer(s).
 * Distributed under the MIT License.
 * (See accompanying file COPYING or copy at
 *  http://www.opensource.org/licenses/mit-license.php.)
 */

extern void myputc(char);

static void myputs(const char *s)
{
  while (*s != '\0')  myputc(*s++);
}

int main()
{
#if 1
  myputs("Hello world.\n");
#endif
#if 0
  myputc('H');
  myputc('e');
  myputc('l');
  myputc('l');
  myputc('o');
  myputc(' ');
  myputc('w');
  myputc('o');
  myputc('r');
  myputc('l');
  myputc('d');
  myputc('.');
  myputc('\n');
#endif
  return 0;
}
