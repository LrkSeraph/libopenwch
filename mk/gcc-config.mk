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
# The support makefile for the RISC-V GCC toolchain, the configuration part.
#
# Derived from libopencm3's mk/gcc-config.mk (LGPL-3-or-later) and from the
# toolchain discovery logic of ch32fun's ch32fun/ch32fun.mk.
#
# please read mk/README for specification how to use this file in your project

# No toolchain flags here on purpose: this module only discovers the toolchain
# and records its capabilities.  Architecture flags come from genlink-config.mk
# (for applications) or from the per-family lib/<family>/Makefile.include (for
# building the library itself).

##
## Toolchain prefix discovery
##
## The probe chain is deliberately restricted to bare-metal RISC-V toolchains.
## `riscv64-linux-gnu-` is intentionally NOT probed: its startup files and libc
## conventions differ enough to break -nostdlib firmware builds, which is the
## same decision ch32fun made.
##
## Override with:  make PREFIX=/opt/xpack/bin/riscv-none-elf-
## or set the environment variable PREFIX.
##
ifeq ($(PREFIX),)
  PREFIX_DEFAULT := $(shell \
	for p in riscv64-unknown-elf riscv64-none-elf riscv32-unknown-elf \
	         riscv-none-elf riscv64-elf riscv32-elf; do \
		if command -v $$p-gcc >/dev/null 2>&1; then echo $$p; exit 0; fi; \
	done; \
	echo riscv64-unknown-elf)
  PREFIX := $(PREFIX_DEFAULT)
endif

## Use := rather than ?= for the tool variables.
##
## GNU make predefines CC, CXX, LD, AS, AR and friends as built-in variables
## (CC defaults to "cc", LD to "ld", AS to "as"), so ?= would never take
## effect and the build would silently use the HOST tools to compile RISC-V
## firmware -- producing x86 objects that fail at link time, or worse, appear
## to succeed when the host binutils happen to cope.
##
## A command-line assignment (make CC=...) still wins over these, because make
## always gives command-line variables priority over makefile assignments.
##
CC		:= $(PREFIX)-gcc
CXX		:= $(PREFIX)-g++
LD		:= $(PREFIX)-gcc
AR		:= $(PREFIX)-ar
AS		:= $(PREFIX)-as
OBJCOPY		:= $(PREFIX)-objcopy
OBJDUMP		:= $(PREFIX)-objdump
GDB		:= $(PREFIX)-gdb
SIZE		:= $(PREFIX)-size
NM		:= $(PREFIX)-nm
CPP		:= $(PREFIX)-gcc -E
RANLIB		:= $(PREFIX)-ranlib

##
## Toolchain capability probing
##
## GCCVERSION / GCCMAJOR are also used by lib/<family>/Makefile.include to
## decide whether the *assembler* accepts the `zmmul` extension.
##
GCCVERSION	?= $(shell $(CC) -dumpversion 2>/dev/null)
GCCMAJOR	?= $(firstword $(subst ., ,$(GCCVERSION)))

##
## openwch_cc_accepts(<arch>)
##
## Returns non-empty if the compiler accepts -march=<arch>.  Cheap probe: one
## empty translation unit, no output file.  Used to decide whether `zmmul` may
## be appended to `rv32ec` (CH32V002/004/005/006/007 need integer multiply but
## `zmmul` is only understood by GCC >= 13).
##
openwch_cc_accepts = $(shell \
	printf '' | $(CC) -march=$(1) -mabi=$(2) -x c -fsyntax-only - 2>/dev/null \
	&& echo yes)

ZMMUL_OK	?= $(call openwch_cc_accepts,rv32ec_zmmul,ilp32e)

##
## openwch_check_toolchain
##
## Hard failure with an actionable message if the compiler is missing.  This is
## intentionally an $(error ...) rather than a silent fallback: silently
## building RISC-V firmware with arm-none-eabi-gcc produces unusable output.
##
openwch_check_toolchain = $(if $(shell command -v $(CC) >/dev/null 2>&1 && echo ok),,\
  $(error Cannot find the RISC-V toolchain '$(CC)'. \
    Install it (Debian/Kali: sudo apt-get install gcc-riscv64-unknown-elf) \
    or pass PREFIX=... pointing at your toolchain, \
    e.g. make PREFIX=/opt/xpack-riscv-none-elf-gcc/bin/riscv-none-elf-))
