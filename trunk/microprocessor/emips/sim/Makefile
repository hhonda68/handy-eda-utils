#
# EMIPS : Embedding MIPS Microprocessor
#
# Copyright (c) 2012 the handy-eda-utils developer(s).
# Distributed under the MIT License.
# (See accompanying file COPYING or copy at
#  http://www.opensource.org/licenses/mit-license.php.)

RUBY := ruby
TSV2V := $(RUBY) ../../../verilog-utils/tsv2v/tsv2v.rb

TSV := $(wildcard *.tsv)
V := $(patsubst %.tsv,%.v,$(TSV))

EMIPS_AW := 16

#PROG := hello
#PROG := pi
#PROG := atan
PROG := muldiv

all: tb.vvp $(PROG).dis $(PROG).adr emips-mem0.hex
	@

tb.vvp: rtl tb.v $(V)
	iverilog -DEMIPS_RTLSIM_XXX -y ../rtl -o tb.vvp tb.v

$(V): %.v: %.tsv
	@echo '[Generating $@ ...]'
	@$(TSV2V) $< > $@~ && chmod -w $@~ && rm -f $@ && mv $@~ $@

CCPFX   := mips-elf
CC      := $(CCPFX)-gcc -mno-check-zero-division -msoft-float
CFLAGS  := -O2 -Wall -fomit-frame-pointer
LDFLAGS := -nostdlib -T ldscript.txt
LDLIBS  := -lgcc

define LINK
	$(CC) $(LDFLAGS) -o $@ $(filter %.o,$^) $(LDLIBS)
endef

$(PROG).elf: boot.o $(PROG).o ldscript.txt
	$(LINK)

emips-mem0.hex: $(PROG)-mem0.hex Makefile
	@echo '[Copying $(PROG)-mem*.hex to emips-mem*.hex ...]'
	@cp $(PROG)-mem3.hex emips-mem3.hex && \
	 cp $(PROG)-mem2.hex emips-mem2.hex && \
	 cp $(PROG)-mem1.hex emips-mem1.hex && \
	 cp $(PROG)-mem0.hex emips-mem0.hex

%-mem0.hex: %-mem.bin
	@echo '[Generating $@ ...]'
	@sz=`stat -c "%s" $<` && cap=$$((1<<$(EMIPS_AW))) && \
	 if [ "$$((sz>cap))" = "1" ] ; then \
	   echo "memory size overflow" ; false ; \
	 else \
	   head -c $$((cap-sz)) /dev/zero | cat $< - | od -v -A n -t x1 -w4 > $@~ && \
	   sed 's/^ \(..\) .. .. ../\1/' $@~ > $@~0~ && \
	   sed 's/^ .. \(..\) .. ../\1/' $@~ > $@~1~ && \
	   sed 's/^ .. .. \(..\) ../\1/' $@~ > $@~2~ && \
	   sed 's/^ .. .. .. \(..\)/\1/' $@~ > $@~3~ && \
	   pfx=$@ && pfx=$${pfx%0.hex} && \
	   rm $@~ && mv $@~3~ $${pfx}3.hex && mv $@~2~ $${pfx}2.hex && mv $@~1~ $${pfx}1.hex && mv $@~0~ $${pfx}0.hex ; \
	 fi

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.dis: %.elf
	$(CCPFX)-objdump -d $< > $@~ && $(RUBY) checkinsn.rb $@~ && mv $@~ $@ 

%.adr: %.elf
	$(CCPFX)-nm -n $< > $@~ && mv $@~ $@

%-mem.bin: %.elf
	$(CCPFX)-objcopy -O binary $< $@

run: tb.vvp
	vvp tb.vvp

wave:
	gtkwave tb.vcd tb.wav

rtl:
	cd ../rtl && $(MAKE)

clean:
	rm -f *.o *.elf *.dis *.adr *.bin *.hex *.vvp
