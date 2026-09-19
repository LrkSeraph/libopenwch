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
# The support makefile for the GCC compiler toolchain, the rules part.
#
# Derived from libopencm3's mk/gcc-rules.mk (LGPL-3-or-later).
#
# please read mk/README for specification how to use this file in your project
#

# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q := @
endif

%.bin: %.elf
	@printf "  OBJCOPY $@\n"
	$(Q)$(OBJCOPY) -Obinary $< $@

%.hex: %.elf
	@printf "  OBJCOPY $@\n"
	$(Q)$(OBJCOPY) -Oihex $< $@

%.srec: %.elf
	@printf "  OBJCOPY $@\n"
	$(Q)$(OBJCOPY) -Osrec $< $@

%.list: %.elf
	@printf "  OBJDUMP $@\n"
	$(Q)$(OBJDUMP) -d -S $< > $@

%.elf: $(OBJS) $(LDSCRIPT) $(LIBDEPS)
	@printf "  LD      $(*).elf\n"
	$(Q)$(LD) $(OBJS) $(LDLIBS) $(LDFLAGS) -T$(LDSCRIPT) $(ARCH_FLAGS) -o $@

%.o: %.c
	@printf "  CC      $<\n"
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) $(ARCH_FLAGS) -o $@ -c $<

%.o: %.S
	@printf "  AS      $<\n"
	$(Q)$(CC) $(CPPFLAGS) $(ARCH_FLAGS) -o $@ -c $<

%.o: %.s
	@printf "  AS      $<\n"
	$(Q)$(CC) $(CPPFLAGS) $(ARCH_FLAGS) -o $@ -c $<

%.o: %.cxx
	@printf "  CXX     $<\n"
	$(Q)$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(ARCH_FLAGS) -o $@ -c $<

%.o: %.cpp
	@printf "  CXX     $(*).cpp\n"
	$(Q)$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(ARCH_FLAGS) -o $@ -c $<
