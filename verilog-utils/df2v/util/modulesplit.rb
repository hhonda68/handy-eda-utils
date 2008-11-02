#!/usr/bin/ruby -w
#
# FDFL (Fixed-point dataflow language) utilities
#
# Copyright (c) 2008 the handy-eda-utils develeper(s).
# Distributed under the MIT License.
# (See accompanying file COPYING or copy at
#  http://www.opensource.org/licenses/mit-license.php.)
#
# Last modified on December 2007
#
################################################################
#
# ruby modulesplit.rb foo.v subdir
#

(ARGV.size == 2)  or fail
srcfile, subdir = ARGV

FileTest.file?(srcfile) && FileTest.directory?(subdir)  or fail

def save_if_changed(filename, newcontents)
  if (FileTest.file?(filename)) then
    do_write = (File.readlines(filename) != newcontents)
  else
    do_write = true
  end
  if (do_write) then
    open(filename, "w") do |f|
      f.print(newcontents)
    end
  end
end

open(srcfile) do |f|
  modulename = nil
  arr = []
  while ($_ = f.gets) do
    arr.push($_)
    case $_
    when /^module (\w+)/
      modulename = $1
      arr.clear
      arr.push($_)
    when /^endmodule/
      save_if_changed("#{subdir}/#{modulename}.v", arr)
    end
  end
end
