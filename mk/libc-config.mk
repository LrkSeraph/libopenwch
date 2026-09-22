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

###############################################################################
# Which C library an application links against, the configuration part.
#
# The library itself needs no C library.  An application usually does: the
# compiler's own startup glue, memcpy, printf and friends.  Two things make
# that awkward here, and both are properties of the *toolchain*, not of the
# project:
#
#   * several distributions build gcc-riscv64-unknown-elf with no newlib at
#     all for the rv32e and rv32imac multilibs, so `-lc -lnosys` fails to link
#     with "cannot find -lc" -- Debian and Ubuntu packages have done this, and
#     Ubuntu's 13.2.0 build also drags in a `-lgloss` from its specs;
#   * libopenwch ships a freestanding mini-libc for exactly that case.
#
# So the choice is probed rather than assumed.  A caller should not have to
# know whether their toolchain was built with newlib:
#
#   LIBOPENWCH_NOSTDLIB=1   force the mini-libc
#   LIBOPENWCH_NOSTDLIB=0   force newlib
#   (unset)                 probe, and use whichever links
#
# The probe links an empty program the same way the real link would.  If the
# multilib has no libc, ld says so and the mini-libc is used instead.
#
# please read mk/README for specification how to use this file in your project
#

##
## LIBOPENWCH_LIBC_PROBE may be set to yes/no to stand in for the probe.  It
## exists so that both branches can be exercised on a machine whose toolchain
## only has one of them, and as an escape hatch if the probe ever guesses
## wrong.
##
ifeq ($(LIBOPENWCH_LIBC_PROBE),)
  libc_probe := $(shell printf 'int main(void) { return 0; }\n' | \
	$(CC) $(ARCH_FLAGS) -x c - -lc -lnosys -o /dev/null 2>/dev/null \
	&& echo yes || echo no)
  LIBOPENWCH_LIBC_PROBE := $(libc_probe)
endif

ifeq ($(LIBOPENWCH_NOSTDLIB),)
  ifeq ($(LIBOPENWCH_LIBC_PROBE),yes)
    LIBOPENWCH_NOSTDLIB := 0
  else
    LIBOPENWCH_NOSTDLIB := 1
    ## Say it once, and say why: a silent switch of C library would be a
    ## strange thing to discover from a linker map.
    $(info libopenwch: this toolchain has no libc for $(ARCH_FLAGS); \
linking the bundled mini-libc)
  endif
endif

ifeq ($(LIBOPENWCH_NOSTDLIB),1)
  LDFLAGS	+= -nostdlib
  LDLIBS	+= $(OPENWCH_DIR)/lib/libopenwch_mini_libc_$(genlink_family).a \
		   -lgcc
else
  LDFLAGS	+= -Wl,--start-group
  LDLIBS	+= -lc -lgcc -lnosys
  LDFLAGS	+= -Wl,--end-group
endif
