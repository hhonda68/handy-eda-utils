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
# Usage:
#   . ezfind.sh
#
#   finds the EZUSB device and sets the environment variable $EZUSB
#

export EZUSB
if EZUSB=`ruby busdev.rb -f 2>/dev/null` ; then
  echo "EZUSB = $EZUSB"
else
  echo "ezfind.sh: Error: cannot find an EZUSB device." 1>&2
  false
fi
