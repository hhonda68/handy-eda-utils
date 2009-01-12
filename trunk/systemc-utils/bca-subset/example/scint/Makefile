.SUFFIXES:

ORG_SYSTEMC := $(HOME)/local/systemc-2.2.0
BCA_SYSTEMC := $(abspath ../..)
ORG_INC := $(ORG_SYSTEMC)/include
BCA_INC := $(BCA_SYSTEMC)/src

CXX := g++
CXXFLAGS := -O2 -Wall -fomit-frame-pointer -fno-toplevel-reorder -fno-align-functions

all:	sample.bca.m32.dis sample.bca.m64.dis sample.org.m32.dis sample.org.m64.dis
	@

sample.bca.%.dis: sample.cc $(BCA_INC)/sysc/datatypes/int/sc_int.h
	$(CXX) -$* -isystem $(BCA_INC) $(CXXFLAGS) -c sample.cc
	objdump -d sample.o | c++filt | sed 's/^[0-9a-f][0-9a-f]* //;s/^ *[0-9a-f][0-9a-f]*:	//' > $@

sample.org.%.dis: sample.cc
	$(CXX) -$* -isystem $(ORG_INC) $(CXXFLAGS) -DORG_SYSTEMC -c sample.cc
	objdump -d sample.o | c++filt | sed 's/^[0-9a-f][0-9a-f]* //;s/^ *[0-9a-f][0-9a-f]*:	//' > $@

clean:
	rm -f *.dis *.o

