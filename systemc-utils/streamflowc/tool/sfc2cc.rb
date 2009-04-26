#!/usr/bin/ruby -w
#
# StreamFlowC : A C++ coding style to describe hardware structure for stream processing
#
# Copyright (c) 2009 the handy-eda-utils developer(s).
# Distributed under the MIT License.
# (See accompanying file COPYING or copy at
#  http://www.opensource.org/licenses/mit-license.php.)
#
################################################################
#
# ruby sfc2cc.rb -h  foo.sfc > foo.h
# ruby sfc2cc.rb -cc foo.sfc > foo.cc
#

Delimiter = "////////////////////////////////////////////////////////////////\n"

PRINT_INSTANCE_NAME_UPON_BINDING_ERROR = true

def format_array(prefix, arr, sep, width, suffix)
  str_cont = sep.sub(/s+$/,"") + "\n" + (" " * prefix.length)
  width -= prefix.length
  ans = "#{prefix}"
  len = 0
  seplen = sep.length
  arr.each do |item|
    itemlen = item.length
    if (len == 0) then
      ans << item
      len = itemlen
    else
      newlen = len + seplen + itemlen
      if (newlen > width) then
        ans << str_cont << item
        len = itemlen
      else
        ans << sep << item
        len = newlen
      end
    end
  end
  ans + suffix
end

def bind_decl(indent, prefixtag, type=nil)
  prefix = (" " * indent) + "void #{prefixtag}operator()("
  arr = []
  arr.push("#{Modname}* streamflowc_parent")  if (type == :impl)
  arr.concat(Param.map{|desc| "#{desc.atype} #{desc.name}"})
  arr.concat(Input.map{|desc| "::streamflowc::portref<#{desc.gtype}>& #{desc.name}"})
  suffix = ")"
  format_array(prefix, arr, ",", 1, suffix)
end

def define_default_impl
  GenLines.concat(Template_desc)
  GenLines.push("struct #{Modname}#{Template_suffix}::impl_t {\n")
  pos = GenLines.size
  GenLines.push("};\n\n")
  pos
end

def define_default_bind
  GenLines.concat(Template_desc)
  GenLines.push(Inline_str)
  GenLines.push(bind_decl(0, "#{Modname}#{Template_suffix}::impl_t::", :impl))
  GenLines.push(" {\n")
  pos = GenLines.size
  GenLines.push("}\n\n")
  pos
end

def line_control(lineno=nil, autogen_srcline = nil)
  line = lineno || ARGF.file.lineno
  file = ARGF.filename + (autogen_srcline ? ".line#{autogen_srcline}" : "")
  "\#line #{line} \"#{file}\"\n"
end

def check_member_usage(line)
  UsedMember[:id]       = true  if (line =~ /\bstreamflowc_id\b/)
  UsedMember[:basename] = true  if (line =~ /\bstreamflowc_basename\b/)
  UsedMember[:name]     = true  if (line =~ /\bstreamflowc_name\b/)
end

