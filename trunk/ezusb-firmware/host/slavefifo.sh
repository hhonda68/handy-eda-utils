#!/bin/sh
#
# Generic EZUSB firmware
#
# Copyright (c) 2008 the handy-eda-utils developer(s).
# Distributed under the MIT License.
# (See accompanying file COPYING or copy at
#  http://www.opensource.org/licenses/mit-license.php.)
#
# Last modified on May 2007.

. ezusb-common.sh

# IFCONFIG := 0b00000011 (External IFCLK, synchronous slave FIFO mode)
# EP2CFG := 0b10100010 (default)
# EP6CFG := 0b11100010 (default)
./ep1out $EZUSB $WREXT 01 e6 03 $WREXT 12 e6 a2 $WREXT 14 e6 e2 || failusb
# Reset FIFO and prime the pump
./ep1out $EZUSB $WREXT 04 e6 80 $WREXT 04 e6 02 $WREXT 04 e6 06 $WREXT 04 e6 00 \
                $WREXT 49 e6 82 $WREXT 49 e6 82 || failusb

# FLAGA = EP2 Empty, FLAGB = EP6 Full, FLAGC = EP2 Full, FLAGD = EP6 Empty
./ep1out $EZUSB $WREXT 02 e6 e8 $WREXT 03 e6 ac $WREXT 70 e6 80 || failusb

# EP2FIFOCFG := 16bit AUTOOUT
# EP6FIFOCFG := 16bit AUTOIN, inhibit-Zero-Length.
./ep1out $EZUSB $WREXT 18 e6 11 $WREXT 1a e6 09 || failusb

exit 0
