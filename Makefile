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

##
## checkpatch.pl comes from the Linux kernel, so a few of its checks are
## wrong for this project.  The ignored types are all cases where the code is
## deliberate and checkpatch does not know it:
##
##   VOLATILE        MMIO register access is the library's whole purpose.
##   NEW_TYPEDEFS    `foo_t` names are part of the documented API style.
##   CAMELCASE       register and field names mirror the WCH reference manual,
##                   which the project requires (see AGENTS.md).
##   COMPLEX_MACRO   the CSR helpers are `__asm__ volatile` statements; there
##                   is no expression to parenthesise.
##   SPACING         the `:::` clobber list of an asm statement reads to
##                   checkpatch as a malformed ternary.
##   AVOID_EXTERNS   linker-provided symbols (_data_lma and friends) have no
##                   header to be declared in.
##   STORAGE_CLASS   macros that combine an attribute with a storage class.
##   BRACES          the `do { } while (0)` and `while (1) { ; }` macro and
##                   busy-wait idioms.
##
## The last two exist because clang-format owns formatting here (see
## .githooks/pre-commit) and the two tools genuinely disagree: clang-format
## pads continuation lines and aligned escapes with spaces, which is the only
## way to align to a column at all, while the kernel rule demands tabs.  When
## the formatter and the checker disagree, the formatter wins -- that is the
## whole point of having one.
##
##   LEADING_SPACE          clang-format alignment padding.
##   SUSPECT_CODE_INDENT    ditto, for a macro body broken across lines.
##
STYLECHECKIGNORE := VOLATILE,NEW_TYPEDEFS,CAMELCASE,COMPLEX_MACRO,SPACING
STYLECHECKIGNORE := $(STYLECHECKIGNORE),AVOID_EXTERNS,STORAGE_CLASS,BRACES
STYLECHECKIGNORE := $(STYLECHECKIGNORE),LEADING_SPACE,SUSPECT_CODE_INDENT

STYLECHECKFLAGS := --no-tree -f --terse --mailback --ignore $(STYLECHECKIGNORE)

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

clean: $(IRQ_DEFN_FILES:=.cleanhdr) $(LIB_DIRS:=.clean) doc.clean styleclean genlinktests.clean apitest.clean
	$(Q)$(RM) .stamp_failure_*

%.clean:
	$(Q)if [ -d $* ]; then \
		printf "  CLEAN   $*\n"; \
		$(MAKE) -C $* clean PREFIX="$(PREFIX)" V="$(V)" || exit $?; \
	fi;

##
## Style checking
##
## A .stylecheck file is left behind for every source that has findings, and
## removed for every source that is clean.  stylecheck then fails if any were
## left, so this is a gate rather than a report: the findings for *all* files
## are printed before make gives up, instead of stopping at the first one.
##
STYLECHECKFILES := $(wildcard include/*/*.h include/*/*/*.h include/*/*/*/*.h)
STYLECHECKFILES += $(wildcard lib/*/*.h lib/*/*/*.h lib/*/*/*/*.h)
STYLECHECKFILES += $(wildcard lib/*/*.c lib/*/*/*.c lib/*/*/*/*.c)

STYLECHECKSTAMPS := $(STYLECHECKFILES:=.stylecheck)

stylecheck: $(STYLECHECKSTAMPS)
	$(Q)fail=; \
	for f in $(STYLECHECKSTAMPS); do \
		[ -s "$$f" ] && fail="$$fail $${f%.stylecheck}"; \
	done; \
	if [ -n "$$fail" ]; then \
		printf "\n  STYLE   findings in:"; \
		for f in $$fail; do printf "\n            %s" "$$f"; done; \
		printf "\n\n"; \
		exit 1; \
	fi;
	@printf "  STYLE   clean\n"

styleclean: $(STYLECHECKFILES:=.styleclean)

# the cat is due to multithreaded nature - we like to have consistent chunks of text on the output
%.stylecheck: %
	$(Q)if grep -q "* It was generated by the irq2nvic_h script." $* ; then \
		rm -f $*.stylecheck; \
	else \
		$(STYLECHECK) $(STYLECHECKFLAGS) $* > $*.stylecheck; \
		if [ -s $*.stylecheck ]; then \
			cat $*.stylecheck; \
		else \
			rm -f $*.stylecheck; \
		fi; \
	fi

%.styleclean:
	$(Q)$(RM) $*.stylecheck;

##
## API smoke test
##
## Compiles and links tests/<family>/api_smoke.c against the whole public API.
## It is a build-level test: the ELF is produced but never run.
##
## The set of tests is derived from TARGETS rather than hardcoded, and the
## target depends on `lib`.  Without both, `make TARGETS=ch32v0 apitest` tried
## to link tests/ch5xx58x against an archive that was never built.
##
APITEST_DIRS := $(wildcard $(addprefix tests/,$(TARGETS)))

apitest: lib
	$(Q)for d in $(APITEST_DIRS); do \
		printf "  TEST    %s\n" "$$d"; \
		$(MAKE) -C $$d PREFIX="$(PREFIX)" || exit $$?; \
	done

apitest.clean:
	$(Q)for d in $(APITEST_DIRS); do $(MAKE) -C $$d clean; done

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

##
## Formatting and hooks.
##
## Formatting is clang-format's job and happens on commit, never in CI: see
## .githooks/pre-commit.  That hook is a no-op when clang-format is not
## installed, so a contributor without LLVM is not blocked.
##
## `make hooks` points this clone's core.hooksPath at the tracked .githooks
## directory.  It is a per-clone setting, hence a target rather than something
## checked in.
##
hooks:
	$(Q)git config core.hooksPath .githooks
	@printf "  HOOKS   core.hooksPath -> .githooks\n"

unhooks:
	$(Q)git config --unset core.hooksPath || true
	@printf "  HOOKS   core.hooksPath unset\n"

.PHONY: build lib $(LIB_DIRS) doc html clean generatedheaders cleanheaders \
	stylecheck styleclean genlinktests genlinktests.clean apitest apitest.clean \
	list-targets hooks unhooks
