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

EMIPS_IAW := 12
EMIPS_DAW := 16

#PROG := hello
#PROG := pi
PROG := atan

all: tb.vvp $(PROG).dis $(PROG).adr emips-imem.hex emips-dmem0.hex
	@

tb.vvp: rtl tb.v $(V)
	iverilog -y ../rtl -o tb.vvp tb.v

$(V): %.v: %.tsv
	@echo '[Generating $@ ...]'
	@$(TSV2V) $< > $@~ && chmod -w $@~ && rm -f $@ && mv $@~ $@

CCPFX   := mips-elf
CC      := $(CCPFX)-gcc -mcode-readable=no -mno-check-zero-division -msoft-float
CFLAGS  := -O2 -Wall -fomit-frame-pointer
LDFLAGS := -nostdlib -T ldscript.txt
LDLIBS  := -lgcc

define LINK
	$(CC) $(LDFLAGS) -o $@ $(filter %.o,$^) $(LDLIBS)
endef

$(PROG).elf: boot.o $(PROG).o ldscript.txt
	$(LINK)

emips-imem.hex: $(PROG)-imem.hex Makefile
	@echo '[Copying $< to $@ ...]'
	@cp $< $@

emips-dmem0.hex: $(PROG)-dmem0.hex Makefile
	@echo '[Copying $(PROG)-dmem*.hex to emips-dmem*.hex ...]'
	@cp $(PROG)-dmem3.hex emips-dmem3.hex && \
	 cp $(PROG)-dmem2.hex emips-dmem2.hex && \
	 cp $(PROG)-dmem1.hex emips-dmem1.hex && \
	 cp $(PROG)-dmem0.hex emips-dmem0.hex

%-imem.hex: %-imem.bin
	@echo '[Generating $@ ...]'
	@sz=`stat -c "%s" $<` && cap=$$((4<<$(EMIPS_IAW))) && \
	 if [ "$$((sz>cap))" = "1" ] ; then \
	   echo "IMEM size overflow" ; false ; \
	 else \
	   head -c $$((cap-sz)) /dev/zero | cat $< - | od -v -A n -t x1 -w4 | sed 's/ //g' > $@~ && mv $@~ $@ ; \
	 fi

%-dmem0.hex: %-dmem.bin
	@echo '[Generating $@ ...]'
	@sz=`stat -c "%s" $<` && cap=$$((1<<$(EMIPS_DAW))) && \
	 if [ "$$((sz>cap))" = "1" ] ; then \
	   echo "DMEM size overflow" ; false ; \
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
	$(CCPFX)-objdump -d $< > $@~ && mv $@~ $@

%.adr: %.elf
	$(CCPFX)-nm -n $< > $@~ && mv $@~ $@

%-imem.bin: %.elf
	$(CCPFX)-objcopy -j .text -O binary $< $@

%-dmem.bin: %.elf
	$(CCPFX)-objcopy -j .data -O binary $< $@

run: tb.vvp
	vvp tb.vvp

wave:
	gtkwave tb.vcd tb.wav

rtl:
	cd ../rtl && $(MAKE)

clean:
	rm -f *.o *.elf *.dis *.adr *.bin *.hex *.vvp
