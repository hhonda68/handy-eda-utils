#
# Generic EZUSB firmware
#
# Copyright (c) 2008 the handy-eda-utils develeper(s).
# Distributed under the MIT License.
# (See accompanying file COPYING or copy at
#  http://www.opensource.org/licenses/mit-license.php.)
#
# Last modified on May 2007.

OEA=b2
OEB=b3
OEC=b4
OED=b5
OEE=b6
IOA=80
IOB=90
IOC=a0
IOD=b0
IOE=b1

WRINT=00
WREXT=01
WRSFR=02
WRCFG=03
RDINT=04
RDEXT=05
RDSFR=06

function fail() {
  echo $0: Error: "$*". 1>&2
  exit 2
}

function failusb {
  fail "cannot access the EZUSB device"
}

test "$EZUSB" || fail "EZUSB environment variable is not set"
test -f "$EZUSB" || fail "invalid EZUSB environment variable"

