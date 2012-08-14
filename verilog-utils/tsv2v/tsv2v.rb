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
#   o  9 y = a + b
#   endmodule
#
# Generated Verilog code:
#   module add8(a,b,y);
#   input [7:0] a, b;
#   output [8:0] y;
#   assign y = a + b;
#   endmodule
#
# Lines to be converted
#   p [WIDTH] NAME0 DEFAULT0 NAME1 DEFAULT1 ...
#   l [WIDTH] NAME0 VALUE0 NAME1 VALUE1 ...
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
#       SUBMOD ... (\$)?(\w+)(#\(.+\))?(:\w+)?(\[\S+:\S+\])?
#          $1 : prefixed-submodule flag (optional)
#          $2 : submodule name
#          $3 : parameters (optional)
#          $4 : instance name (optional)
#          $5 : range for multiple instantiation (optional)
#
# Preprocess before conversion
#   1. If a line matches /^#/ or /^\s+#\s/, delete the whole line.
#   2. If the line does not match /^[a-z]\s/, print the line as-is.
#   3. If the line matches /\s+#.*$/, delete $&.
#   4. If the line matches /\s*\\$/,
#        replace $& with " " and append the next line and goto step 3.
#   5. Expand "ranged name field"s (matching /^\w+(\[(\d+([-,]\d+)+)\]\w*)+@?$/).
#        foo[0-3]      --> foo0 foo1 foo2 foo3
#        foo[3-0]bar   --> foo3bar foo2bar foo1bar foo0bar
#        a[0-15]       --> a0 a1 a2 ... a13 a14 a15
#        a[00-15]      --> a00 a01 a02 ... a13 a14 a15
#        a[0,2,4-6]    --> a0 a2 a4 a5 a6
#        a[0-2][5-7]   --> a05 a06 a07 a15 a16 a17 a25 a26 a27
#        a[0-2]b[5-7]c --> a0b5c a0b6c a0b7c a1b5c a1b6c a1b7c a2b5c a2b6c a2b7c
#   6. Expand "handshake channel field"s (matching /^\w+@$/)
#        i 8 foo@ --> i   foo_vld
#                     o   foo_rdy
#                     i 8 foo
#        o 8 foo@ --> o   foo_vld
#                     i   foo_rdy
#                     o 8 foo
#        w 8 foo@ --> w   foo_vld foo_rdy
#                     w 8 foo
#        m submod foo@ --> m submod foo_vld foo_rdy foo
#   7. Expand "common handshaking idiom" lines
#        @ dstchan = srcchan
#           --> a dstchan_vld = srcchan_vld
#               a srcchan_rdy = dstchan_rdy
#        @ dstchan = srcchan0 srcchan1 srcchan2 ... srcchanN
#           --> a dstchan_vld  = srcchan0_vld & srcchan1_vld & srcchan2_vld & ... & srcchanN_vld & 1'b1
#               a srcchan0_rdy = 1'b1         & srcchan1_vld & srcchan2_vld & ... & srcchanN_vld & dstchan_rdy
#               a srcchan1_rdy = srcchan0_vld & 1'b1         & srcchan2_vld & ... & srcchanN_vld & dstchan_rdy
#               a srcchan2_rdy = srcchan0_vld & srcchan1_vld & 1'b1         & ... & srcchanN_vld & dstchan_rdy
#               ...(ditto)...
#               a srcchanN_rdy = srcchan0_vld & srcchan1_vld & srcchan2_vld & ... & 1'b1         & dstchan_rdy
#        @ dstchan0 dstchan1 ... dstchanN = srcchan
#           --> m FORKHSK#(N+1) clk rst srcchan_vld srcchan_rdy \
#                   {dstchan0_vld,dstchan1_vld,...,dstchanN_vld} {dstchan0_rdy,dstchan1_rdy,...,dstchanN_rdy}
#        @ [WIDTH] NUM allchan0 [allchan1 ...]
#           --> w NUM       allchan0_vld allchan0_rdy [allchan1_vld allchan1_rdy ...]
#               w WIDTH*NUM allchan0 [allchan1 ...]
#        @ allchan = { srcchan0 srcchan1 ... srcchanN } 
#           --> w N+1     allchan_vld allchan_rdy
#               w W*(N+1) allchan
#               a         allchan_vld = { srcchan0_vld srcchan1_vld ... srcchanN_vld }
#               a         { srcchan0_rdy srcchan1_rdy ... srcchanN_rdy } = allchan_rdy
#               a         allchan = { srcchan0 srcchan1 ... srcchanN }
#               (Note: all source channels must have the same width)
#        @ { dstchan0 dstchan1 ... dstchanN } = allchan
#           --> w N+1     allchan_vld allchan_rdy
#               w W*(N+1) allchan
#               a         { dstchan0_vld dstchan1_vld ... dstchanN_vld } = allchan_vld
#               a         allchan_rdy = { dstchan0_rdy dstchan1_rdy ... dstchanN_rdy }
#               a         { dstchan0 dstchan1 ... dstchanN } = allchan
#               (Note: all destination channels must have the same width)

