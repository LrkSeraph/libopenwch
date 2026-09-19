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
## CH58x family: CH582, CH583, CH584, CH585.
##
## QingKe V4 core, RV32IMAC.  These parts have no HSI: the 32 MHz HSE crystal
## is mandatory and the PLL runs at a fixed 480 MHz, so bring-up must power
## the crystal before selecting a PLL clock source.
##
## Most system, clock, power and GPIO registers are RWA (write-protected) and
## need the 0x57/0xA8 safe-access unlock sequence; see lib/ch5xx58x/rwa.c.
##
## .highcode is required: flash-controller and low-power code cannot run from
## flash.
##
## NOTE ON PATHS: every include path is absolute and derived from $(CURDIR)
## through $(abspath).  Relative '../..' paths are deliberately avoided
## because they are fragile under virtualised / sandboxed filesystems.
##

LIBNAME		= libopenwch_ch5xx58x
SRCLIBDIR	?= $(OPENWCH_TOP)/lib

OPENWCH_TOP	:= $(abspath $(CURDIR)/../..)
OPENWCH_INC	:= $(OPENWCH_TOP)/include
OPENWCH_QINGKE	:= $(OPENWCH_TOP)/lib/qingke
OPENWCH_MINILIB	:= $(OPENWCH_TOP)/lib/mini_libc
OPENWCH_BLE	:= $(OPENWCH_TOP)/lib/ble
OPENWCH_COMMON	:= $(CURDIR)/common

CC		:= $(PREFIX)-gcc
AR		:= $(PREFIX)-ar

VPATH += .:$(OPENWCH_QINGKE):$(OPENWCH_COMMON):$(OPENWCH_MINILIB):$(OPENWCH_BLE)

##
## The freestanding mini libc for this family, in its own archive.  Built and
## linked only when an application asks for it with LIBOPENWCH_NOSTDLIB=1.
##
MINILIB		= libopenwch_mini_libc_$(notdir $(CURDIR))
MINILIB_OBJS	= mini_libc.o


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
OBJS += rwa.o clk.o
OBJS += gpio.o gpio_common_v1.o
OBJS += adc.o adc_common_v1.o flash.o flash_common_v1.o
OBJS += tmr.o tmr_common_v1.o
OBJS += pwm.o pwm_common_v1.o
OBJS += pwr.o pwr_common_v1.o
OBJS += spi.o spi_common_v1.o
OBJS += i2c.o i2c_common_v1.o
OBJS += uart.o uart_common_v1.o
OBJS += sys.o sys_common_v1.o

##
## Bluetooth LE.
##
## This is the layer over WCH's closed-source stack, not the stack itself: the
## objects below call into LIBCH58xBLE.a, which an application links
## separately (see README).  They live in this archive because the stack is
## CH58x-only, and they cost nothing to an application that never references
## them -- an archive member is only pulled in to satisfy an undefined symbol.
##
OBJS += ble.o ble_tmos.o ble_gap.o ble_gatt.o

##
## Compile flags: RV32IMAC, no float ABI.
##
TGT_CFLAGS	= -Os -g3 \
		  -Wall -Wextra -Wimplicit-function-declaration \
		  -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes \
		  -Wundef -Wshadow \
		  -I$(OPENWCH_INC) -fno-common \
		  -ffunction-sections -fdata-sections -MD \
		  -mno-relax \
		  -march=rv32imac_zicsr_zifencei -mabi=ilp32 \
		  -DCH5XX58X
TGT_CFLAGS	+= $(DEBUG_FLAGS)
TGT_CFLAGS	+= $(STANDARD_FLAGS)

ARFLAGS		= rcs

include $(OPENWCH_TOP)/lib/Makefile.include
