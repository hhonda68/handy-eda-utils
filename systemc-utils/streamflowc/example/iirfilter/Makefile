.SUFFIXES:

RUBY := ruby
SFC2CC := ../../tool/sfc2cc.rb
CXX := g++
CXXFLAGS := -O2 -Wall -fomit-frame-pointer
LDFLAGS :=
LDLIBS :=
LIBOBJS := ../../lib/libstreamflowc.a

SFC    := $(wildcard *.sfc)
SFC_H  := $(patsubst %.sfc,%.h,$(SFC))
SFC_CC := $(patsubst %.sfc,%.cc,$(SFC))

GENFILES := $(SFC_H) $(SFC_CC)

SRCS := $(SFC_CC) main.cc
OBJS := $(patsubst %.cc,%.o,$(SRCS))

all: a.out
	@

run: a.out
	@./a.out

$(LIBOBJS): %.o:
	@

$(SFC_H): %.h: %.sfc $(SFC2CC)
	@echo '[Genetaring $@ ...]'
	@$(RUBY) $(SFC2CC) -h $< > $@~ && chmod -w $@~ && rm -f $@ && mv $@~ $@

$(SFC_CC): %.cc: %.sfc $(SFC2CC)
	@echo '[Genetaring $@ ...]'
	@$(RUBY) $(SFC2CC) -cc $< > $@~ && chmod -w $@~ && rm -f $@ && mv $@~ $@

%.o: %.cc
	@echo '[Compiling $< ...]'
	@$(CXX) $(CXXFLAGS) -c $<

a.out: $(OBJS) $(LIBOBJS)
	@echo '[Linking $@ ...]'
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	@echo '[Cleaning ...]'
	@rm -f a.out *.o *~ $(GENFILES)

dep: $(GENFILES)
	@echo '[Generating Makefile.dep ...]'
	@f=Makefile.dep && $(CXX) $(CXXFLAGS) -MM $(SRCS) > $$f~ && chmod -w $$f~ && rm -f $$f && mv $$f~ $$f

-include Makefile.dep

