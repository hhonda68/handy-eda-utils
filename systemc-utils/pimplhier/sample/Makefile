#
# Wrapper for pimpl idiom on hierachical modules
#
# Copyright (c) 2008 the handy-eda-utils developer(s).
# Distributed under the MIT License.
# (See accompanying file COPYING or copy at
#  http://www.opensource.org/licenses/mit-license.php.)
#
################################################################
#
# export SYSTEMC=/usr/local/systemc-2.2.0
# make INLINE=no dep
# make INLINE=no clean all run 2>&1 | tee log-pimpl.txt
# mv Makefile.dep Makefile.dep.pimpl
# make INLINE=yes dep
# make INLINE=yes clean all run 2>&1 | tee log-inline.txt
# mv Makefile.dep Makefile.dep.inline
# diff log-pimpl.txt log-inline.txt
# diff Makefile.dep.pimpl Makefile.dep.inline

.SUFFIXES:

ifeq ($(INLINE),yes)
CXXFLAGS_INLINE := -DINLINE_HIERARCHICAL_MODULE
else
CXXFLAGS_INLINE :=
endif

CXX := g++ -isystem $(SYSTEMC)/include
CXXFLAGS := -O2 -fomit-frame-pointer -Wall -Werror $(CXXFLAGS_INLINE)
LDFLAGS := -L$(SYSTEMC)/lib-linux
LDLIBS := -lsystemc

RUBY := ruby

SRCS := $(wildcard *.cc)
OBJS := $(patsubst %.cc,%.o,$(SRCS))

define LINK
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
endef

all:	a.out
	@

run:	a.out
	./a.out

a.out: $(OBJS)
	$(LINK)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $<

%.h: %.h.in genhiermod.rb
	rm -f $@ && $(RUBY) genhiermod.rb -h $< > $@~ && chmod -w $@~ && mv $@~ $@

%.cc: %.h.in genhiermod.rb
	rm -f $@ && $(RUBY) genhiermod.rb -cc $< > $@~ && chmod -w $@~ && mv $@~ $@

%.icc: %.icc.in genhiermod.rb
	rm -f $@ && $(RUBY) genhiermod.rb -icc $< > $@~ && chmod -w $@~ && mv $@~ $@

-include Makefile.dep

GENFILES := hiermod.h hiermod.cc hiermod.icc

dep: $(GENFILES)
	$(CXX) $(CXXFLAGS) -MM *.cc > Makefile.dep~ && mv Makefile.dep~ Makefile.dep

clean:
	rm -f a.out *.o *~ $(GENFILES)

