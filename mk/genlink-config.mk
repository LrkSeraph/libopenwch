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
# The generic linker script generator, the configuration part.
#
# Derived from libopencm3's mk/genlink-config.mk (LGPL-3-or-later), reworked
# for RISC-V:
#
#   * ARM's -mcpu/-mthumb/-mfpu/-mfloat-abi are replaced by -march/-mabi taken
#     from the MARCH/MABI keys of ld/devices.data;
#   * ld/devices.data gains the RISC-V keys MARCH, MABI, ZMMUL, ENTRY, GP and
#     HICODE (see ld/README);
#   * the library archives are libopenwch_<family>.a / libopenwch_<subfamily>.a.
#
# Variable contract (same shape as libopencm3):
#   CPPFLAGS   appended  (-D<family> ... -I)
#   ARCH_FLAGS replaced  (-march=... -mabi=...)
#   LDSCRIPT   replaced  (generated.$(DEVICE).ld)
#   LDLIBS     appended  (-lopenwch_<family>)
#   LDFLAGS    appended  (-L.../lib)
#
# please read mk/README for specification how to use this file in your project
#

ifeq ($(DEVICE),)
$(warning no DEVICE specified for linker script generator)
endif

LDSCRIPT	= generated.$(DEVICE).ld
DEVICES_DATA	= $(OPENWCH_DIR)/ld/devices.data

genlink_family		:=$(shell $(OPENWCH_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) FAMILY)
genlink_subfamily	:=$(shell $(OPENWCH_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) SUBFAMILY)
genlink_march		:=$(shell $(OPENWCH_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) MARCH)
genlink_zext		:=$(shell $(OPENWCH_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) ZEXT)
genlink_mabi		:=$(shell $(OPENWCH_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) MABI)
genlink_zmmul		:=$(shell $(OPENWCH_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) ZMMUL)
genlink_cppflags	:=$(shell $(OPENWCH_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) CPPFLAGS)

CPPFLAGS	+= $(genlink_cppflags)

##
## Architecture flags
##
## zmmul handling: CH32V002/004/005/006/007 are RV32EC parts that nevertheless
## implement the integer multiply instructions.  GCC >= 13 expresses this as the
## `zmmul` extension; older GCC does not know the extension name at all, so it
## must be omitted (the multiply helpers then come from libgcc, i.e. soft
## multiply).
##
## The probe lives here rather than in mk/gcc-config.mk so that this module is
## self-sufficient: an application only has to include genlink-config.mk to get
## a correct ARCH_FLAGS, without having to know which other module computes
## which helper variable.
##
## ZMMUL_OK may still be supplied by the caller (mk/gcc-config.mk sets it) to
## avoid probing twice.
##
ifneq ($(genlink_zmmul),1)
genlink_zmmul_ok :=
else ifneq ($(ZMMUL_OK),)
genlink_zmmul_ok := $(ZMMUL_OK)
else ifneq ($(genlink_march),)
genlink_zmmul_ok := $(shell \
	printf '' | $(CC) -march=$(genlink_march)_zmmul -mabi=$(genlink_mabi) \
		-x c -fsyntax-only - 2>/dev/null && echo yes)
endif

## Order matters for readability only, but keep it canonical:
##   <base ISA>[_zmmul]<always-on extensions>
genlink_march_full := $(genlink_march)
ifeq ($(genlink_zmmul_ok),yes)
genlink_march_full := $(genlink_march)_zmmul
endif
genlink_march_full := $(genlink_march_full)$(genlink_zext)

ifneq ($(genlink_march_full),)
ARCH_FLAGS	:=-march=$(genlink_march_full)
else
$(warning $(DEVICE): no MARCH= key found in $(DEVICES_DATA))
ARCH_FLAGS	:=
endif

ifneq ($(genlink_mabi),)
ARCH_FLAGS	+=-mabi=$(genlink_mabi)
endif

##
## Library selection: prefer the exact family archive, fall back to the
## subfamily archive.  Warnings (not errors) so that a fresh checkout can be
## configured before the library has been built once.
##
ifneq (,$(wildcard $(OPENWCH_DIR)/lib/libopenwch_$(genlink_family).a))
LIBNAME = openwch_$(genlink_family)
else
ifneq (,$(wildcard $(OPENWCH_DIR)/lib/libopenwch_$(genlink_subfamily).a))
LIBNAME = openwch_$(genlink_subfamily)
else
$(warning $(OPENWCH_DIR)/lib/libopenwch_$(genlink_family).a library variant for the selected device does not exist. Run 'make' in $(OPENWCH_DIR) first.)
LIBNAME = openwch_$(genlink_family)
endif
endif

LDLIBS		+= -l$(LIBNAME)
LIBDEPS		+= $(OPENWCH_DIR)/lib/lib$(LIBNAME).a

ifneq (,$(wildcard $(OPENWCH_DIR)/lib))
LDFLAGS		+= -L$(OPENWCH_DIR)/lib
else
$(warning $(OPENWCH_DIR)/lib as given by OPENWCH_DIR does not exist.)
endif

ifneq (,$(wildcard $(OPENWCH_DIR)/include))
CPPFLAGS	+= -I$(OPENWCH_DIR)/include
else
$(warning $(OPENWCH_DIR)/include as given by OPENWCH_DIR does not exist.)
endif
