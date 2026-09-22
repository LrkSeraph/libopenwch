# Makefile to build the libopenwch API documentation.
#
# Derived from libopencm3's doc/Makefile (LGPL-3-or-later), but simplified:
# libopencm3 generates a Doxyfile and a source list per target through helper
# scripts; libopenwch uses one Doxyfile template with substitutions, because
# the families differ only in their predefine, input set, and output directory.
#
# This file is part of the libopenwch project.
#
# Copyright (C) 2025 libopenwch contributors
#
# This library is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library.  If not, see <http://www.gnu.org/licenses/>.

TARGETS ?= ch32v0 ch5xx58x

DOXYGEN ?= doxygen

TOP := $(abspath $(CURDIR)/..)

# The family define is the upper-cased target name, which is exactly what the
# dispatch headers test against.
fam_upper = $(shell echo $(1) | tr a-z A-Z)

doc: html

html:
	@if ! command -v $(DOXYGEN) >/dev/null 2>&1; then \
		echo "  ----    doxygen not found; skipping documentation build"; \
		echo "  NOTE    install doxygen to generate the API documentation"; \
		exit 0; \
	fi; \
	$(MAKE) -C $(TOP) irq || exit $$?; \
	for t in $(TARGETS); do \
		u=$$(echo $$t | tr a-z A-Z); \
		extra=$$(if [ "$$t" = "ch5xx58x" ]; then echo "include/libopenwch/ble lib/ble"; fi); \
		echo "  DOXYGEN $$t"; \
		mkdir -p $(CURDIR)/$$t; \
		sed -e "s/@FAMILY@/$$t/g" \
		    -e "s/@FAMILY_UPPER@/$$u/g" \
		    -e "s|@OUTPUT_DIR@|doc/$$t|g" \
		    -e "s|@FAMILY_EXTRA_INPUT@|$$extra|g" \
		    Doxyfile.in > $$t/Doxyfile || exit $$?; \
		(cd $(TOP) && $(DOXYGEN) doc/$$t/Doxyfile >/dev/null) || exit $$?; \
	done
	@echo "  ----    documentation written under doc/<family>/html/"
	@echo "  NOTE    warnings (if any) are in doc/<family>/doxygen.log"

clean:
	@$(RM) -rf html
	@for t in $(TARGETS); do $(RM) -rf $$t; done

.PHONY: doc html clean
