#
# Lightweight restricted-width integer library for C++.
#
# Copyright (c) 2008 the handy-eda-utils developer(s).
# Distributed under the MIT License.
# (See accompanying file COPYING or copy at
#  http://www.opensource.org/licenses/mit-license.php.)

.SUFFIXES:

CXX := g++
CXXFLAGS := -O2 -Wall

all: fx.o
	@

%.o: %.cc
	@echo '[Compiling $< ...]'
	@$(CXX) $(CXXFLAGS) -c $<

clean:
	@echo '[Cleaning ...]'
	@-rm -f *.o *~

fx.o: fx.h
