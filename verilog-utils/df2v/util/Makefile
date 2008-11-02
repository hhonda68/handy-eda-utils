#
# FDFL (Fixed-point dataflow language) utilities
#
# Copyright (c) 2008 the handy-eda-utils develeper(s).
# Distributed under the MIT License.
# (See accompanying file COPYING or copy at
#  http://www.opensource.org/licenses/mit-license.php.)
#
# Last modified on December 2007
#

.SUFFIXES:
RUBY := ruby

MODULE := myblock
DFISRCS := foo bar
VSRCS := hoge fuga

all: mid/.stamp
	@

$(MODULE).dfi: $(foreach n,$(DFISRCS),$n.dfi) Makefile
	@echo '[Generating $@ ...]'
	@rm -f $@ && cat $(filter-out Makefile,$^) > $@~ && chmod -w $@~ && mv $@~ $@

$(MODULE)-misc.v: $(foreach n,$(VSRCS),$n.v) Makefile
	@echo '[Generating $@ ...]'
	@rm -f $@ && cat $(filter-out Makefile,$^) > $@~ && chmod -w $@~ && mv $@~ $@

%.df:	%.dfi dfi2df.rb
	@echo '[Generating $@ ...]'
	@rm -f $@ && $(RUBY) dfi2df.rb $< > $@~ && chmod -w $@~ && mv $@~ $@

%.v:	%.df df2v.rb
	@echo '[Generating $@ ...]'
	@$(RUBY) df2v.rb $< > $@~ && mv $@~ $@

leaf/.stamp-%: %.v modulesplit.rb
	@test -d leaf || mkdir leaf
	@echo '[Splitting $< ...]'
	@$(RUBY) modulesplit.rb $< leaf
	@echo 'Done.' > $@~ && mv $@~ $@

mid/.dep-$(MODULE): $(MODULE).v moduledepend.rb
	@test -d mid || mkdir mid
	@echo '[Generating $@ ...]'
	@$(RUBY) moduledepend.rb --fdfl $< > $@~ && mv $@~ $@

mid/.dep-$(MODULE)-misc: $(MODULE)-misc.v moduledepend.rb
	@test -d mid || mkdir mid
	@echo '[Generating $@ ...]'
	@$(RUBY) moduledepend.rb --verilog $(MODULE)_ $< > $@~ && mv $@~ $@

mid/.dep: mid/.dep-$(MODULE)-misc mid/.dep-$(MODULE) moduledepend.rb
	@echo '[Generating $@ ...]'
	@$(RUBY) moduledepend.rb --flatten $(filter mid/%,$^) > $@~ && mv $@~ $@

mid/.stamp: leaf/.stamp-$(MODULE) leaf/.stamp-$(MODULE)-misc mid/.dep moduledepend.rb
	@$(RUBY) moduledepend.rb --genmid leaf mid mid/.dep
	@echo 'Done.' > $@~ && mv $@~ $@

clean:
	@echo '[Cleaning ...]'
	@rm -rf $(MODULE).dfi $(MODULE).df $(MODULE).v $(MODULE)-misc.v leaf mid

