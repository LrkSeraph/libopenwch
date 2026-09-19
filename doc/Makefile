# Makefile to build the libopenwch documentation.
#
# Derived from libopencm3's doc/Makefile (LGPL-3-or-later).  The per-device
# Doxyfile / layout / source list are generated from doc/templates/ and from
# the dependency (.d) files produced by a library build, so the library has to
# be built at least once before `make -C doc html` is meaningful.
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

# setup TARGETS if not set for legacy and ease of debug.
TARGETS ?= ch32v0 ch5xx58x

TARGETS_DIRS = $(subst /,,$(TARGETS))

doc: html

html:
	@echo "  ----    libopenwch Doxygen build"
	@echo "  NOTE    doxygen integration lands in P4 (see phase.md)."
	@echo "  NOTE    The API headers under include/libopenwch/ are already"
	@echo "          annotated with @defgroup/@addtogroup for this step."

clean:
	@$(RM) -rf html */html */doxy.sourcelist */Doxyfile */DoxygenLayout_*.xml

.PHONY: doc html clean
