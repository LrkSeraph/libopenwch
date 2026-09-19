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
## CH58x API smoke test.
##
## Compiles tests/ch5xx58x/api_smoke.c against the whole public API and links it
## with the library, so that every header, every prototype and the archive are
## proven to fit together.  The -Werror is deliberate: the library itself is
## built with the same warning set, so a warning here is a regression.
##
## Run from the libopenwch root with:  make apitest
##

OPENWCH_DIR	?= ../..
DEVICE		?= ch582m

## Toolchain discovery, shared with the library and the application template.
## PREFIX is the toolchain prefix WITHOUT the trailing dash.
include $(OPENWCH_DIR)/mk/gcc-config.mk

DEVICES_DATA	= $(OPENWCH_DIR)/ld/devices.data

## Architecture flags and the family define come from the device database, so
## the test uses exactly the ISA the application would.
GENLINK_PY	= $(OPENWCH_DIR)/scripts/genlink.py
ARCH_FLAGS	:= -march=$(shell $(GENLINK_PY) $(DEVICES_DATA) $(DEVICE) MARCH)$(shell $(GENLINK_PY) $(DEVICES_DATA) $(DEVICE) ZEXT) \
		   -mabi=$(shell $(GENLINK_PY) $(DEVICES_DATA) $(DEVICE) MABI)
FAMILY_CPPFLAGS	:= $(shell $(GENLINK_PY) $(DEVICES_DATA) $(DEVICE) CPPFLAGS)

## The startup code in lib/qingke/vector.c may call memcpy()/memset() to copy
## .data and zero .bss -- whether it does depends on the compiler version, so
## the archive cannot be assumed to be free of them.  This link is freestanding
## (-nostdlib), so satisfy the reference from the mini-libc rather than from a
## C library the toolchain may not ship.
MINILIB		= $(OPENWCH_DIR)/lib/libopenwch_mini_libc_$(shell $(GENLINK_PY) $(DEVICES_DATA) $(DEVICE) FAMILY).a

WARNINGS	= -Wall -Wextra -Wredundant-decls -Wmissing-prototypes \
		  -Wstrict-prototypes -Wundef -Wshadow -Werror

CFLAGS		= -Os -g3 -std=c99 -fno-common -mno-relax \
		  -ffunction-sections -fdata-sections \
		  -I$(OPENWCH_DIR)/include $(FAMILY_CPPFLAGS) $(WARNINGS)

all: api_smoke.elf

api_smoke.o: api_smoke.c
	@printf "  CC      $<\n"
	$(Q)$(CC) $(CFLAGS) $(ARCH_FLAGS) -c $< -o $@

## The linker script comes from the same generator an application uses.
generated.$(DEVICE).ld: $(OPENWCH_DIR)/ld/linker.ld.S $(DEVICES_DATA)
	@printf "  GENLNK  $(DEVICE)\n"
	$(Q)$(CC) $(ARCH_FLAGS) $(shell $(GENLINK_PY) $(DEVICES_DATA) $(DEVICE) DEFS) \
		-P -E $< -o $@

api_smoke.elf: api_smoke.o generated.$(DEVICE).ld
	@printf "  LD      $@\n"
	$(Q)$(CC) $(CFLAGS) $(ARCH_FLAGS) -nostdlib \
		-T generated.$(DEVICE).ld -Wl,--gc-sections \
		$< -L$(OPENWCH_DIR)/lib -lopenwch_$(shell $(GENLINK_PY) $(DEVICES_DATA) $(DEVICE) FAMILY) \
		$(MINILIB) -lgcc -o $@
	@$(SIZE) $@

clean:
	$(Q)$(RM) *.o *.elf generated.*.ld

.PHONY: all clean
