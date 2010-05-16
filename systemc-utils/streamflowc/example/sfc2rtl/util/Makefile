#
# Utility modules for StreamFlowC-to-RTL conversion
#
# Copyright (c) 2010 the handy-eda-utils developer(s).
# Distributed under the MIT License.
# (See accompanying file COPYING or copy at
#  http://www.opensource.org/licenses/mit-license.php.)
#
RUBY := ruby
TSV2V := $(RUBY) ../../../../../verilog-utils/tsv2v/tsv2v.rb

TSV := $(wildcard *.tsv)
V := $(patsubst %.tsv,%.v,$(TSV))

all: $(V)
	@

$(V): %.v: %.tsv
	@echo '[Generating $@ ...]'
	@$(TSV2V) $< > $@~ && chmod -w $@~ && rm -f $@ && mv $@~ $@
