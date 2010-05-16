RUBY := ruby
TSV2V := $(RUBY) ../../../../../verilog-utils/tsv2v/tsv2v.rb

TSV := $(wildcard *.tsv)
V := $(patsubst %.tsv,%.v,$(TSV))

all: tb.vvp
	@

tb.vvp: tb.v $(V)
	iverilog -y . -y ../util -o tb.vvp tb.v

$(V): %.v: %.tsv
	@echo '[Generating $@ ...]'
	@$(TSV2V) $< > $@~ && chmod -w $@~ && rm -f $@ && mv $@~ $@

VARIATION := 0 1 2 3
CHECK_VARIATION := $(addprefix check,$(VARIATION))

$(CHECK_VARIATION): check%: tb.vvp
	@chkopt="" && \
	 if [ "$$(($*%2))" != "0" ] ; then chkopt="$$chkopt +pace_ivld" ; fi && \
	 if [ "$$(($*/2))" != "0" ] ; then chkopt="$$chkopt +pace_ordy" ; fi && \
	 cmd="vvp tb.vvp $$chkopt" && \
	 echo "$$cmd > log$*.txt" && $$cmd > log$*.txt && mv tb.vcd tb$*.vcd
	@echo '[Verifying log data ...]'
	@sed -n '/ d800/q;/ d[0-9]/{s/.*: //;p}' log$*.txt > log$*-d.txt
	@sed -n '/ avg/{s/.*: //;p}' log$*.txt > log$*-a.txt
	@grep '^d' ../avgvar2/log.txt > log-d-ref.txt
	@grep '^avg' ../avgvar2/log.txt > log-a-ref.txt
	@diff log-d-ref.txt log$*-d.txt > /dev/null || { echo "Verify Error on idata" ; exit 2 ; }
	@diff log-a-ref.txt log$*-a.txt > /dev/null || { echo "Verify Error on odata" ; exit 2 ; }

check: $(CHECK_VARIATION)
	@