MaxLineLen = 70

def abbrev_instance_name(modulename)
  # foo_bar_baz --> baz
  # FooBarBaz   --> baz
  modulename.sub(/^.*_([a-zA-Z])/){$1}.sub(/^.*[a-z\d]([A-Z])/){$1}.downcase \
    .gsub(/_+/,"_").gsub(/([a-z])_/){$1}.sub(/(.)_$/){$1}
end

def handshaking_idiom_fork(nr_chan, srcvld, srcrdy, alldstvld, alldstrdy)
  "m FORKHSK\#(#{nr_chan}) clk rst #{srcvld} #{srcrdy} #{alldstvld} #{alldstrdy}"
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
      ix = commalist.rindex(?,, MaxLineLen-indent2-1) || commalist.index(?,, MaxLineLen-indent2)
      ix  or break
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
    ix = line.rindex(?,, MaxLineLen-1) || line.index(?,, MaxLineLen)
    ix  or break
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

ModuleInfo = Struct.new(:name, :params, :args, :arglines, :bodylines, :instseq, :topname, :hskwidth)
ModInfo = ModuleInfo.new

ModInfo.name = nil
ModInfo.params = []
ModInfo.args = []
ModInfo.arglines = []
ModInfo.bodylines = []
ModInfo.instseq = {}
ModInfo.topname = nil	# name of the first (topmost) module
ModInfo.hskwidth = {}	# width of handshaking channels

def print_module
  println(prettyprint(0, "module #{ModInfo.name}(", ModInfo.args.join(","), ");"))
  println(ModInfo.arglines, ModInfo.bodylines, "endmodule")
  ModInfo.name = nil
  ModInfo.params.clear
  ModInfo.args.clear
  ModInfo.arglines.clear
  ModInfo.bodylines.clear
  ModInfo.instseq.clear
  ModInfo.hskwidth.clear
end

def prepend_optional_prefix(name)
  # e.g.
  #   when ModInfo.topname = "FooBar",
  #      "$Baz" --> "FooBarBaz"
  #   when ModInfo.topname = "foo_bar",
  #      "$baz" --> "foo_bar_baz"
  name.sub(/^\$(.)/){
    letter = $1
    ModInfo.topname + ((letter =~ /[A-Z]/) ? "" : "_") + letter
  }
end

def instance_name(submod)
  name = abbrev_instance_name(submod)
  seqnum = (ModInfo.instseq[name] ||= 0)
  instname = name + ((name =~ /\d$/) ? "_" : "") + seqnum.to_s
  ModInfo.instseq[name] = seqnum + 1
  instname
end

