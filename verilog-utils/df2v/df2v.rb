#!/usr/bin/ruby -w
#
# FDFL (Fixed-point dataflow language) to Verilog converter
#
# Copyright (c) 2008 the handy-eda-utils develeper(s).
# Distributed under the MIT License.
# (See accompanying file COPYING or copy at
#  http://www.opensource.org/licenses/mit-license.php.)
#
# Last modified on February 2008.

require 'set'

CLKNAME = "i_clk"

def abbrev_instance_name(modulename)
  if (modulename =~ /^SC\d\d(\w+)X\w1$/) then
    $1.downcase
  else
    modulename.sub(/.*_/,"")
  end
end

class Integer
  def to_s_width(width)
    width ? ("%0*d" % [width,self]) : self.to_s
  end
end

class Parser

  ValidName = /^[a-zA-Z_]\w*$/

  class BitWidth
    class ImmOrParam
      def initialize(val)
        @val = val
      end
      def imm?
        Integer === @val
      end
      def param?
        String === @val
      end
      def imm
        imm? ? @val : nil
      end
      def param
        param? ? @val : nil
      end
      def paramindex
        param? ? (@val[1] - ?A) : nil
      end
      def ==(other)
        @val == other.val
      end
      def eql?(other)
        @val.eql?(other.val)
      end
      def hash
        @val.hash
      end
      protected
      attr_reader :val
    end
    def initialize(type, totalbits = 1, fracbits = 0)
      @type = type
      @totalbits = ImmOrParam.new(totalbits)
      @fracbits = ImmOrParam.new(fracbits)
    end
    attr_reader :type, :totalbits, :fracbits
    def bool?
      @type == "b"
    end
    def effectively_bool?
      (@type == "b") || (@type == "u" && @totalbits.imm == 1 && @fracbits.imm == 0)
    end
    def signed?
      @type == "s"
    end
    def nameindex
      (@type == "b") ? 2 : 4
    end
    def parameterized?
      @totalbits.param? || @fracbits.param?
    end
    def ==(other)
      (@type == other.type) && (@totalbits == other.totalbits) && (@fracbits == other.fracbits)
    end
    def attrlist
      [signed?, @totalbits.imm, @fracbits.imm, @totalbits.imm-@fracbits.imm]
    end
    def to_s(flag = nil)
      if (bool?) then
        "b"
      else
        sign = (flag == :sign_ignore) ? "i" : (signed? ? "s" : "u")
        tb = @totalbits.imm
        fb = @fracbits.imm
        ib = tb - fb
        "%s%s%df%s%d" % [ sign, (ib<0) ? "m" : "", ib.abs, (fb<0) ? "m" : "", fb.abs ]
      end
    end
  end

  def self.prettyprint(indent, headstr, commalist, tailstr, maxcolumn)
    indentstr = " "*indent
    if (indent + headstr.length + commalist.length + tailstr.length <= maxcolumn) then
      print(indentstr, headstr, commalist, tailstr, "\n")
    else
      print(indentstr, headstr, "\n")
      indent2 = indent + 2
      indentstr2 = " "*indent2
      while (indent2+commalist.length > maxcolumn) do
        ix = commalist.rindex(?,, maxcolumn-indent2-1)
        print(indentstr2, commalist[0..ix], "\n")
        commalist = commalist[(ix+1)..-1]
      end
      print(indentstr2, commalist, "\n")
      print(indentstr, tailstr, "\n")
    end
  end

  class Signal
    def initialize(name, iwo, index, width)
      registered = false
      case iwo
      when "r"
        iwo = "w"
        registered = true
      when "q"
        iwo = "o"
        registered = true
      end
      @name = name
      @iwo = iwo
      @index = index
      @width = width
      @registered = registered
    end
    attr_reader :name, :iwo, :index, :width
    def registered?
      @registered
    end
    def rdname
      @name
    end
    def wrname
      @registered ? (@name+"_next_") : @name
    end
  end

  class Parameter
    def initialize(name, type)
      @name = name
      @type = type
      @sig = []
    end
    attr_reader :name, :type, :sig
  end

  module AssignmentOperation
    # :copy, :verbose, :concat, :add, :sub, :mul, :div, :shl, :shr,
    # :cmplt, :cmpgt, :cmple, :cmpge, :maskand, :maskxor, :squ, :neg, :ifthenelse
    # :decompose, :decomposeverbose
    def self.emit_verilog_usecode(desc, instancename, sigtab)
      sym = desc[0]
      dst = desc[1]
      args = desc[2..-1]
      case sym
      when :copy
        do_copy(dst, args[0], sigtab)
      when :verbose
        do_verbose(dst, args[0], sigtab)
      when :concat
        do_concat(dst, args, sigtab)
      when :add, :sub, :mul, :div
        do_arith(sym, instancename, dst, args[0], args[1], sigtab)
      when :shl, :shr
        do_shift(sym, dst, args[0], args[1], sigtab)
      when :cmplt, :cmpgt, :cmple, :cmpge
        do_cmp(sym, instancename, dst, args[0], args[1], sigtab)
      when :maskand, :maskxor
        do_masklogic(sym, dst, args[0], args[1], sigtab)
      when :squ, :neg
        do_unary(sym, instancename, dst, args[0], sigtab)
      when :ifthenelse
        do_ifthenelse(dst, args[0], args[1], args[2], sigtab)
      when :decompose
        do_decompose(dst, args[0], sigtab)
      when :decomposeverbose
        do_decomposeverbose(dst, args[0], sigtab)
      else
        fail "internal error"
      end
    end

    private
    def self.do_copy(dst, src, sigtab)
      dsig = sigtab[dst]
      ssig = sigtab[src]
      if (dsig.width.effectively_bool? && ssig.width.effectively_bool?) then
        print("  assign #{dsig.wrname} = #{ssig.rdname};\n")
      else
        ds, dt, df, di = dsig.width.attrlist
        ss, st, sf, si = ssig.width.attrlist
        print("  assign #{dsig.wrname} = {")
        if (di > si) then
          if (ss) then
            printf(" {%d{%s[%d]}},", di-si, ssig.rdname, st-1)
          else
            printf(" {%d{1'b0}},", di-si)
          end
        end
        if (df <= sf) then
          printf(" %s[%d:%d]", ssig.rdname, [di,si].min + sf - 1, sf - df)
        else
          printf(" %s[%d:0], {%d{1'b0}}", ssig.rdname, [di,si].min + sf - 1, df - sf)
        end
        print(" };\n")
      end
    end
    def self.do_verbose(dst, src, sigtab)
      print("  assign #{sigtab[dst].wrname} = #{src};\n")
    end
    def self.do_concat(dst, args, sigtab)
      allwidth = 0
      args.each do |arg|
        if (arg =~ ValidName) then
          sigtab.include?(arg)  or fail "unknown signal #{arg}"
          allwidth += sigtab[arg].width.attrlist[1]
        else
          allwidth = nil
          break
        end
      end
      if (allwidth) then
        (sigtab[dst].width.attrlist[1] == allwidth)  or fail "concatenation width mismatch"
      end
      print("  assign #{sigtab[dst].wrname} = {#{args.join(',')}};\n")
    end
    def self.do_arith(sym, instancename, dst, left, right, sigtab)
      dsig = sigtab[dst]
      lsig = sigtab[left]
      rsig = sigtab[right]
      dw = dsig.width
      lw = lsig.width
      rw = rsig.width
      ds, dt, df, di = dw.attrlist
      ls, lt, lf, li = lw.attrlist
      rs, rt, rf, ri = rw.attrlist
      sign = (ls ? "s" : "u") + (rs ? "s" : "u")
      printf("  fix_%s%s #(%d,%d,%d,%d,%d,%d) %s(%s, %s, %s);\n",
             sym.to_s, sign, li,lf,ri,rf,di,df,
             instancename, lsig.rdname, rsig.rdname, dsig.wrname)
    end
    def self.do_shift(sym, dst, left, right, sigtab)
      dsig = sigtab[dst]
      lsig = sigtab[left]
      ds, dt, df, di = dsig.width.attrlist
      ls, lt, lf, li = lsig.width.attrlist
      shl = (sym == :shl) ? right.to_i : -(right.to_i)
      (ds == ls) && (dt == lt) && (df == lf-shl)  or fail "shift operand type mismatch"
      printf("  assign %s = %s;\n", dsig.wrname, lsig.rdname)
    end
    def self.do_cmp(sym, instancename, dst, left, right, sigtab)
      lsig = sigtab[left]
      rsig = sigtab[right]
      dsig = sigtab[dst]
      lw = lsig.width
      rw = rsig.width
      ls, lt, lf, li = lw.attrlist
      rs, rt, rf, ri = rw.attrlist
      sign = (ls ? "s" : "u") + (rs ? "s" : "u")
      printf("  fix_%s%s #(%d,%d,%d,%d) %s(%s, %s, %s);\n",
             sym.to_s, sign, li,lf,ri,rf,
             instancename, lsig.rdname, rsig.rdname, dsig.wrname)
    end
    def self.do_masklogic(sym, dst, left, right, sigtab)
      dsig = sigtab[dst]
      rsig = sigtab[right]
      ds, dt, df, di = dsig.width.attrlist
      rs, rt, rf, ri = rsig.width.attrlist
      (ds == rs) && (dt == rt) && (df == rf)  or fail "#{sym.to_s} width mismatch"
      printf("  assign %s = {%d{%s}} %s %s;\n",
             dsig.wrname, dt, left, (sym == :maskand) ? "&" : "^", rsig.rdname)
    end
    def self.do_unary(sym, instancename, dst, operand, sigtab)
      dsig = sigtab[dst]
      osig = sigtab[operand]
      dw = dsig.width
      ow = osig.width
      ds, dt, df, di = dw.attrlist
      os, ot, of, oi = ow.attrlist
      sign = (os ? "s" : "u")
      printf("  fix_%s%s #(%d,%d,%d,%d) %s(%s, %s);\n",
             sym.to_s, sign, oi,of,di,df,
             instancename, osig.rdname, dsig.wrname)
    end
    def self.do_ifthenelse(dst, cond, left, right, sigtab)
      dsig = sigtab[dst]
      lsig = sigtab[left]
      rsig = sigtab[right]
      ds, dt, df, di = dsig.width.attrlist
      ls, lt, lf, li = lsig.width.attrlist
      rs, rt, rf, ri = rsig.width.attrlist
      ((ds == ls) && (ds == rs) && (dt == lt) && (dt == rt) && (df == lf) && (df == rf)) \
        or fail "ifthenelse width mismatch"
      printf("  assign %s = %s ? %s : %s;\n", dsig.wrname, cond, lsig.rdname, rsig.rdname)
    end
    def self.do_decompose(dsts, src, sigtab)
      dsigs = dsts.map{|dst| sigtab[dst]}
      ssig = sigtab[src]
      (dsigs.inject(0){|r,dsig| r + dsig.width.attrlist[1]} == ssig.width.attrlist[1]) \
        or fail "decomposition width mismatch"
      dsignames = dsigs.map{|dsig| dsig.wrname}
      print("  assign {#{dsignames.join(",")}} = #{ssig.rdname};\n")
    end
    def self.do_decomposeverbose(dsts, src, sigtab)
      dsignames = dsts.map{|dst| sigtab[dst].wrname}
      print("  assign {#{dsignames.join(",")}} = #{src};\n")
    end
  end

  module BuiltinCircuit
    # :addandsub, :addci
    def self.match_parameter(sym, args)
      case sym
      when :addandsub
        (args.size == 4)  or return false
      when :addci
        (args.size == 4) or return false
        args[2].width.effectively_bool?  or return false
      end
      args
    end
    def self.emit_verilog_usecode(sym, instancename, args, sigtab)
      case sym
      when :addandsub
        do_addandsub(instancename, args[0], args[1], args[2], args[3], sigtab)
      when :addci
        do_addci(instancename, args[0], args[1], args[2], args[3], sigtab)
      else
        fail "internal error"
      end
    end

    private
    def self.do_addandsub(instancename, left, right, sum, diff, sigtab)
      lsig = sigtab[left]
      rsig = sigtab[right]
      ssig = sigtab[sum]
      dsig = sigtab[diff]
      lw = lsig.width
      rw = rsig.width
      sw = ssig.width
      dw = dsig.width
      ls, lt, lf, li = lw.attrlist
      rs, rt, rf, ri = rw.attrlist
      ss, st, sf, si = sw.attrlist
      ds, dt, df, di = dw.attrlist
      sign = (ls ? "s" : "u") + (rs ? "s" : "u")
      printf("  fix_addandsub%s #(%d,%d,%d,%d,%d,%d,%d,%d) %s(%s, %s, %s, %s);\n",
             sign, li,lf,ri,rf,si,sf,di,df,
             instancename, lsig.rdname, rsig.rdname, ssig.wrname, dsig.wrname)
    end
    def self.do_addci(instancename, left, right, rightci, sum, sigtab)
      lsig = sigtab[left]
      rsig = sigtab[right]
      csig = sigtab[rightci]
      ssig = sigtab[sum]
      lw = lsig.width
      rw = rsig.width
      sw = ssig.width
      ls, lt, lf, li = lw.attrlist
      rs, rt, rf, ri = rw.attrlist
      ss, st, sf, si = sw.attrlist
      sign = (ls ? "s" : "u") + (rs ? "s" : "u")
      printf("  fix_addci%s #(%d,%d,%d,%d,%d,%d) %s(%s, %s, %s, %s);\n",
             sign, li,lf,ri,rf,si,sf,
             instancename, lsig.rdname, rsig.rdname, csig.rdname, ssig.wrname)
    end
  end

  class Circuit
    def initialize(name)
      @name = name
      @input = []	# index -> name
      @wire = []	# index -> name
      @output = []	# index -> name
      @operation = []	# list of assignments and submodule instantiations
      @sig = {}		# name -> Signal
      @param = {}	# paramname (e.g. "@A","@B") -> Parameter
      @usedparamdesc = Set.new
      @definedparamdesc = Set.new
      @need_clk = nil
      # Elements of @usedparamdesc and @definedparamdesc are
      #   array pairs of modulename and paramval.
      # A paramval is an array of integer or nil.
      #   (each integer corresponds to @A,@B,@C...)
      # @usedparamdesc = already instantiated parameter combinations
      # @definedparamdesc = already defined parameter combinations
    end
    attr_reader :name, :input, :wire, :output, :operation, :sig, :param
    def add_signal(sig)
      case sig.iwo
      when "i"
        @input.push(sig.name)
      when "w"
        @wire.push(sig.name)
      when "o"
        @output.push(sig.name)
      else
        fail "internal error"
      end
      @sig[sig.name] = sig
      tpname = sig.width.totalbits.param
      fpname = sig.width.fracbits.param
      if (tpname) then
        if (@param.include?(tpname)) then
          (@param[tpname].type == :totalbits)  or fail "parameter type mismatch"
        else
          @param[tpname] = Parameter.new(tpname, :totalbits)
        end
        @param[tpname].sig.push(sig)
      end
      if (fpname) then
        if (@param.include?(fpname)) then
          (@param[fpname].type == :fracbits)  or fail "parameter type mismatch"
        else
          @param[fpname] = Parameter.new(fpname, :fracbits)
        end
        @param[fpname].sig.push(sig)
      end
    end
    def parameterized?
      !@param.empty?
    end
    def match_parameter(args)	# array of argument Signals
      # true : match
      # false : mismatch
      # [int_or_str, int_or_str, ...] : variable parameter list
      (args.size == @input.size + @output.size)  or fail "parameter number mismatch"
      paramval = Array.new(26)
      args.each_with_index do |arg,ix|
        signame = (ix < @input.size) ? @input[ix] : @output[ix-@input.size]
        sig = @sig[signame]
        argwidth = arg.width
        sigwidth = sig.width
        (sigwidth.type == argwidth.type)  or return false
        if (sigwidth.totalbits.imm?) then
          (sigwidth.totalbits == argwidth.totalbits)  or return false
        else
          pn = sigwidth.totalbits.paramindex
          if (paramval[pn]) then
            (paramval[pn] == argwidth.totalbits)  or return false
          else
            paramval[pn] = argwidth.totalbits
          end
        end
        if (sigwidth.fracbits.imm?) then
          (sigwidth.fracbits == argwidth.fracbits)  or return false
        else
          pn = sigwidth.fracbits.paramindex
          if (paramval[pn]) then
            (paramval[pn] == argwidth.fracbits)  or return false
          else
            paramval[pn] = argwidth.fracbits
          end
        end
      end
      parameterized? ? paramval : true
    end
    def need_clock?
      if (@need_clk.nil?) then
        if (@sig.values.find{|s| s.registered?}) then
          @need_clk = true
        elsif (@operation.find{|op| (op[0] == :instantiation) && op[1].need_clock? }) then
          @need_clk = true
        else
          @need_clk = false
        end
      end
      @need_clk
    end

    def emit_verilog_usecode(instancename, args, sigtab, explicitparam)
      argsig = args.map{|x| sigtab[x]}
      params = match_parameter(argsig)  or fail "internal error"
      if (Array === params) then
        # parameterized instantiation
        if (@operation.empty?) then
          # externally defined parameterized module
          if (explicitparam) then
            modulename = @name + " #" + explicitparam
          else
            modulename = @name + " #(" + params.compact.map{|x| x.imm}.join(",") + ")"
          end
        else
          (!explicitparam)  or fail "explicit parameter(s) for a non-external submodule"
          modulename = @name + "_"
          params.each_with_index do |val,ix|
            if (val) then
              modulename += ("%c%s%d" % [(?A+ix), (val.imm<0) ? "m" : "", val.imm.abs])
            end
          end
          paramdesc = [modulename, params]
          @usedparamdesc << paramdesc  unless (@definedparamdesc.include?(paramdesc))
        end
      else
        # non-parameterized instantiation (trivial)
        modulename = @name
      end
      headstr = "%s %s(" % [modulename, instancename]
      tailstr = ");"
      cargs = need_clock? ? [CLKNAME] : []
      iargs = argsig[0,@input.size].map{|s| s.rdname}
      oargs = argsig[@input.size..-1].map{|s| s.wrname}
      Parser.prettyprint(2, headstr, (cargs+iargs+oargs).join(","), tailstr, 70)
    end
    def emit_verilog_defcode(paramdesc = nil)
      if (parameterized? && paramdesc.nil?) then
        #print("DEFER #{@name}\n")
        return
      end
      if (parameterized?) then
        modulename, paramval = paramdesc
        asig = {}
        @sig.each do |signame,sig|
          bw = sig.width
          tb = bw.totalbits.param? ? paramval[bw.totalbits.paramindex].imm : bw.totalbits.imm
          fb = bw.fracbits.param? ? paramval[bw.fracbits.paramindex].imm : bw.fracbits.imm
          asig[signame] = Signal.new(signame, sig.iwo, sig.index,
                                     BitWidth.new(bw.type, tb, fb))
        end
      else
        modulename = @name
        asig = @sig
      end
      if (@operation.empty?) then
        print("// module #{modulename}(...);  [externally defined]\n\n")
        return
      end
      Parser.prettyprint(0, "module #{modulename}(",
                         (need_clock? ? (CLKNAME+",") : "") + (@input+@output).join(","),
                         ");", 70)
      print("  input         #{CLKNAME};\n")  if need_clock?
      (@input.size+@output.size+@wire.size).times do |ix|
        if (ix < @input.size) then
          sig = asig[@input[ix]]
          decls = [ ["input ", sig.width, sig.name] ]
        elsif (ix < @input.size+@output.size) then
          sig = asig[@output[ix-@input.size]]
          if (sig.registered?) then
            decls = [
              ["output", sig.width, sig.rdname],
              ["reg   ", sig.width, sig.rdname],
              ["wire  ", sig.width, sig.wrname]
            ]
          else
            decls = [ ["output", sig.width, sig.name] ]
          end
        else
          sig = asig[@wire[ix-@input.size-@output.size]]
          if (sig.registered?) then
            decls = [ ["reg   ", sig.width, sig.rdname], ["wire  ", sig.width, sig.wrname] ]
          else
            decls = [ ["wire  ", sig.width, sig.name] ]
          end
        end
        decls.each do |decl|
          io,bw,rwname = decl
          if (bw.bool?) then
            printf("  #{io}        #{rwname};\n")
          else
            tb = bw.totalbits.imm
            fb = bw.fracbits.imm
            widthstr = "%s[%d:0]" % [(tb>=11) ? "" : " ", tb-1]
            str = "  #{io} #{widthstr} #{rwname};"
            printf("%-39s // %s[%d,%d]\n", str, bw.type.upcase, tb-fb, fb)
          end
        end
      end
      instance_seqnum = {}
      @operation.each do |operation|
        case operation[0]
        when :assignment
          desc = operation[1]
          sym = desc[0]
          seqnum = (instance_seqnum[sym] ||= 0)
          instancename = sym.to_s + seqnum.to_s
          AssignmentOperation.emit_verilog_usecode(desc, instancename, asig)
          instance_seqnum[sym] = seqnum + 1
        when :builtincircuit
          sym, args, instancename = operation[1,3]
          unless instancename then
            seqnum = (instance_seqnum[sym] ||= 0)
            instancename = sym.to_s + seqnum.to_s
            instance_seqnum[sym] = seqnum + 1
          end
          BuiltinCircuit.emit_verilog_usecode(sym, instancename, args, asig)
        when :instantiation
          submodule, args, explicitparam, instancename = operation[1,4]
          unless instancename then
            name = abbrev_instance_name(submodule.name)
            seqnum = (instance_seqnum[name] ||= 0)
            instancename = name + ((name =~ /\d$/) ? "_" : "") + seqnum.to_s
            instance_seqnum[name] = seqnum + 1
          end
          submodule.emit_verilog_usecode(instancename, args, asig, explicitparam)
        else
          fail "internal error"
        end
      end
      if (@sig.values.find{|s| s.registered?}) then
        print("  always @(posedge #{CLKNAME}) begin\n")
        (@output.size+@wire.size).times do |ix|
          if (ix < @output.size) then
            name = @output[ix]
          else
            name = @wire[ix-@output.size]
          end
          sig = @sig[name]
          next unless sig.registered?
          printf("    %s <= %s;\n", sig.rdname, sig.wrname)
        end
        print("  end\n")
      end
      printf("endmodule\n\n")
    end
    def emit_verilog_defcodes
      doneflag = @usedparamdesc.empty? ? false : true
      @usedparamdesc.sort_by{|desc| desc[0]}.each do |desc|
        emit_verilog_defcode(desc)
        @definedparamdesc << desc
      end
      @usedparamdesc.clear
      doneflag
    end
  end

  def initialize
    @circuits = {}	# name -> Circuit
    @cur = nil
  end
  def parse(f)
    case f[0]
    when "module"
      do_module(f)
    when "i"
      do_input(f)
    when "w","o","r","q"
      do_wire_output(f)
    when "m"
      if (f.size >= 4 && f[2] == "=") then
        do_assignment(f)
      elsif (f.size >= 6 && (f.size & 1) == 0 && f[f.size/2] == "=") then
        ix2 = f.size/2 + 1
        ((f.size-2)/2).times do |i|
          do_assignment(["m", f[1+i], "=", f[ix2+i]])
        end
      elsif (f.size >= 6 && f[1] == "{" && f[-3] == "}" && f[-2] == "=") then
        do_decomposition(f)
      else
        do_submodule(f)
      end
    when "endmodule"
      do_endmodule(f)
    else
      fail "curious command '#{f[0]}'"
    end
  end
  def flush
    begin
      doneflag = false
      @circuits.each do |name,circuit|
        doneflag |= circuit.emit_verilog_defcodes
      end
    end while doneflag
  end
  private
  def do_module(f)
    @cur.nil?  or fail "nested module definition"
    f.size == 2  or fail "syntax error"
    name = f[1]
    name =~ ValidName  or fail "invalid module name '#{f[1]}'"
    (!@circuits.include?(name))  or fail "duplicate module name"
    @cur = Circuit.new(name)
  end
  def parse_bitwidth(f)
    case f[1]
    when "u","s"
      (f.size >= 5)  or fail "syntax error"
      type = f[1]
      case f[2]
      when /^\d+$/
        totalbits = f[2].to_i
      when /^\@[A-Z]$/
        totalbits = f[2]
      else
        fail "syntax error"
      end
      case f[3]
      when /^-?\d+$/
        fracbits = f[3].to_i
      when /^\@[A-Z]$/
        fracbits = f[3]
      else
        fail "syntax error"
      end
    when "b"
      (f.size >= 3)  or fail "syntax error"
      type = f[1]
      totalbits = 1
      fracbits = 0
    else
      fail "syntax error"
    end
    BitWidth.new(type, totalbits, fracbits)
  end
  def do_input(f)
    @cur  or fail "input command outside module definition"
    f.size >= 2  or fail "syntax error"
    bw = parse_bitwidth(f)
    f[bw.nameindex..-1].each do |name|
      (name =~ ValidName)  or fail "invalid name '#{name}'"
      (!@cur.sig.include?(name))  or fail "duplicate name '#{name}'"
      index = @cur.input.size
      sig = Signal.new(name, "i", index, bw)
      @cur.add_signal(sig)
    end
  end 
  def do_wire_output(f)
    cmd = f[0]
    case cmd
    when "w"
      cmdname = "wire"
    when "o"
      cmdname = "output"
    when "r"
      cmdname = "reg"
    when "q"
      cmdname = "outreg"
    else
      fail "internal error"
    end
    @cur  or fail "#{cmdname} command outside module definition"
    f.size >= 2  or fail "syntax error"
    bw = parse_bitwidth(f)
    ix = bw.nameindex
    assign = []
    if ((f.size >= ix+3) && f[ix+1] == "=") then
      assign.push([cmd] + f[ix..-1])
      f[(ix+1)..-1] = nil
    elsif ((f.size >= ix+5) && (((f.size-(ix+5)) & 1) == 0) \
           && f[ix+(f.size-(ix+1))/2] == "=") then
      n = (f.size-(ix+1)) / 2
      ix2 = ix + n + 1
      n.times do |i|
        assign.push([cmd, f[ix+i], "=", f[ix2+i]])
      end
      f[(ix+n)..-1] = nil
    end
    f[ix..-1].each do |name|
      (name =~ ValidName)  or fail "invalid name '#{name}'"
      (!@cur.sig.include?(name))  or fail "duplicate name '#{name}'"
      index = (cmd == "w" || cmd == "r") ? @cur.wire.size : @cur.output.size
      sig = Signal.new(name, cmd, index, bw)
      @cur.add_signal(sig)
    end
    assign.each do |ass|
      do_assignment(ass)
    end
  end
  The_BinOp = {
    "+" => :add,
    "-" => :sub,
    "*" => :mul,
    "/" => :div,
    "<<" => :shl,
    ">>" => :shr,
    "<"  => :cmplt,
    ">"  => :cmpgt,
    "<=" => :cmple,
    ">=" => :cmpge,
    "&"  => :maskand,
    "^"  => :maskxor,
  }
  The_BuiltinCircuit = {
    "addandsub" => :addandsub,
    "addci" => :addci,
  }
  def do_assignment(f)
    @cur  or fail "assignment command outside module definition"
    dest = f[1]
    assert_signal(dest)
    desc = nil
    if (f[-1] == ")") then
      # 0  1    2    3      4 5  last
      # m dest  = sudmodule ( ... )
      (f.size >= 6 && f[4] == "(")  or fail "syntax error"
      do_submodule(f.values_at(0,3,5..-2,1))
    elsif (f.size == 4) then
      # m dest = expression  (copy or verbose)
      if (f[3] =~ ValidName) then
        assert_signal(f[3])
        desc = [:copy, dest, f[3]]
      else
        desc = [:verbose, dest, f[3]]
      end
    elsif (f.size >= 6 && f[3] == '{' && f[-1] == '}') then
      desc = [:concat, dest] + f[4..-2]
    elsif (f.size == 6) then
      # binary expression
      left, opstr, right = f[3..5]
      The_BinOp.include?(opstr)  or fail "syntax error"
      op = The_BinOp[opstr]
      case op
      when :maskand, :maskxor
        (left =~ /^\{\{(.*)\}\}$/)  or fail "'#{opstr}' operator without broadcast-mask"
        left = $1
        assert_signal(right)
      when :shl, :shr
        assert_signal(left)
        right =~ /^\d+$/  or fail "non-const operand for '#{opstr}' operator"
      when :cmplt, :cmpgt, :cmple, :cmpge
        assert_signal(left)
        assert_signal(right)
        @cur.sig[dest].width.effectively_bool?  or fail "non-bool destination for cmp operator"
      when :mul
        assert_signal(left)
        assert_signal(right)
        if (left == right) then
          (!@cur.sig[dest].width.signed?)  or fail "signed result for squ operator"
          op = :squ
        end
      else
        assert_signal(left)
        assert_signal(right)
      end
      desc = [op, dest, left, right]
    elsif (f.size == 5 && f[3] == "-") then
      # unary minus (negation)
      op = :neg
      operand = f[4]
      assert_signal(operand)
      desc = [op, dest, operand]
    elsif (f.size == 8 && f[4] == "?" && f[6] == ":") then
      assert_signal(f[5])
      assert_signal(f[7])
      desc = [:ifthenelse, dest, f[3], f[5], f[7]]
    else
      fail "syntax error"
    end
    @cur.operation.push([:assignment, desc])  if desc
  end
  def do_decomposition(f)
    @cur  or fail "decomposition command outside module definition"
    dests = f[2..-4]
    dests.each do |dest| assert_signal(dest) end
    src = f[-1]
    if (src =~ ValidName) then
      assert_signal(src)
      desc = [:decompose, dests, src]
    else
      desc = [:decomposeverbose, dests, src]
    end
    @cur.operation.push([:assignment, desc])
  end
  def assert_signal(str)
    (str =~ ValidName)  or fail "syntax error"
    @cur.sig.include?(str)  or fail "unknown signal '#{str}'"
  end
  def do_submodule(f)
    # m submodulename sig0 sig1 sig2 ...
    (f.size >= 3)  or fail "syntax error"
    name = f[1].gsub(/:(\w+)$/, "")
    instname = $1
    if (instname) then
      (instname =~ ValidName)  or fail "syntax error"
    end
    name = name.gsub(/\(.*\)$/, "")
    explicitparam = $&
    (name =~ ValidName)  or fail "syntax error"
    (The_BuiltinCircuit.include?(name) || @circuits.include?(name)) \
      or fail "unknown submodule '#{name}'"
    args = f[2..-1]
    args.each_with_index do |arg,ix|
      assert_signal(arg)
    end
    sigargs = args.map{|x| @cur.sig[x]}
    if (The_BuiltinCircuit.include?(name)) then
      (!explicitparam)  or fail "explicit parameter(s) for a built-in circuit"
      sym = The_BuiltinCircuit[name]
      BuiltinCircuit.match_parameter(sym, sigargs) \
        or fail "parameter mismatch"
      @cur.operation.push([:builtincircuit, sym, args, instname])
    else
      @circuits[name].match_parameter(sigargs)  or fail "parameter mismatch"
      @cur.operation.push([:instantiation, @circuits[name], args, explicitparam, instname])
    end
  end
  def do_endmodule(f)
    @cur  or fail "endmodule outside module definition"
    f.size == 1  or fail "syntax error"
    @cur.emit_verilog_defcode
    @circuits[@cur.name] = @cur
    @cur = nil
  end
