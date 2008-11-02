#!/bin/sh
#
# Generic EZUSB firmware
#
# Copyright (c) 2008 the handy-eda-utils develeper(s).
# Distributed under the MIT License.
# (See accompanying file COPYING or copy at
#  http://www.opensource.org/licenses/mit-license.php.)
#
# Last modified on May 2007.

. ezusb-common.sh

./loadfirm $EZUSB ../ezusb/code/firm.ihx || fail "cannot load EZUSB firmware"

function failinit() {
  fail "cannot initialize EZUSB device registers"
}

# CPUCS := 0x10 (CPU clock 48MHz), CKCON := 0x00 (0-wait xdata access)
./ep1out $EZUSB $WREXT 00 e6 10 $WRSFR 8e 00 || failinit

# Reset IFCONFIG, EPxCFG, EPxFIFOCFG, PINFLAGSxx, FIFO
./ep1out $EZUSB $WREXT 01 e6 80 $WREXT 02 e6 00 $WREXT 03 e6 00 \
                $WREXT 12 e6 a2 $WREXT 14 e6 e2 $WREXT 18 e6 05 $WREXT 1a e6 05 || failinit

./ep1out $EZUSB $WREXT 04 e6 80 $WREXT 04 e6 02 $WREXT 04 e6 04 \
                $WREXT 04 e6 06 $WREXT 04 e6 08 $WREXT 04 e6 00 || failinit

# Reset I/O port alternate configurations
./ep1out $EZUSB $WREXT 70 e6 00 $WREXT 71 e6 00 $WREXT 72 e6 00 \
                $WRSFR b2 00 $WRSFR b3 00 $WRSFR b4 00 $WRSFR b5 00 $WRSFR b6 00 \
                $WRSFR 80 00 $WRSFR 90 00 $WRSFR a0 00 $WRSFR b0 00 $WRSFR b1 00 || failinit

# Set "Chip Revision Control" register as Cypress recommends.
./ep1out $EZUSB $WREXT 0b e6 03 || failinit

exit 0

