#!/usr/bin/ruby -w
#
# TSV (Type-Saving Verilog) to Verilog converter
#
# Copyright (c) 2008 the handy-eda-utils developer(s).
# Distributed under the MIT License.
# (See accompanying file COPYING or copy at
#  http://www.opensource.org/licenses/mit-license.php.)
#

#
# TSV code example:
#   module add8
#   i  8 a b
#   o 16 y = a + b
#   endmodule
#
# Generated Verilog code:
#   module add8(a,b,y);
#   input [7:0] a, b;
#   output [15:0] y;
#   assign y = a + b;
#   endmodule
#
# Lines to be converted
#   i [WIDTH] NAME0 NAME1 ...
#   o [WIDTH] NAME0 NAME1 ...
#   w [WIDTH] NAME0 NAME1 ...
#   o [WIDTH] NAME = SUBMOD ( ARG0 ARG1 ... )
#   o [WIDTH] NAME = { PART0 PART1 ... }
#   o [WIDTH] NAME = OTHER_VERBATIM_EXPR
#   w [WIDTH] NAME = SUBMOD ( ARG0 ARG1 ... )
#   w [WIDTH] NAME = { PART0 PART1 ... }
#   w [WIDTH] NAME = OTHER_VERBATIM_EXPR
#   a TARGET = SUBMOD ( ARG0 ARG1 ... )
#   a TARGET = { PART0 PART1 ... }
#   a TARGET = OTHER_VERBATIM_EXPR
#   a { PART0 PART1 ... } = SUBMOD ( ARG0 ARG1 ... )
#   a { PART0 PART1 ... } = { PART10 PART11 ... }
#   a { PART0 PART1 ... } = OTHER_VERBATIM_EXPR
#   m SUBMOD ARG0 ARG1 ...
#
#       SUBMOD ... (\w+)(#\(.+\))?(:\w+)?
#          $1 : submodule name
#          $2 : parameters (optional)
#          $3 : instance name (optional)
#
# Preprocess before conversion
#   1. If a line matches /^#/ or /^\s+#\s/, delete the whole line.
#   2. If the line does not match /^[a-z]\s/, print the line as-is.
#   3. If the line matches /\s+#.*$/, delete $&.
#   4. If the line matches /\s*\\$/,
#        replace $& with " " and append the next line and goto step 3.
#   5. Expand "ranged name field"s (matching /^\w+(\[(\d+([-,]\d+)+)\]\w*)+$/).
#        foo[0-3]      --> foo0 foo1 foo2 foo3
#        foo[3-0]bar   --> foo3bar foo2bar foo1bar foo0bar
#        a[0-15]       --> a0 a1 a2 ... a13 a14 a15
#        a[00-15]      --> a00 a01 a02 ... a13 a14 a15
#        a[0,2,4-6]    --> a0 a2 a4 a5 a6
#        a[0-2][5-7]   --> a05 a06 a07 a15 a16 a17 a25 a26 a27
#        a[0-2]b[5-7]c --> a0b5c a0b6c a0b7c a1b5c a1b6c a1b7c a2b5c a2b6c a2b7c
#

MaxLineLen = 70

def abbrev_instance_name(modulename)
  # foo_bar_baz --> baz
  # FooBarBaz   --> baz
  modulename.sub(/^.*_([a-zA-Z])/){$1}.sub(/^.*[a-z\d]([A-Z])/){$1}.downcase \
    .gsub(/_+/,"_").gsub(/([a-z])_/){$1}.sub(/(.)_$/){$1}
end

################################################################

def prettyprint(indent, headstr, commalist, tailstr)
  ans = []
  indentstr = " "*indent
  if (indent + headstr.length + commalist.length + tailstr.length <= MaxLineLen) then
    ans.push([indentstr, headstr, commalist, tailstr].to_s)
  else
    ans.push([indentstr, headstr].to_s)
    indent2 = indent + 2
    indentstr2 = " "*indent2
    while (indent2+commalist.length > MaxLineLen) do
      ix = commalist.rindex(?,, MaxLineLen-indent2-1)
      ans.push([indentstr2, commalist[0..ix]].to_s)
      commalist = commalist[(ix+1)..-1]
    end
    ans.push([indentstr2, commalist].to_s)
    ans.push([indentstr, tailstr].to_s)
  end
  ans
end

def wrap_line(line)
  ans = []
  while (line.size > MaxLineLen) do
    ix = line.rindex(?,, MaxLineLen-1)
    ix  or fail "cannot wrap line at comma"
    ans.push(line[0..ix])
    line = "  " + line[ix+1..-1]
  end
  ans.push(line)
  ans
end

def println(*args)
  args.each do |data|
    case data
    when Array
      data.each do |elem|
        println(elem)
      end
    when String
      print(data, "\n")
    else
      fail
    end
  end
end

ModuleInfo = Struct.new(:name, :args, :arglines, :bodylines, :instseq)
ModInfo = ModuleInfo.new

ModInfo.name = nil
ModInfo.args = []
ModInfo.arglines = []
ModInfo.bodylines = []
ModInfo.instseq = {}

def print_module
  println(prettyprint(0, "module #{ModInfo.name}(", ModInfo.args.join(","), ");"))
  println(ModInfo.arglines, ModInfo.bodylines, "endmodule")
  ModInfo.name = nil
  ModInfo.args.clear
  ModInfo.arglines.clear
  ModInfo.bodylines.clear
  ModInfo.instseq.clear
