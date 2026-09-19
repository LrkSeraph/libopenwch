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
# The generic linker script generator, the rules part.
#
# Derived from libopencm3's mk/genlink-rules.mk (LGPL-3-or-later).
#
# The device's DEFS (from ld/devices.data) are turned into -D_ROM=..., -D_RAM=...,
# -D_ROM_OFF=..., -D_RAM_OFF=... and friends, plus the RISC-V-specific
# -D_ENTRY=..., -D_GP=..., -D_HICODE=... .  ld/linker.ld.S is then run through
# the preprocessor to produce the final linker script for the selected device.
#
# please read mk/README for specification how to use this file in your project
#

ifneq ($(V),1)
Q := @
endif

$(LDSCRIPT): $(OPENWCH_DIR)/ld/linker.ld.S $(DEVICES_DATA)
	@printf "  GENLNK  $(DEVICE)\n"
	$(Q)$(CPP) $(ARCH_FLAGS) $(shell $(OPENWCH_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) DEFS) -P -E $< -o $@
