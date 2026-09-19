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
## CH32V0 family: CH32V003, CH32V002, CH32V004, CH32V005, CH32V006, CH32V007.
##
## QingKe V2 core, RV32EC (16 registers, compressed instructions).  The
## multiplier is present on every part, but only the CH32V002/004/005/006/007
## advertise it as the `zmmul` extension; the CH32V003 is built as plain
## rv32ec so that toolchains predating the extension name still work.
##
## There is no .highcode section on these parts: nothing has to run from RAM,
## because the flash controller is never reconfigured after boot.
##
## NOTE ON PATHS: every include path is absolute and derived from $(CURDIR)
## through $(abspath).  Relative '../..' paths are deliberately avoided
## because they are fragile under virtualised / sandboxed filesystems.
##

LIBNAME		= libopenwch_ch32v0
SRCLIBDIR	?= $(OPENWCH_TOP)/lib

OPENWCH_TOP	:= $(abspath $(CURDIR)/../..)
OPENWCH_INC	:= $(OPENWCH_TOP)/include
OPENWCH_QINGKE	:= $(OPENWCH_TOP)/lib/qingke
OPENWCH_COMMON	:= $(CURDIR)/common
OPENWCH_FAMILY_COMMON := $(CURDIR)/common

CC		:= $(PREFIX)-gcc
AR		:= $(PREFIX)-ar

VPATH += .:$(OPENWCH_QINGKE):$(OPENWCH_COMMON)

##
## Objects
##
## The core layer (vector.o, vector_chipset.o, nvic.o, assert.o, systick.o) is
## added by lib/Makefile.include.
##
OBJS += vector_handlers.o vector_names.o

##
## Peripheral drivers.  Every object listed here must have a source file in
## this directory or in ./common/.
##
# P2 plan: uncomment each line as its source file lands.
OBJS += gpio.o gpio_common_v1.o
OBJS += rcc.o rcc_common_v1.o
#OBJS += usart.o usart_common_v1.o
#OBJS += tim.o tim_common_v1.o
#OBJS += spi.o spi_common_v1.o
#OBJS += i2c.o i2c_common_v1.o
#OBJS += adc.o adc_common_v1.o
#OBJS += dma.o dma_common_v1.o
#OBJS += exti.o exti_common_v1.o
#OBJS += flash.o flash_common_v1.o
#OBJS += iwdg.o wwdg.o pwr.o opa.o syscfg.o dbgmcu.o

##
## Compile flags.
##
## The family is built as plain rv32ec, WITHOUT the zmmul extension.
##
## Rationale: the archive is shared by every CH32V00x part, and the CH32V003
## does not advertise zmmul, so tagging the archive's objects with it would
## make a linked CH32V003 image claim an extension the part does not have.
##
## Nothing in the core or peripheral code needs the tag either:
##   * the core layer contains no mul/div at all (verified with objdump);
##   * QingKe V2 has no hardware divide, so / and % always call libgcc;
##   * multiplication uses the hardware only when the *application* asks for
##     it, and an application built for a CH32V002/004/005/006/007 gets
##     -march=rv32ec_zmmul_zicsr_zifencei from ld/devices.data automatically.
##
TGT_CFLAGS	= -Os -g3 \
		  -Wall -Wextra -Wimplicit-function-declaration \
		  -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes \
		  -Wundef -Wshadow \
		  -I$(OPENWCH_INC) -fno-common \
		  -ffunction-sections -fdata-sections -MD \
		  -mno-relax \
		  -march=rv32ec_zicsr_zifencei -mabi=ilp32e \
		  -DCH32V0
TGT_CFLAGS	+= $(DEBUG_FLAGS)
TGT_CFLAGS	+= $(STANDARD_FLAGS)

ARFLAGS		= rcs

include $(OPENWCH_TOP)/lib/Makefile.include