begin
  ARGV.size >= 1  or fail "invalid argument(s)"
  opt = ARGV.shift
  (opt == "-h") or (opt == "-cc")  or fail "unrecognized option \"#{opt}\""
  Opt_header = (opt == "-h")

  # Assume the first line is "// -*- c++ -*-"
  gets  or fail "unexpected EOF"
  print if (Opt_header)

  # search "module" declaration line
  Template_desc = []
  print(line_control(2))  if (Opt_header)
  while true do
    gets  or fail "unexpected EOF"
    break if ($_ =~ /^module\s+(\w+)\s+/)
    print if (Opt_header)
    Template_desc.clear  if ($_ =~ /^template\s*</)
    Template_desc.push($_)
  end
  Modname = $1
  Template_desc.clear  unless ((!Template_desc.empty?) && Template_desc[-1] =~ />\s*(\/\/.*)?$/)
  Typename_str = Template_desc.empty? ? "" : "typename "
  Template_str = Template_desc.empty? ? "" : "template "
  print($_.sub(/^module/, "struct"))  if (Opt_header)

  # parse module declaration
  ModDeclLineno = ARGF.file.lineno
  state = nil
  ModSig = Struct.new(:type, :gtype, :atype, :name)
  Param = []
  Input = []
  Output = []
  piodecllineno = nil
  PIODeclLines = []
  while (true) do
    gets  or fail "unexpected EOF"
    case $_
    when /^\s*(param|input|output)\s*:\s*(\/\/.*)?$/
      piodecllineno = ARGF.file.lineno  unless piodecllineno
      state = $1
      PIODeclLines.push($_.sub(/^\s*/){$&+"//"})
    when /^\};\s*$/
      break
    else
      if (state) then
        PIODeclLines.push("#{$_}");
        chomp!
        sub!(/\s*\/\/.*$/,"")
        next if ($_ =~ /^\s*$/)
        $_ =~ /(([a-zA-Z_]\w*\s*,\s*)*[a-zA-Z_]\w*)\s*;\s*$/  or fail "syntax error"
        names = $1
        type = $`
        type.sub!(/^\s*/,"")
        type.sub!(/\s*$/,"")
        gtype = (type =~ /^:/ || type =~ />$/) ? " #{type} " : type
        names.gsub(/\s/,"").split(",").each do |name|
          desc = ModSig.new(type, gtype, nil, name)
          case state
          when "param"
            desc.atype = "#{Typename_str}::streamflowc::fastarg<#{gtype}>::type"
            Param.push(desc)
          when "input"
            desc.atype = "#{Typename_str}::streamflowc::port<#{gtype}>::arg_type"
            Input.push(desc)
          when "output"
            desc.atype = "::streamflowc::portref<#{gtype}>"
            Output.push(desc)
          end
        end
      else
        print  if (Opt_header)
      end
    end
  end

  InputIndex = {}
  Input.each_with_index do |desc,ix|
    InputIndex[desc.name] = ix
  end
  OutTypeCount = Hash.new(0)
  Output.each do |desc|
    OutTypeCount[desc.type] += 1
  end

  if Template_desc.empty? then
    Template_suffix = ""
  else
    Template_suffix = "<" + Template_desc.join("").gsub(/\s*\/\/.*$/,"").sub(/>\Z/,",").scan(/[a-zA-Z_]\w*\s*,/) \
                                         .join("").gsub(/\s+/,"").sub(/,$/,">")
  end

  # auto-generate module declaration
  if (Opt_header) then
    # dummy struct for syntax check
    print(line_control(1, ModDeclLineno), "  struct streamflowc_ports_t {\n")
    print(line_control(piodecllineno), PIODeclLines.map{|line| "  " + line})
    print("  };\n");
    # output ports etc.
    print(line_control(1, ARGF.file.lineno))
    Output.each do |desc|
      print("  #{desc.atype} #{desc.name};\n")
    end
    print("  explicit #{Modname}(const ::streamflowc::module_name& = 0);\n")
    print("  ~#{Modname}();\n")
    print(bind_decl(2, ""), ";\n")
    if (! Output.empty?) then
      if (Output.size == 1) then
        desc = Output[0]
        print("  operator #{desc.atype}&() { return #{desc.name}; }\n")
      elsif (OutTypeCount.values.sort[0] < 2) then
        outtypearr = OutTypeCount.keys
        prefix = "  typedef ::streamflowc::outtypes" + ((outtypearr[0] =~ /^:/) ? "< " : "<")
        suffix = ((outtypearr[-1] =~ />$/) ? " >" : ">") + " streamflowc_outtypes;\n"
        print(format_array(prefix, outtypearr, ",", 100, suffix))
        Output.each_with_index do |desc,ix|
          next if (OutTypeCount[desc.type] >= 2)
          gtype_ix = desc.gtype.sub(/\s*$/){",#{ix}#{$&}"}
          print("  operator #{Typename_str}streamflowc_outtypes::#{Template_str}nth<#{gtype_ix}>::type()",
                " { return #{desc.name}; }\n")
        end
      end
    end
    print("private:\n",
          "  struct impl_t;\n",
          "  impl_t *m_impl;\n",
          line_control,
          "};\n")
  end

  if (Opt_header) then
    if (Template_desc.empty?) then
      while (gets) do
        break if $_ == Delimiter
        print
      end
      exit 0
    end
  else 
    while (gets) do
      break if $_ == Delimiter
    end
    unless (Template_desc.empty?) then
      first = true
      while (gets) do
        next  if first && ($_ =~ /^\s*$/)
        first = false
        print
      end
      print "// Empty\n"  if first
      exit 0
    end
  end

  Def = {}
  GenLines = []
  Inline_str = "__attribute__((always_inline)) inline\n"
  UsedParam = {}
  UsedMember = { :id => false, :basename => false, :name => false }
  UsedMember[:basename] = true   if (PRINT_INSTANCE_NAME_UPON_BINDING_ERROR)
  genlines_impl = nil
  genlines_bind = nil
  genlines_end = nil
  explicit_impl_bind = 0
  state = :none   # none impl bind input
  GenLines.push(line_control(ARGF.file.lineno+1))  if (! Opt_header)
  while (gets) do
    break if (Opt_header && $_ == Delimiter)
    case $_
    when /^struct\s+#{Modname}\s*\{\s*(\/\/.*)?$/o
      (state == :none)  or fail "syntax error"
      Template_desc.each do |line|
        GenLines.push(line, line_control)
      end
      GenLines.push($_.sub(/^struct\s+#{Modname}/o){$&+"#{Template_suffix}::impl_t"})
      GenLines.push(line_control(1, ARGF.file.lineno))
      state = :impl
      genlines_impl = GenLines.size
      explicit_impl_bind += 1
      GenLines.push(line_control(ARGF.file.lineno+1))
    when /^#{Modname}\s*\(\s*\)(\s*\{(.*\})?)?\s*(\/\/.*)?$/o
      (state == :none)  or fail "syntax error"
      brace,body = $1,$2
      curline = line_control
      unless genlines_impl
        GenLines.push(line_control(1, ARGF.file.lineno))
        genlines_impl = define_default_impl
        GenLines.push(curline)
      end
      Template_desc.each do |line|
        GenLines.push(line, curline)
      end
      GenLines.push(Inline_str, curline)
      GenLines.push(bind_decl(0, "#{Modname}#{Template_suffix}::impl_t::", :impl).gsub(/\n/,"\n#{curline}"))
      GenLines.push(brace ? " {\n" : "\n")
      unless (brace) then
        gets  or fail "unexpected EOF"
        $_ == "{\n"  or fail "syntax error"
        GenLines.push($_)
      end
      GenLines.push(line_control(1, ARGF.file.lineno))
      genlines_bind = GenLines.size
      if (body) then
        body.sub!(/^\s*/, "  ")
        body.sub!(/\s+\}$/, "\n")
        check_member_usage(body)
        GenLines.push(curline, body, line_control, "}\n")
        genlines_end = [ GenLines.size, line_control(1, ARGF.file.lineno+1), line_control(ARGF.file.lineno+1) ]
      else
        state = :bind
        GenLines.push(line_control(ARGF.file.lineno+1))
      end
      explicit_impl_bind += 1
    when /^#{Modname}\s*\(\s*([a-zA-Z_]\w*)\s*\)(\s*\{(.*\})?)?\s*(\/\/.*)?$/o
      (state == :none)  or fail "syntax error"
      name,brace,body = $1,$2,$3
      InputIndex.include?(name)  or fail "curious input signal name"
      desc = Input[InputIndex[name]]
      curline = line_control
      unless genlines_impl
        GenLines.push(line_control(1, ARGF.file.lineno))
        genlines_impl = define_default_impl
        GenLines.push(curline)
      end
      unless genlines_bind then
        GenLines.push(line_control(1, ARGF.file.lineno))
        genlines_bind ||= define_default_bind
        GenLines.push(curline)
      end
      Template_desc.each do |line|
        GenLines.push(line, curline)
      end
      GenLines.push(Inline_str, curline)
      GenLines.push("void #{Modname}#{Template_suffix}::impl_t::streamflowc_mproc_#{name}(#{desc.atype} #{name})#{brace}\n")
      Def[name] = true
      if (body) then
        body.scan(/\bm_(\w+)\b/) do |match|
          UsedParam[match[0]] = true
        end
        check_member_usage(body)
        genlines_end = [ GenLines.size, line_control(1, ARGF.file.lineno+1), line_control(ARGF.file.lineno+1) ]
      else
        state = :input
        GenLines.push(line_control(ARGF.file.lineno+1))
      end
    else
      GenLines.push($_)
      $_.scan(/\bm_(\w+)\b/) do |match|   # no care for comments and strings
        UsedParam[match[0]] = true
      end
      check_member_usage($_)
      if (state != :none) then
        if ($_ =~ /^\}/) then
          genlines_end = [ GenLines.size, line_control(1, ARGF.file.lineno+1), line_control(ARGF.file.lineno+1) ]
          state = :none
        end
      end
    end
  end

  Hierarchical = Def.empty?
  if Hierarchical then
    explicit_impl_bind == 2  or fail "empty implementation for hierarchical module"
  else
    Input.each do |desc|
      Def.include?(desc.name)  or fail "no definition for input signal #{desc.name}"
    end
  end

  if (Opt_header) then
    startix = 0
  else
    startix = 1
    startix += 1  while (GenLines[startix] =~ /^\s*$/)
    print(GenLines[0].sub(/^\#line (\d+)/){"#line "+($1.to_i+startix-1).to_s})
  end
  print(GenLines[startix..genlines_impl-1])
  if Hierarchical then
    # collect submodule instances
    ix = genlines_impl
    str = ""
    while (GenLines[ix] !~ /^\}/) do
      str += GenLines[ix].chomp.sub(/\s*\/\/.*$/," ")
      ix += 1
    end
    Instances = []
    str.split(";").each do |s|
      s =~ /([a-zA-Z_]\w*\s*,\s*)*[a-zA-Z_]\w*\s*$/  or fail "curious instance list in hierarchical module"
      s0 = $&
      Instances.concat(s0.gsub(/\s+/,"").split(","))
    end
  else
    Input.each do |desc|
      print("  ::streamflowc::port<#{desc.gtype}> streamflowc_port_#{desc.name};\n")
    end
    Output.each do |desc|
      print("  #{desc.atype} #{desc.name}_out;\n")
    end
    Param.each do |desc|
      next unless UsedParam.include?(desc.name)
      print("  #{desc.type} m_#{desc.name};\n")
    end
  end
  initlist = []
  if (UsedMember[:id]) then
    print("  static int streamflowc_nr_instance;\n")
    print("  const int streamflowc_id;\n")
    initlist.push("streamflowc_id(streamflowc_nr_instance++)")
  end
  if (UsedMember[:basename]) then
    print("  const char * const streamflowc_basename;\n")
    initlist.push("streamflowc_basename(::streamflowc::simcontext::get_current_basename())")
  end
  if (UsedMember[:name]) then
    print("  const char * const streamflowc_name;\n")      
    initlist.push("streamflowc_name(::streamflowc::simcontext::get_current_name())")
  end
  initlist.concat(Instances.map{|x| x+'("'+x+'")'})  if (Hierarchical)
  if (initlist.empty?) then
    # assert(! Hierarchical)
    print("  impl_t() {\n")
  else
    suffix = Hierarchical ? " {}\n" : " {\n"
    print(format_array("  impl_t() : ", initlist, ",", 100, suffix))
  end
  unless Hierarchical then
    Input.each do |desc|
      print("    streamflowc_port_#{desc.name}.m_proc = streamflowc_proc_#{desc.name};\n")
    end
    print("  }\n")
    Input.each do |desc|
      print("  static void streamflowc_proc_#{desc.name}(void *p, #{desc.atype} arg);\n")
    end
    Input.each do |desc|
      print("  ", Inline_str.chomp, " void streamflowc_mproc_#{desc.name}(#{desc.atype} #{desc.name});\n")
    end
  end
  print("  ", Inline_str, bind_decl(2, "", :impl), ";\n")
  print(GenLines[genlines_impl..genlines_bind-1])
  if (PRINT_INSTANCE_NAME_UPON_BINDING_ERROR) then
    print("  ::streamflowc::module_name streamflowc_module_name(streamflowc_basename);\n")
  end
  if Hierarchical then
    Output.each do |desc|
      print("  #{desc.atype}& #{desc.name} = streamflowc_parent->#{desc.name};\n")
    end
  else
    Param.each do |desc|
      next unless UsedParam.include?(desc.name)
      print("  m_#{desc.name} = #{desc.name};\n")
    end
    Input.each do |desc|
      print("  #{desc.name} = streamflowc_port_#{desc.name};\n")
    end
  end
  print(GenLines[genlines_bind..genlines_end[0]-1])
  print(genlines_end[1])
  print("\n")
  # Constructor
  print(Template_desc, "#{Modname}#{Template_suffix}::#{Modname}(const ::streamflowc::module_name&) {\n")
  print("  m_impl = new impl_t;\n")
  unless Hierarchical then
    Output.each do |desc|
      print("  #{desc.name} = m_impl->#{desc.name}_out;\n")
    end
  end
  print("}\n\n")
  if (UsedMember[:id]) then
    print(Template_desc, "int #{Modname}#{Template_suffix}::impl_t::streamflowc_nr_instance = 0;\n\n")
  end
  # Descructor
  print(Template_desc, "#{Modname}#{Template_suffix}::~#{Modname}() { delete m_impl; }\n\n")
  # Binder
  print(Template_desc, bind_decl(0, "#{Modname}#{Template_suffix}::"), " {\n")
  prefix = "  (*m_impl)("
  arr = ["this"] + Param.map{|desc| desc.name} + Input.map{|desc| desc.name}
  suffix = ");\n}\n\n"
  print(format_array(prefix, arr, ",", 100, suffix))
  # input proc
  unless Hierarchical then
    Input.each_with_index do |desc,ix|
      print(Template_desc)
      print("void #{Modname}#{Template_suffix}::impl_t::streamflowc_proc_#{desc.name}(void *p, #{desc.atype} arg) {\n")
      print("  __SIZE_TYPE__ addr = reinterpret_cast<__SIZE_TYPE__>(p)-sizeof(__SIZE_TYPE__)*#{ix};\n")
      print("  reinterpret_cast<#{Typename_str}#{Modname}#{Template_suffix}::impl_t*>(addr)",
            "->streamflowc_mproc_#{desc.name}(arg);\n")
      print("}\n\n")
    end
  end
  print(genlines_end[2])
  print(GenLines[genlines_end[0]..-1])

rescue RuntimeError
  if (ARGF.closed?) then
    STDERR.print("Error: #{$!}.\n")
  else
    STDERR.print("#{ARGF.filename}:#{ARGF.file.lineno}: #{$!}.\n")
  end
  exit(2)
end