end

def instance_name(submod)
  name = abbrev_instance_name(submod)
  seqnum = (ModInfo.instseq[name] ||= 0)
  instname = name + ((name =~ /\d$/) ? "_" : "") + seqnum.to_s
  ModInfo.instseq[name] = seqnum + 1
  instname
end

def append_instance_name(submod)
  ans = submod.sub(/:(\w+)$/){" "+$1}
  if (ans == submod) then
    submod =~ /^\w+/
    ans = submod + " " + instance_name($&)
  end
  ans
end

class Integer
  def to_s_width(width)
    width ? ("%0*d" % [width,self]) : self.to_s
  end
end

def expand_ranges(line)
  arr = line.split
  i = 0
  while (i < arr.size) do
    str = arr[i]
    if (str =~ /^\w+(\[(\d+([-,]\d+)+)\]\w*)+$/) then
      str =~ /^(\w+)\[(\d+([-,]\d+)+)\](.*)$/
      pre, body, post = $1, $2, $4
      nums = []
      body.split(",").each do |rng|
        rng =~ /^(\d+)(-(\d+))?$/  or fail "range syntax error"
        if ($2) then
          from, to = $1.to_i, $3.to_i
          width = ($1.length == $3.length) ? $1.length : nil
          if (from < to) then
            nums += (from..to).map{|n| n.to_s_width(width)}
          else
            nums += (to..from).map{|n| n.to_s_width(width)}.reverse
          end
        else
          nums.push($1)
        end
      end
      arr[i,1] = nums.map{|s| pre + s + post}
    else
      i += 1
    end
  end
  arr
end

TypeStr = { "i" => "input", "o" => "output", "w" => "wire", "a" => "assign" }

def convert(line)
  line.sub!(/\s+#.*$/,"")
  while (line.sub!(/\s*\\$/," ")) do
    gets  or fail "unexpected EOF within line-continuation"
    chomp!
    line += $_
    line.sub!(/\s+#.*$/,"")
  end
  arr = expand_ranges(line)
  case arr[0]
  when "i","o","w","a"
    type = arr.shift
    if (arr[0] =~ /^\d+$/) then
      (type != "a")  or fail "syntax error"
      width = " [#{arr[0].to_i-1}:0] "
      arr.shift
    else
      width = " "
    end
    if (arr[0] == "{") then
      ix = (0...arr.size).find{|i| arr[i] == "}"}
      (ix && (ix >= 2)) or fail "syntax error"
      arr[0..ix] = "{" + arr[1..ix-1].join(",") + "}"
    end
    if (arr.size >= 2 && arr[1] == "=") then
      (type != "i")  or fail "syntax error"
      sig = arr[0]
      line = TypeStr[type] + width + sig
      if (type == "o") then
        ModInfo.args.push(sig)
        ModInfo.arglines.push(line+";")
        type = "a"
        width = " "
        line = TypeStr[type] + width + sig
      end
      if (arr.size >= 5 && arr[2] == "{" && arr[-1] == "}") then
        ModInfo.bodylines.push(prettyprint(0, line+" = {", arr[3..-2].join(","), "};"))
      elsif (arr.size >= 5 && arr[3] == "(" && arr[-1] == ")") then
        args = arr[4..-2]
        args.push(sig)
        # o   foo = submod ( arg0 arg1 ... )
        # o 8 foo = submod ( arg0 arg1 ... )
        submod = append_instance_name(arr[2])
        if (type == "w") then
          ModInfo.bodylines.push(line+";")
        end
        ModInfo.bodylines.push(prettyprint(0, "#{submod}(", args.join(","), ");"))
      else
        # o   foo = EXPR
        # o 8 foo = EXPR
        ModInfo.bodylines.push(line + " " + arr[1..-1].join(" ") + ";")
      end
    else
      # o   foo bar ...
      # o 8 foo bar ...
      (type != "a")  or fail "syntax error"
      line = TypeStr[type] + width + arr.join(",") + ";"
      if (type == "i" || type == "o") then
        ModInfo.args.push(arr)
        ModInfo.arglines.push(wrap_line(line))
      else
        ModInfo.bodylines.push(wrap_line(line))
      end
    end
  when "m"
    submod = append_instance_name(arr[1])
    args = arr[2..-1]
    ModInfo.bodylines.push(prettyprint(0, "#{submod}(", args.join(","), ");"))
  else
    fail "syntax error"
  end
end

begin
  while (gets) do
    next if ($_ =~ /^#/)
    next if ($_ =~ /^\s+#\s/)
    chomp!
    line = $_
    case $_
    when /^module (\w+)$/
      ModInfo.name.nil?  or fail "module declaration within another module definition"
      ModInfo.name = $1
    when /^[a-z]\s/
      convert(line)
    when /^endmodule\s*$/
      if (ModInfo.name) then
        print_module
      else
        print(line,"\n")
      end
    else
      if (ModInfo.name) then
        ModInfo.bodylines.push(line)
      else
        print(line,"\n")
      end
    end
  end
rescue RuntimeError
  if (ARGF.closed?) then
    STDERR.print("Error: #{$!}.\n")
  else
    STDERR.print "#{ARGF.filename}:#{ARGF.file.lineno}: #{$!}.\n"
  end
  exit(2)
end