.SUFFIXES:

ORG_SYSTEMC := $(HOME)/local/systemc-2.2.0
BCA_SYSTEMC := $(abspath ../..)
ORG_LIBSYSTEMC := $(ORG_SYSTEMC)/lib-linux/libsystemc.a
BCA_LIBSYSTEMC := $(BCA_SYSTEMC)/objdir/libsystemc.a

ORG_CXX := g++ -isystem $(ORG_SYSTEMC)/include
BCA_CXX := g++ -isystem $(BCA_SYSTEMC)/src
CXXFLAGS := -O2 -fomit-frame-pointer -Wall
#CXXFLAGS := -g -Wall

COMMON_OBJS := stimulus.o display.o
BEH_OBJS := main.o fir.o $(COMMON_OBJS)
RTL_OBJS := main_rtl.o fir_fsm.o fir_data.o $(COMMON_OBJS)

ORG_BEH_OBJS := $(addprefix obj-org/,$(BEH_OBJS))
ORG_RTL_OBJS := $(addprefix obj-org/,$(RTL_OBJS))
BCA_BEH_OBJS := $(addprefix obj-bca/,$(BEH_OBJS))
BCA_RTL_OBJS := $(addprefix obj-bca/,$(RTL_OBJS))

all: a.out.org-beh a.out.bca-beh a.out.org-rtl a.out.bca-rtl
	@

check: check-org-beh check-bca-beh check-org-rtl check-bca-rtl
	@

check-%: a.out.% log-%.golden
	./a.out.$* > log-$*
	diff log-$* log-$*.golden

log-bca-beh.golden: log-org-beh.golden
	@echo '[Generating $@ ...]'
	@sed 's/ ns$$/ns/;s/ us$$/000ns/;$$d' $< > $@

log-bca-rtl.golden: log-org-rtl.golden
	@echo '[Generating $@ ...]'
	@sed '1{p;p;p;p;p;p};s/ ns$$/ns/;s/ us$$/000ns/;$$d' $< > $@

a.out.org-beh: $(ORG_BEH_OBJS) $(ORG_LIBSYSTEMC)
	$(ORG_CXX) $(CXXFLAGS) -o $@ $^

a.out.org-rtl: $(ORG_RTL_OBJS) $(ORG_LIBSYSTEMC)
	$(ORG_CXX) $(CXXFLAGS) -o $@ $^

a.out.bca-beh: $(BCA_BEH_OBJS) $(BCA_LIBSYSTEMC)
	$(BCA_CXX) $(CXXFLAGS) -o $@ $^

a.out.bca-rtl: $(BCA_RTL_OBJS) $(BCA_LIBSYSTEMC)
	$(BCA_CXX) $(CXXFLAGS) -o $@ $^

obj-org/%.o: %.cc
	@test -d obj-org || mkdir -p obj-org
	$(ORG_CXX) $(CXXFLAGS) -c -o $@ $<

obj-bca/%.o: %.cc
	@test -d obj-bca || mkdir -p obj-bca
	$(BCA_CXX) $(CXXFLAGS) -c -o $@ $<

dep:
	$(ORG_CXX) $(CXXFLAGS) -MM *.cc | sed 's|^\([a-z]\)|obj-org/\1|' > Makefile.dep~
	$(BCA_CXX) $(CXXFLAGS) -M  *.cc | sed 's|^\([a-z]\)|obj-bca/\1|' >> Makefile.dep~
	@mv Makefile.dep~ Makefile.dep

-include Makefile.dep

clean:
	rm -rf obj-org obj-bca a.out* *~ log-bca-*.golden