end

def expand_ranges(arr)
  ans = arr.dup
  i = 0
  while (i < ans.size) do
    str = ans[i]
    if (str =~ /^(\S*?)\[(\d+([-,]\d+)+)\](\S*)$/) then
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
      ans[i,1] = nums.map{|s| pre + s + post}
    else
      i += 1
    end
  end
  ans
end

begin
  parser = Parser.new
  loop = nil
  continued_str = ""
  while (gets) do
    next if $_ =~ /^\s*\#/
    next if $_ =~ /^\s*$/
    chomp!
    sub!(/\s+\#.*$/, "")
    if (sub!(/\\$/,"")) then
      continued_str += $_
      next
    end
    f = (continued_str + $_).split
    continued_str = ""
    low,high = 0, 0
    width = 1
    if (f[-1] =~ /^\[\d+([:,]\d+)+\]$/) then
      ranges = f[-1][1..-2]
      f.pop
      nums = []
      ranges.split(",").each do |rng|
        rng =~ /^(\d+)(:(\d+))?$/  or fail "range syntax error"
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
      nums.each do |idx|
        loop = idx
        idx1 = (idx.to_i+1).to_s_width(idx.length)
        parser.parse(expand_ranges(f.map{ |s|
          s.gsub(/\#/){idx}.gsub(/\$((un)?signed\()?/){$1?$&:idx1}
        }))
      end
      loop = nil
    else
      parser.parse(expand_ranges(f))
    end
  end
  parser.flush

rescue RuntimeError
  if (ARGF.closed?) then
    STDERR.print("Error: #{$!}.\n")
  else
    loopdesc = loop ? "(loop=#{loop})" : ""
    STDERR.print "#{ARGF.filename}:#{ARGF.file.lineno}#{loopdesc}: #{$!}.\n"
  end
  exit(2)
end

