#
# Lightweight functional digital logic simulator library.
#

# Copyright (c) 2008 the handy-eda-utils develeper(s).
# Distributed under the MIT License.
# (See accompanying file COPYING or copy at
#  http://www.opensource.org/licenses/mit-license.php.)

# Last modified on October 2005.

.SUFFIXES:

CXX := g++
CXXS := $(CXX)
CXXM := $(CXX) -DCYCLEC_MULTICLOCK
CXXFLAGS := -O2 -Wall
#CXXFLAGS := -g
LDFLAGS :=
LDLIBS :=

SINGLECLOCK_PROGRAMS := sample-nand sample-counter
MULTICLOCK_PROGRAMS := sample-dualclock

define LINK
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
endef

all:	$(SINGLECLOCK_PROGRAMS) $(MULTICLOCK_PROGRAMS)
	@

cyclec-singleclock.o: cyclec.cc
	$(CXXS) $(CXXFLAGS) -c -o $@ $<

cyclec-multiclock.o: cyclec.cc
	$(CXXM) $(CXXFLAGS) -c -o $@ $<

$(addsuffix .o,$(SINGLECLOCK_PROGRAMS)): %.o: %.cc
	$(CXXS) $(CXXFLAGS) -c $*.cc

$(addsuffix .o,$(MULTICLOCK_PROGRAMS)): %.o: %.cc
	$(CXXM) $(CXXFLAGS) -c $*.cc

$(SINGLECLOCK_PROGRAMS): %: %.o cyclec-singleclock.o
	$(LINK)

$(MULTICLOCK_PROGRAMS): %: %.o cyclec-multiclock.o
	$(LINK)

dep:
	$(CXXS) $(CXXFLAGS) -MM -MT cyclec-singleclock.o cyclec.cc > Makefile.dep~
	$(CXXM) $(CXXFLAGS) -MM -MT cyclec-multiclock.o cyclec.cc >> Makefile.dep~
	$(CXXS) $(CXXFLAGS) -MM $(addsuffix .cc,$(SINGLECLOCK_PROGRAMS)) >> Makefile.dep~
	$(CXXM) $(CXXFLAGS) -MM $(addsuffix .cc,$(MULTICLOCK_PROGRAMS)) >> Makefile.dep~
	mv Makefile.dep~ Makefile.dep

clean:
	rm -f *.o sample-nand sample-counter sample-dualclock

-include Makefile.dep

