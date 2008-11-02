#!/usr/bin/ruby -w
#
# IDFL (Integer dataflow language) to FDFL (Fixed-point dataflow language) converter
#
# Copyright (c) 2008 the handy-eda-utils develeper(s).
# Distributed under the MIT License.
# (See accompanying file COPYING or copy at
#  http://www.opensource.org/licenses/mit-license.php.)
#
# Last modified on December 2007.
#
################################################################
#
# ruby dfi2df.rb foo.dfi > foo.df
#
# foo.dfi:
#   module sel2to1
#   i    i_sel
#   i 8  i_indata[0-1]
#   o 8  o_outdata
#   endmodule
#
# foo.df:
#   module sel2to1
#   i b     i_sel
#   i u 8 0 i_indata[0-1]
#   o u 8 0 o_outdata
#   endmodule
#

while (gets) do
  sub!(/^([iwroq])\s+(\S+)/){
    c, d = $1, $2
    (d =~ /^[0-9@]/) ? "#{c} u #{d} 0" : "#{c} b #{d}"
  }  
  print
end
