.SUFFIXES:

ORG_SYSTEMC := $(HOME)/local/systemc-2.2.0
BCA_SYSTEMC := $(abspath ../..)
ORG_LIBSYSTEMC := $(ORG_SYSTEMC)/lib-linux/libsystemc.a
BCA_LIBSYSTEMC := $(BCA_SYSTEMC)/objdir/libsystemc.a

ORG_CXX := g++ -isystem $(ORG_SYSTEMC)/include
BCA_CXX := g++ -isystem $(BCA_SYSTEMC)/src
CXXFLAGS := -O2 -fomit-frame-pointer -Wall
#CXXFLAGS := -g -Wall

OBJS := main.o fft.o source.o sink.o
ORG_OBJS := $(addprefix obj-org/,$(OBJS))
BCA_OBJS := $(addprefix obj-bca/,$(OBJS))

all: a.out.org a.out.bca
	@

check: check-org check-bca
	@

a.out.org: $(ORG_OBJS) $(ORG_LIBSYSTEMC)
	$(ORG_CXX) $(CXXFLAGS) -o $@ $^

a.out.bca: $(BCA_OBJS) $(BCA_LIBSYSTEMC)
	$(BCA_CXX) $(CXXFLAGS) -o $@ $^

check-%: a.out.%
	@rm -f out_real out_imag
	./a.out.$*
	diff out_real out_real.golden
	diff out_imag out_imag.golden
	@rm -f out_real out_imag

obj-org/%.o: %.cc
	@test -d obj-org || mkdir -p obj-org
	$(ORG_CXX) $(CXXFLAGS) -c -o $@ $<

obj-bca/%.o: %.cc
	@test -d obj-bca || mkdir -p obj-bca
	$(BCA_CXX) $(CXXFLAGS) -c -o $@ $<

obj-bca/%.ss: %.cc
	@test -d obj-bca || mkdir -p obj-bca
	$(BCA_CXX) $(CXXFLAGS) -S -o - $< | c++filt > $@

dep:
	$(ORG_CXX) $(CXXFLAGS) -MM *.cc | sed 's|^\([a-z]\)|obj-org/\1|' > Makefile.dep~
	$(BCA_CXX) $(CXXFLAGS) -M  *.cc | sed 's|^\([a-z]\)|obj-bca/\1|' >> Makefile.dep~
	@mv Makefile.dep~ Makefile.dep

-include Makefile.dep

clean:
	rm -rf obj-org obj-bca a.out* *~ out_real out_imag
