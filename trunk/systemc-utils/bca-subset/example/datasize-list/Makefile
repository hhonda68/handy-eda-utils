.SUFFIXES:

ORG_SYSTEMC := $(HOME)/local/systemc-2.2.0
BCA_SYSTEMC := $(abspath ../..)
ORG_LIBSYSTEMC := $(ORG_SYSTEMC)/lib-linux/libsystemc.a
BCA_LIBSYSTEMC := $(BCA_SYSTEMC)/objdir/libsystemc.a

ORG_CXX := g++ -isystem $(ORG_SYSTEMC)/include
BCA_CXX := g++ -isystem $(BCA_SYSTEMC)/src
CXXFLAGS := -O2 -fomit-frame-pointer -Wall
#CXXFLAGS := -g -Wall

all: a.out.org a.out.bca
	@

a.out.org: main.cc $(ORG_LIBSYSTEMC)
	$(ORG_CXX) $(CXXFLAGS) -o $@ $^

a.out.bca: main.cc $(BCA_LIBSYSTEMC)
	$(BCA_CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -rf a.out* *~
