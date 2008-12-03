#!/usr/bin/ruby -w
#
# FDFL (Fixed-point dataflow language) utilities
#
# Copyright (c) 2008 the handy-eda-utils developer(s).
# Distributed under the MIT License.
# (See accompanying file COPYING or copy at
#  http://www.opensource.org/licenses/mit-license.php.)
#
# Last modified on February 2008.
#
################################################################
#
# ruby moduledepend.rb --fdfl           generated-from-fdfl.v > generated-from-fdfl.dep
# ruby moduledepend.rb --verilog prefix handwritten.v > handwritten.dep
# ruby moduledepend.rb --flatten        foo.dep bar.dep > all-flattened.dep
# ruby moduledepend.rb --genmid srcdir dstdir all-flattend.dep

require 'tsort'

ARGV.size >= 1  or fail
mode = ARGV.shift

class DependencyGraph < Hash
include TSort
def initialize
  super
end
alias tsort_each_node each_key
def tsort_each_child(node, &block)
  fetch(node).each(&block)
end
end

case mode
when "--fdfl"
  modulename = nil
  dep = []
  while (gets) do
    case $_
    when /^module (\w+)/
      modulename = $1
      dep.clear
    when /^  (\w+) (#\S+ )?\w+\(/   # no need to parse corresponding \)
      submod = $1
      dep.push(submod)  unless (submod =~ /^SC22/)
    when /^endmodule/
      print(modulename, ": ", dep.uniq.join(" "), "\n")
    end
  end
when "--verilog"
  ARGV.size >= 1  or fail
  prefix = ARGV.shift
  modulename = nil
  dep = []
  while (gets) do
    case $_
    when /^module\s+(\w+)/
      modulename = $1
      dep.clear
    when /^\s*(#{prefix}\w+)/o
      dep.push($1)
    when /^endmodule/
      print(modulename, ": ", dep.uniq.join(" "), "\n")
    end
  end
when "--flatten"
  childtab = DependencyGraph.new
  while (gets) do
    chomp!
    f = split
    modulename = f.shift
    modulename.sub!(/:$/,"")  or fail
    (!childtab.include?(modulename))  or fail "#{modulename}: duplicated definition"
    childtab[modulename] = f
  end
  deps = {}
  childtab.tsort_each do |modulename|
    descendant = childtab[modulename].map{ |submod|
                   (deps.include?(submod) ? deps[submod] : []) + [submod]
                 }.flatten.uniq
    print(modulename, ": ", descendant.join(" "), "\n")
    deps[modulename] = descendant
  end
when "--genmid"
  ARGV.size >= 2  or fail
  srcdir = ARGV.shift
  dstdir = ARGV.shift
  FileTest.directory?(srcdir)  or fail
  FileTest.directory?(dstdir)  or fail
  while (gets) do
    chomp!
    f = split
    f[0].sub!(/:$/,"")  or fail
    tgt = "#{dstdir}/#{f[0]}.v"
    srcs = f.map{|name| "#{srcdir}/#{name}.v"}
    uptodate = FileTest.file?(tgt)
    if (uptodate) then
      tgttime = File.mtime(tgt)
      srcs.each do |src|
        FileTest.file?(src)  or fail
        if (File.mtime(src) > tgttime) then
          uptodate = false
          break
        end
      end
    end
    unless (uptodate) then
      print("[Generating #{tgt} ...]\n")
      trytgt = tgt + "~"
      open(trytgt, "w") do |dfp|
        (f[1..-1]+[f[0]]).each do |name|
          dfp.print("`ifndef #{name.upcase}_DEFINED\n",
                    "`define #{name.upcase}_DEFINED\n",
                    File.read("#{srcdir}/#{name}.v"),
                    "`endif\n",
                    "\n")
        end
      end
      File.rename(trytgt, tgt)
    end
  end
else
  fail
end
