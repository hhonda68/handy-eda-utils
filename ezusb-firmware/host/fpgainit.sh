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
#
################################################################
#
# ./fpgainit.sh [configuration_rbf_file]

. ezusb-common.sh

configfile=${1:-../fpga/rtl/fpga_top.rbf}

sz=`ruby -e 'print FileTest.size(ARGV[0])' "$configfile"` || fail "cannot read configfile"

npak=$(((sz+511)/512))

test $npak -ne 0 || fail "empty configfile"
test $npak -lt 65535 || fail "too big configfile (more than 65535 USB packets)"

npakstr=`printf "%02x %02x" $(((npak-1)&255)) $(((npak-1)>>8))`

# Reset I/O ports
./ep1out $EZUSB $WREXT 70 e6 00 $WREXT 71 e6 00 $WREXT 72 e6 00 \
  $WRSFR $OEA 00 $WRSFR $OEB 00 $WRSFR $OEC 00 $WRSFR $OED 00 $WRSFR $OEE 00 \
  $WRSFR $IOA 00 $WRSFR $IOB 00 $WRSFR $IOC 00 $WRSFR $IOD 00 $WRSFR $IOE 00 || failusb

# bit     name
# -----------------
#  7  I  CONF_DONE
#  6  I  ASDO (unused)
#  5  I  INIT_DONE
#  4  I  DATA0 (unused)
#  3  O  nCE
#  2  I  nCS (unused)
#  1  O  nCONFIG
#  0  I  DCLK (unused)

# Assert nCONFIG and check CONF_DONE low
./ep1out $EZUSB $WRSFR $OEC 0a $RDSFR $IOC || failusb
d=`./ep1in $EZUSB` || failusb

test "$d" == "75" -o "$d" == "65" || \
  fail "curious FPGA status when nCONFIG=0: expect=75 or 65, actual=$d"

# Confirm that PE4 is not tied to GND.
./ep1out $EZUSB $RDSFR $IOE || failusb
d=`./ep1in $EZUSB` || failusb
test "$((0x$d&0x10))" -ne 0 || fail "curious DIPSW status (DIPSW4 == ON?)"

# Check PE4 and PC4 are connected
./ep1out $EZUSB $WRSFR $OEE 10 $RDSFR $IOC || failusb
d=`./ep1in $EZUSB` || failusb
test "$d" == "65" || fail "PC4 and PE4 are not connected.  Check DIPSW setting"

./ep1out $EZUSB $WRSFR $OEE 00 $RDSFR $IOC || failusb
d=`./ep1in $EZUSB` || failusb
test "$d" == "75" || fail "PC4 and PE4 are not connected.  Check DIPSW setting"

# Setup serial interface
./ep1out $EZUSB $WREXT 72 e6 10 $WRSFR c0 20 || failusb

# Negate nCONFIG
./ep1out $EZUSB $WRSFR $IOC 02 $RDSFR $IOC || failusb
d=`./ep1in $EZUSB` || failusb
test "$d" == "77" || fail "curious FPGA status when nCONFIG=1: expect=77, actual=$d"

# re-arm EP2 and transfer
./ep1out $EZUSB $WREXT 49 e6 82 $WREXT 49 e6 82 \
         $WRCFG $npakstr $RDSFR $IOC $WRCFG 01 00 || failusb
./ep2out --autopad $EZUSB $configfile || failusb
d1=`./ep1in $EZUSB` || failusb
./ep1out $EZUSB $RDSFR $IOC || failusb
d2=`./ep1in $EZUSB` || failusb

d=`printf "%02x %02x" $((0x$d1|0x10)) $((0x$d2|0x10))`

# Check INIT_DONE
test "$d" == "57 f7" || \
  fail "curious FPGA status after config: expect={57 f7}, actual={$d}"

# Finalize serial interface
./ep1out $EZUSB $WREXT 72 e6 00 $WRSFR c0 00 || failusb

exit 0
