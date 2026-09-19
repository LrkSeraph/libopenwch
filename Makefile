##
## This file is part of the libopenwch project.
##
## Copyright (C) 2025 libopenwch contributors
##
## This library is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this library.  If not, see <http://www.gnu.org/licenses/>.
##

##
## libopenwch top-level build.
##
## Structure and behaviour follow libopencm3's root Makefile: one entry in
## TARGETS per buildable library directory under lib/, each built by a
## recursive make, with failures collected into .stamp_failure_* files so that
## every target is attempted and `make -j` works.
##
## Differences from libopencm3:
##   * TARGETS may be nested (`ch32v/003`), which libopencm3 supports as well
##     (`stm32/f1`), but the IRQ file substitutions below deliberately avoid the
##     '*/' prefix that libopencm3 uses, because it never matches the output of
##     $(wildcard ...) and silently defeats the dependency.
##   * the generated files are nvic.h, vector_handlers.c and vector_names.c.
##

include mk/gcc-config.mk

TARGETS ?=	ch32v0 \
		ch5xx58x

STYLECHECK      := scripts/checkpatch.pl
STYLECHECKFLAGS := --no-tree -f --terse --mailback

# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q := @
# Do not print "Entering directory ...".
MAKEFLAGS += --no-print-directory
endif

##
## Interrupt definition files and the headers they generate
##
## Avoid the use of shell find, for windows compatibility.
##
IRQ_DEFN_FILES  := $(foreach TARGET,$(TARGETS),$(wildcard include/libopenwch/$(TARGET)/irq.json))

all: build

build: lib

##
## One recipe per device generating all of its interrupt files at once.
##
## The substitution deliberately does NOT use libopencm3's './include/...'
## pattern: $(wildcard ...) yields paths without the leading './', so that
## pattern never matches and the generated-file dependency is silently lost.
##
## TARGETS holds library directory names (which are the genlink FAMILY names),
## so the target set is fully known at parse time.
##
IRQ_FILES_ch32v0   := include/libopenwch/ch32v0/nvic.h \
                      lib/ch32v0/vector_handlers.c \
                      lib/ch32v0/vector_names.c
IRQ_FILES_ch5xx58x := include/libopenwch/ch5xx58x/nvic.h \
                      lib/ch5xx58x/vector_handlers.c \
                      lib/ch5xx58x/vector_names.c
IRQ_FILES_ch5xx57x := include/libopenwch/ch5xx57x/nvic.h \
                      lib/ch5xx57x/vector_handlers.c \
                      lib/ch5xx57x/vector_names.c
IRQ_FILES_ch5xx59x := include/libopenwch/ch5xx59x/nvic.h \
                      lib/ch5xx59x/vector_handlers.c \
                      lib/ch5xx59x/vector_names.c
IRQ_FILES_ch32v0v4 := include/libopenwch/ch32v0v4/nvic.h \
                      lib/ch32v0v4/vector_handlers.c \
                      lib/ch32v0v4/vector_names.c
IRQ_FILES_ch32x0   := include/libopenwch/ch32x0/nvic.h \
                      lib/ch32x0/vector_handlers.c \
                      lib/ch32x0/vector_names.c
IRQ_FILES_ch32l1   := include/libopenwch/ch32l1/nvic.h \
                      lib/ch32l1/vector_handlers.c \
                      lib/ch32l1/vector_names.c

IRQ_GENERATED_FILES := $(foreach t,$(TARGETS),$(IRQ_FILES_$(t)))

$(IRQ_GENERATED_FILES) &: $(IRQ_DEFN_FILES) ./scripts/irq2nvic_h
	$(Q)for f in $(IRQ_DEFN_FILES); do \
		./scripts/irq2nvic_h ./$$f || exit $$?; \
	done

# Only irq.json files have generated headers to remove.  Anchoring the
# pattern on the /irq.json suffix keeps it disjoint from the %.clean rule
# below, which would otherwise also match these targets.
%/irq.json.cleanhdr:
	$(Q)./scripts/irq2nvic_h --remove ./$*/irq.json

##
## Library build
##
LIB_DIRS := $(wildcard $(addprefix lib/,$(TARGETS)))

$(LIB_DIRS): $(IRQ_GENERATED_FILES)
	$(Q)$(RM) .stamp_failure_$(subst /,_,$@)
	@printf "  BUILD   $@\n";
	$(Q)$(MAKE) --directory=$@ PREFIX="$(PREFIX)" || \
		echo "Failure building: $@: code: $$?" > .stamp_failure_$(subst /,_,$@)

lib: $(LIB_DIRS)
	$(Q)$(RM) .stamp_failure_tld
	$(Q)for failure in .stamp_failure_*; do \
		[ -f $$failure ] && cat $$failure >> .stamp_failure_tld || true; \
	done;
	$(Q)[ -f .stamp_failure_tld ] && cat .stamp_failure_tld && exit 1 || true;

html doc:
	$(Q)$(MAKE) -C doc html TARGETS="$(TARGETS)"

clean: $(IRQ_DEFN_FILES:=.cleanhdr) $(LIB_DIRS:=.clean) doc.clean styleclean genlinktests.clean
	$(Q)$(RM) .stamp_failure_*

%.clean:
	$(Q)if [ -d $* ]; then \
		printf "  CLEAN   $*\n"; \
		$(MAKE) -C $* clean PREFIX="$(PREFIX)" V="$(V)" || exit $?; \
	fi;

##
## Style checking
##
STYLECHECKFILES := $(wildcard include/*/*.h include/*/*/*.h include/*/*/*/*.h)
STYLECHECKFILES += $(wildcard lib/*/*.h lib/*/*/*.h lib/*/*/*/*.h)
STYLECHECKFILES += $(wildcard lib/*/*.c lib/*/*/*.c lib/*/*/*/*.c)

stylecheck: $(STYLECHECKFILES:=.stylecheck)
styleclean: $(STYLECHECKFILES:=.styleclean)

# the cat is due to multithreaded nature - we like to have consistent chunks of text on the output
%.stylecheck: %
	$(Q)if ! grep -q "* It was generated by the irq2nvic_h script." $* ; then \
		$(STYLECHECK) $(STYLECHECKFLAGS) $* > $*.stylecheck; \
		if [ -s $*.stylecheck ]; then \
			cat $*.stylecheck; \
		else \
			rm -f $*.stylecheck; \
		fi; \
	fi;

%.styleclean:
	$(Q)$(RM) $*.stylecheck;

##
## Linker script generation smoke tests
##
LDTESTS := $(wildcard ld/tests/*.data)

genlinktests: $(LDTESTS:.data=.ldtest)
genlinktests.clean:
	$(Q)$(RM) $(LDTESTS:.data=.out)

%.ldtest:
	@if ./scripts/genlinktest.sh $* >/dev/null 2>&1; then \
		printf "  TEST  OK  : $*\n"; \
	else \
		printf "  TEST FAIL : $*\n"; \
	fi;

list-targets:
	@echo $(TARGETS)

.PHONY: build lib $(LIB_DIRS) doc html clean generatedheaders cleanheaders \
	stylecheck styleclean genlinktests genlinktests.clean list-targets
