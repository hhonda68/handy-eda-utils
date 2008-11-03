#!/usr/bin/ruby
#
# Wrapper for pimpl idiom on hierachical modules
#
# Copyright (c) 2008 the handy-eda-utils develeper(s).
# Distributed under the MIT License.
# (See accompanying file COPYING or copy at
#  http://www.opensource.org/licenses/mit-license.php.)
#
################################################################
#
# ruby genhiermod.rb -h   foo.h.in   > foo.h
# ruby genhiermod.rb -cc  foo.h.in   > foo.cc
# ruby genhiermod.rb -icc foo.icc.in > foo.icc

SWITCHER_MACRO = "INLINE_HIERARCHICAL_MODULE"

def gen_header
  mod = nil
  lmod = nil
  while (gets) do
    if ($_ =~ /^SC_MODULE\((\w+)\)/) then
      mod = $1
      lmod = mod.downcase
      print((<<-EOD).gsub(/^      /,""))
      #ifdef #{SWITCHER_MACRO}
      #include "#{lmod}.icc"
      #endif

      EOD
    elsif (mod && $_ == "};\n") then
      print((<<-EOD).gsub(/^      /,""))
      #ifdef #{SWITCHER_MACRO}
      #include "#{lmod}.icc"
      #else
        SC_CTOR(#{mod}) { construct(); }
        ~#{mod}();
      private:
        struct impl_t;
        impl_t *m_impl;
        void construct();
      #endif
      EOD
    end
    print
  end
end

def gen_cc
  while (gets) do
    break if ($_ =~ /^SC_MODULE\((\w+)\)/)
  end
  mod = $1
  umod = mod.upcase
  lmod = mod.downcase
  print((<<-EOD).gsub(/^  /,""));
  #include "#{lmod}.h"
  #define #{umod}_CC
  #include "#{lmod}.icc"
  EOD
end

def split_to_sections(lines)
  delimiter = "////////////////////////////////////////////////////////////////\n"
  sections = []
  ix = 0
  while true do
    ix_start = ix
    ix += 1  while (ix < lines.size && lines[ix] != delimiter)
    ix_end = ix-1
    ix_start += 1  while (lines[ix_start] =~ /^\s*$/)
    ix_end   -= 1  while (lines[ix_end] =~ /^\s*$/)
    sections.push([ix_start, ix_end])
    break if (ix >= lines.size)
    ix += 1
  end
  return sections
end

def gen_icc
  lines = ARGF.readlines
  sections = split_to_sections(lines)
  (sections.size >= 4)  or fail
  mod = nil
  umod = nil
  lmod = nil
  prefix = ""
  dtor_body = ""
  (sections[3][0]..sections[3][1]).each do |ix|
    if (lines[ix] =~ /^\s*(m_impl->\w+)\s*=\s*new\b/) then
      dtor_body += "  delete #{$1};\n"
    elsif (lines[ix] !~ /^\s*$/) then
      break
    end
  end
  ix = 0
  while (ix < lines.size) do
    str_pre = nil
    str_post = nil
    if ((sections[0][0]..sections[0][1]) === ix && (lines[ix] =~ /^\/\/SC_MODULE\((\w+)\)/)) then
      mod = $1
      umod = mod.upcase
      lmod = mod.downcase
      lines[ix] = <<-EOD.gsub(/^      /,"")
      #ifdef #{SWITCHER_MACRO}
      # ifndef #{umod}_CC
      #  ifndef #{umod}_INLINED_HDR
      #   define #{umod}_INLINED_HDR
      #  else
      #   undef #{umod}_INLINED_HDR
      #   define #{umod}_INLINED_IMPL
      #  endif
      # else
      #  define #{umod}_INLINED_CC
      # endif
      #else
      # ifndef #{umod}_CC
      #  error "#{lmod}.icc included from #{lmod}.h under pimpl style???"
      # else
      #  define #{umod}_PIMPL_CC
      # endif
      #endif
      EOD
    end
    if (ix == sections[1][0]) then
      str_pre = "#if defined(#{umod}_INLINED_HDR) || defined(#{umod}_PIMPL_CC)\n"
    end
    if (ix == sections[1][1]) then
      str_post = "#endif\n"
    end
    if (ix == sections[2][0]) then
      prefix = "  "
      str_pre = <<-EOD.gsub(/^      /,"")
      #ifdef #{umod}_PIMPL_CC
      struct #{mod}::impl_t {
      #endif
      #if defined(#{umod}_INLINED_IMPL) || defined(#{umod}_PIMPL_CC)
      EOD
    end
    if (ix == sections[2][1]) then
      str_post = <<-EOD.gsub(/^      /,"")
      #endif
      #ifdef #{umod}_PIMPL_CC
      };
      #endif
      EOD
    end
    if (ix == sections[3][0]) then
      prefix = "  "
      str_pre = <<-EOD.gsub(/^      /,"")
      #if defined(#{umod}_PIMPL_CC) || defined(#{umod}_INLINED_IMPL)
      #ifdef #{umod}_PIMPL_CC
      void #{mod}::construct()
      #else
      SC_CTOR(#{mod})
      #endif
      {
      #ifdef #{umod}_PIMPL_CC
        m_impl = new impl_t;
      #else
        #{mod} *m_impl = this;
      #endif
      EOD
    end
    if (ix == sections[3][1]) then
      str_post = <<-EOD.gsub(/^      /,"")
      }
      #endif

      ////////////////////////////////////////////////////////////////

      #if defined(#{umod}_PIMPL_CC) || defined(#{umod}_INLINED_IMPL)
      #ifdef #{umod}_PIMPL_CC
      #{mod}::
      #endif
      ~#{mod}() {
      #ifdef #{umod}_INLINED_IMPL
        #{mod} *m_impl = this;
      #endif
      EOD
      str_post += dtor_body + <<-EOD.gsub(/^      /,"")
      #ifdef #{umod}_PIMPL_CC
        delete m_impl;
      #endif
      }
      #endif

      ////////////////////////////////////////////////////////////////

      #undef #{umod}_PIMPL_CC
      #undef #{umod}_INLINED_IMPL
      // do not undef #{umod}_INLINED_HDR
      EOD
    end
    if (str_pre) then
      print(str_pre)
    end
    print(prefix, lines[ix])
    if (str_post) then
      print(str_post)
      prefix = ""
    end
    ix += 1
  end
end

################################################################

ARGV.size >= 1  or fail
opt = ARGV.shift
case opt
when "-h"
  gen_header
when "-cc"
  gen_cc
when "-icc"
  gen_icc
else
  fail
end

