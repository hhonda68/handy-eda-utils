#
# Generic EZUSB firmware
#
# Copyright (c) 2008 the handy-eda-utils developer(s).
# Distributed under the MIT License.
# (See accompanying file COPYING or copy at
#  http://www.opensource.org/licenses/mit-license.php.)
#
# Last modified on May 2007.

.SUFFIXES:

# sdcc 2.6.0
CC := ../tool/instdir/bin/sdcc -mmcs51

all:	firm.ihx
	@

%.ihx: %.c
	$(CC) $^
