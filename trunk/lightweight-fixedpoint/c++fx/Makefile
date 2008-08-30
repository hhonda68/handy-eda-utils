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