def append_instance_name(submod)
  if (submod =~ /\[\S+:\S+\]$/) then
    submod = $`
    range = $&
  else
    range = ""
  end
  submod = prepend_optional_prefix(submod)
  ans = submod.sub(/:(\w+)$/){" "+$1}
  if (ans == submod) then
    submod =~ /^\w+/
    ans = submod + " " + instance_name($&)
  end
  ans + range
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
    if (str =~ /^\w+(\[(\d+([-,]\d+)+)\]\w*)+@?$/) then
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

def expand_handshake(name)
  name.sub!(/^(\w+)@$/){$1} ? [name+"_vld", name+"_rdy", name] : name
end

TypeStr = { "i" => "input", "o" => "output", "w" => "wire", "a" => "assign", "p" => "parameter", "l" => "localparam" }
TypeStr_rdy = { "i" => "output", "o" => "input", "w" => "wire" }

def width_of_compound_handshaking_channel(channels)
  nr_chan = channels.size
  ModInfo.hskwidth.include?(channels[0]) or fail "unknown handshaking channel #{channels[0]}"
  widthdef = ModInfo.hskwidth[channels[0]]
  channels[1..-1].each do |s|
    ModInfo.hskwidth.include?(s) or fail "unknown handshaking channel #{s}"
    (ModInfo.hskwidth[s] == widthdef)  or fail "non-uniform handshaking channel widths"
  end
  case widthdef
  when " "
    width = nr_chan.to_s
  when /^ \[(\d+):0\] $/
    width = "#{$1.to_i+1}*#{nr_chan}"
  when /^ \[(.+)-1:0\] $/
    width = "#{$1}*#{nr_chan}"
  else
    fail
  end
  width
end

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
  when "p","l"
    type = arr.shift
    if (arr[0] =~ /^\d+$/) then
      (type != "a")  or fail "syntax error"
      width = " [#{arr[0].to_i-1}:0] "
      arr.shift
    elsif (ModInfo.params.include?(arr[0])) then
      (type != "a")  or fail "syntax error"
      width = " [#{arr[0]}-1:0] "
      arr.shift
    elsif (arr[0] !~ /^\w+$/) then
      (type != "a")  or fail "syntax error"
      width = " [(#{arr[0]})-1:0] "
      arr.shift
    else
      width = " "
    end
    (! arr.empty?) && (arr.size % 2) == 0  or fail "syntax error"
    paramdecl = (0...(arr.size/2)).map{ |i|
      name, value = arr[i*2,2]
      (name =~ /^\w+$/) && (!ModInfo.params.include?(name))  or fail "syntax error"
      ModInfo.params.push(name)
      name + "=" + value
    }
    line = TypeStr[type] + width + paramdecl.join(",") + ";"
    ModInfo.arglines.push(wrap_line(line))
  when "i","o","w","a"
    type = arr.shift
    if (arr[0] =~ /^\d+$/) then
      (type != "a")  or fail "syntax error"
      width = " [#{arr[0].to_i-1}:0] "
      arr.shift
    elsif (ModInfo.params.include?(arr[0])) then
      (type != "a")  or fail "syntax error"
      width = " [#{arr[0]}-1:0] "
      arr.shift
    elsif ((type != "a") && (arr[0] !~ /^\w+@?$/)) then
      width = " [(#{arr[0]})-1:0] "
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
      handshake = sig.sub!(/@$/, "")
      if (handshake) then
        # handshake channel : must be
        #   o   foo = submod ( arg0 arg1 ... )
        #   o 8 foo = submod ( arg0 arg1 ... )
        (arr.size >= 5 && arr[3] == "(" && arr[-1] == ")")   or fail "syntax error"
        submod = append_instance_name(arr[2])
        sig_vld = sig + "_vld"
        sig_rdy = sig + "_rdy"
        unless (type == "a") then
          line = TypeStr[type] + width + sig + ";"
          line_vld = TypeStr[type] + " " + sig_vld + ";"
          line_rdy = TypeStr_rdy[type] + " " + sig_rdy + ";"
          if (type == "o") then
            ModInfo.args.push(sig_vld, sig_rdy, sig)
            ModInfo.arglines.push(line_vld, line_rdy, line)
          else
            ModInfo.bodylines.push(line_vld, line_rdy, line)
          end
          ModInfo.hskwidth[sig] = width
        end
        args = arr[4..-2].map{|x| expand_handshake(x)}
        args.push(sig_vld, sig_rdy, sig)
        ModInfo.bodylines.push(prettyprint(0, "#{submod}(", args.join(","), ");"))
      else
        if (sig[0] == ?{ && sig[-1] == ?}) then
          lines = prettyprint(0, TypeStr[type]+width+"{", sig[1..-2], "}")
          line = lines.pop
          ModInfo.bodylines.push(lines)  unless lines.empty?
        else
          line = TypeStr[type] + width + sig
          if (type == "o") then
            ModInfo.args.push(sig)
            ModInfo.arglines.push(line+";")
            type = "a"
            width = " "
            line = TypeStr[type] + width + sig
          end
        end
        if (arr.size >= 5 && arr[2] == "{" && arr[-1] == "}") then
          ModInfo.bodylines.push(prettyprint(0, line+" = {", arr[3..-2].map{|x| expand_handshake(x)}.join(","), "};"))
        elsif (arr.size >= 5 && arr[3] == "(" && arr[-1] == ")") then
          args = arr[4..-2].map{|x| expand_handshake(x)}
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
      end
    else
      # o   foo bar ...
      # o 8 foo bar ...
      (type != "a")  or fail "syntax error"
      if (arr.any?{|name| name =~ /@$/}) then
        # contains handshake channel
        descs = []
        arr.each do |name|
          if (name.sub!(/@$/,"")) then
            descs.push([TypeStr[type], " ", name+"_vld"],
                       [TypeStr_rdy[type], " ", name+"_rdy"])
            ModInfo.hskwidth[name] = width
          end
          descs.push([TypeStr[type], width, name])
        end
        descs.each do |desc|
          typestr, widthstr, namestr = *desc
          line = typestr + widthstr + namestr + ";"
          if (type == "i" || type == "o") then
            ModInfo.args.push(namestr)
            ModInfo.arglines.push(line)
          else
            ModInfo.bodylines.push(line)
          end
        end
      else
        line = TypeStr[type] + width + arr.join(",") + ";"
        if (type == "i" || type == "o") then
          ModInfo.args.push(arr)
          ModInfo.arglines.push(wrap_line(line))
        else
          ModInfo.bodylines.push(wrap_line(line))
        end
      end
    end
  when "m"
    submod = append_instance_name(arr[1])
    args = arr[2..-1].map{|x| expand_handshake(x)}
    ModInfo.bodylines.push(prettyprint(0, "#{submod}(", args.join(","), ");"))
  when "@"
    if ((arr.size == 4) && (arr[2] == "=")) then
      # @ dstchan = srcchan
      dstchan, srcchan = arr[1], arr[3]
      convert("a #{dstchan}_vld = #{srcchan}_vld")
      convert("a #{srcchan}_rdy = #{dstchan}_rdy")
    elsif ((arr.size >= 7) && (arr[2] == "=") && (arr[3] == "{") && (arr[-1] == "}")) then
      # @ allchan = { srcchan0 srcchan1 ... srcchanN } 
      nr_chan = arr.size - 5
      allchan, srcchans = arr[1], arr[4..-2]
      width = width_of_compound_handshaking_channel(srcchans)
      convert("w #{nr_chan} #{allchan}_vld #{allchan}_rdy")
      convert("w #{width} #{allchan}")
      convert("a #{allchan}_vld = { " + srcchans.map{|s| s+"_vld"}.join(" ") + " }")
      convert("a { " + srcchans.map{|s| s+"_rdy"}.join(" ") + " } = #{allchan}_rdy")
      convert("a #{allchan} = { " + srcchans.join(" ") + " }")
    elsif ((arr.size >= 7) && (arr[1] == "{") && (arr[-3] == "}") && (arr[-2] == "=")) then
      # @ { dstchan0 dstchan1 ... dstchanN } = allchan
      nr_chan = arr.size - 5
      dstchans, allchan = arr[2..-4], arr[-1]
      width = width_of_compound_handshaking_channel(dstchans)
      convert("w #{nr_chan} #{allchan}_vld #{allchan}_rdy")
      convert("w #{width} #{allchan}")
      convert("a { " + dstchans.map{|s| s+"_vld"}.join(" ") + " } = #{allchan}_vld")
      convert("a #{allchan}_rdy = { " + dstchans.map{|s| s+"_rdy"}.join(" ") + " }")
      convert("a { " + dstchans.join(" ") + " } = #{allchan}")
    elsif ((arr.size >= 5) && arr[2] == "=") then
      # @ dstchan = srcchan0 srcchan1 srcchan2 ... srcchanN
      nr_chan = arr.size - 3
      dstchan, srcchans = arr[1], arr[3..-1]
      srcvlds = srcchans.map{|s| s+"_vld"}
      ModInfo.bodylines.push(prettyprint(0, "assign #{dstchan}_vld = &{", srcvlds.join(","), "};"))
      nr_chan.times do |i|
        other_srcvlds = (srcvlds[0,i]+srcvlds[-nr_chan+1+i,nr_chan-1-i]).join(",")
        ModInfo.bodylines.push(prettyprint(0, "assign #{srcchans[i]}_rdy = &{", other_srcvlds+",#{dstchan}_rdy", "};"))
      end
    elsif ((arr.size >= 5) && arr[-2] == "=") then
      # @ dstchan0 dstchan1 ... dstchanN = srcchan
      nr_chan = arr.size - 3
      dstchans, srcchan = arr[1..-3], arr[-1]
      alldstvld = "{" + dstchans.map{|s| s+"_vld"}.join(",") + "}"
      alldstrdy = "{" + dstchans.map{|s| s+"_rdy"}.join(",") + "}"
      convert(handshaking_idiom_fork(nr_chan, srcchan+"_vld", srcchan+"_rdy", alldstvld, alldstrdy))
    elsif (arr.size >= 3) then
      # @ [WIDTH] NUM allchan0 [allchan1 ...]
      if ((arr[1] =~ /^\d+$/) || ModInfo.params.include?(arr[1])) then
        width = arr[1]
      elsif (arr[1] !~ /^\w+$/) then
        width = "(" + arr[1] + ")"
      else
        fail "syntax error"
      end
      if ((arr[2] =~ /^\d+$/) || ModInfo.params.include?(arr[2])) then
        ix = 3
        width += "*" + arr[2]
        nr_chan = arr[2]
      elsif (arr[2] !~ /^\w+$/) then
        ix = 3
        width += "*(" + arr[2] + ")"
        nr_chan = arr[2]
      else
        ix = 2
        width = nr_chan = arr[1]
      end
      (arr.size > ix)  or fail "syntax error"
      allchans = arr[ix..-1]
      convert("w #{nr_chan} " + allchans.map{|s| s+"_vld "+s+"_rdy"}.join(" "))
      convert("w #{width} " + allchans.join(" "))
    else
      fail "syntax error"
    end          
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
    when /^module ((\$)?\w+)$/
      ModInfo.name.nil?  or fail "module declaration within another module definition"
      if ($2) then
        ModInfo.topname  or fail "prefixed submodule definition w/o preceding parent module definition"
      else
        ModInfo.topname ||= $1
      end
      ModInfo.name = prepend_optional_prefix($1)
    when /^[a-z@]\s/
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
