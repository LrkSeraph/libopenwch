##
## This file is part of the libopenwch template.
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
## Toolchain discovery and the host-side utilities.
##
## The RISC-V probe chain is the same one a library build uses, so a project
## can be built with exactly the toolchain that built libopenwch.
##
## PREFIX is the toolchain prefix WITHOUT the trailing '-', e.g.
##	riscv64-unknown-elf
## and may be overridden on the command line or in the environment.
##

ifeq ($(PREFIX),)
  PREFIX := $(shell \
	for p in riscv64-unknown-elf riscv64-none-elf riscv32-unknown-elf \
	         riscv-none-elf riscv64-elf riscv32-elf; do \
		if command -v $$p-gcc >/dev/null 2>&1; then echo $$p; exit 0; fi; \
	done; \
	echo riscv64-unknown-elf)
endif

## Use := (not ?=) on purpose: GNU make predefines CC, CXX and a few others as
## built-in variables (CC defaults to `cc`), so ?= would never take effect and
## the build would silently use the host compiler.
CC		:= $(PREFIX)-gcc
CXX		:= $(PREFIX)-g++
LD		:= $(PREFIX)-gcc
AS		:= $(PREFIX)-as
AR		:= $(PREFIX)-ar
OBJCOPY		:= $(PREFIX)-objcopy
OBJDUMP		:= $(PREFIX)-objdump
GDB		:= $(PREFIX)-gdb
SIZE		:= $(PREFIX)-size

##
## Flashing.
##
## minichlink (https://github.com/cnlohr/ch32fun) drives the WCH-Link and the
## built-in USB ISP bootloader, and needs no vendor software.  Point
## MINICHLINK at the built binary if it is not on PATH.
##
MINICHLINK	?= minichlink
MINICHLINK_FLAGS ?= -b

## Interactive terminal over the single-wire debug channel.
monitor:
	$(MINICHLINK) -T

## Unbrick / reset a part that no longer answers.
unbrick:
	$(MINICHLINK) -u

## Fail early with an actionable message instead of a wall of compiler noise.
ifeq ($(shell command -v $(CC) >/dev/null 2>&1 && echo ok),)
$(error Cannot find the RISC-V toolchain '$(CC)'. Install it (Debian/Kali: \
    sudo apt-get install gcc-riscv64-unknown-elf) or pass PREFIX=...)
endif

.PHONY: monitor unbrick
