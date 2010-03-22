#
# StreamFlowC : A C++ coding style to describe hardware structure for stream processing
#
# Copyright (c) 2009 the handy-eda-utils developer(s).
# Distributed under the MIT License.
# (See accompanying file COPYING or copy at
#  http://www.opensource.org/licenses/mit-license.php.)
#

.SUFFIXES:

RUBY := ruby
SFC2CC := ../tool/sfc2cc.rb
CC := g++
CFLAGS := -O2 -Wall -fomit-frame-pointer
AR := ar cru

SFC := fork trig fbbuf buf headcut
SFC_H := $(addsuffix .h,$(SFC))

OBJS := streamflowc.o
LIBFILE := libstreamflowc.a
HDRFILE := streamflowc.h sfcut.h

LIBTARGETS := $(addprefix ../lib/,$(LIBFILE))
HDRTARGETS := $(addprefix ../include/,$(HDRFILE))

all: $(LIBTARGETS) $(HDRTARGETS)
	@

$(LIBTARGETS): ../lib/%: %
	@echo '[Copying $* ...]'
	@mkdir -p ../lib && rm -f $@ && cp $< $@~ && chmod -w $@~ && mv $@~ $@

$(HDRTARGETS): ../include/%: %
	@echo '[Copying $* ...]'
	@mkdir -p ../include && rm -f $@ && cp $< $@~ && chmod -w $@~ && mv $@~ $@

$(LIBFILE): $(OBJS)
	@echo '[Archiving $@ ...]'
	@rm -f $@ && $(AR) $@ $^

sfcut.h: $(SFC_H) Makefile streamflowc.h
	@echo '[Generating $@ ...]'
	@{ sed -n '/^#/q;p' streamflowc.h && \
	   echo '#ifndef SFCUT_H' && \
	   echo '#define SFCUT_H' && \
	   echo '' && \
	   echo 'namespace sfcut {' && \
	   echo '' && \
	   echo '// StreamFlowC declarations:' && \
	   for i in $(SFC) ; do sed '1,10d;/^} \/\/ namespace/,/^$$/d;s|^|// |;/^\/\/ };/q' $$i.sfc || exit 2 ; done && \
	   echo '' && \
	   for i in $(SFC) ; do echo '#line 11 "'$$i.sfc'"' && sed '1,11d' $$i.h || exit 2 ; done && \
	   echo '} // namespace sfcut' && \
	   echo '' && \
	   echo '#endif' ; } > $@~ && chmod -w $@~ && rm -f $@ && mv $@~ $@

%.o: %.cc
	@echo '[Compiling $< ...]'
	@$(CXX) $(CXXFLAGS) -c $<

$(SFC_H): %.h: %.sfc $(SFC2CC)
	@echo '[Genetaring $@ ...]'
	@$(RUBY) $(SFC2CC) -h $< > $@~ && chmod -w $@~ && rm -f $@ && mv $@~ $@

clean:
	@echo '[Cleaning ...]'
	@rm -f *~ *.o *.a $(SFC_H) sfcut.h $(LIBTARGETS) $(HDRTARGETS)
