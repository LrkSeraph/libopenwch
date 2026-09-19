/** @defgroup ch5xx58x_memorymap_defines Memory map

@brief <b>Peripheral memory map for the CH58x family</b>

@ingroup CH5XX58X_defines

@version 1.0.0

@date 1 January 2025

LGPL License Terms @ref lgpl_license
 */
/*
 * This file is part of the libopenwch project.
 *
 * Copyright (C) 2025 libopenwch contributors
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

/**@{*/

#ifndef LIBOPENWCH_MEMORYMAP_H
#define LIBOPENWCH_MEMORYMAP_H

#include <libopenwch/qingke/memorymap.h>

/*
 * The CH58x does NOT use the STM32-style bus layout of the CH32V00x.  Every
 * peripheral sits in one flat SFR window at 0x40000000, at a 0x400-aligned
 * offset:
 *
 *	0x40001000  SYS     system, clock, sleep, power, RTC, ADC/touch
 *	0x400010a0  GPIOA
 *	0x400010c0  GPIOB
 *	0x40002000  TMR0
 *	0x40002400  TMR1
 *	0x40002800  TMR2
 *	0x40002c00  TMR3
 *	0x40003000  UART0
 *	0x40003400  UART1
 *	0x40003800  UART2
 *	0x40003c00  UART3
 *	0x40004000  SPI0
 *	0x40004400  SPI1
 *	0x40004800  I2C
 *	0x40005000  PWMX
 *	0x40008000  USB
 *	0x40008400  USB2   (CH583/CH582 only)
 *	0x4000c000  BLE    (registers only; the stack is a closed library)
 */

#define SFR_BASE			(0x40000000U)

/* --- Code / data --------------------------------------------------------- */
#define CODE_BASE			(0x00000000U)
#define RAM_BASE			(0x20000000U)

/* --- System -------------------------------------------------------------- */
#define SYS_BASE			(SFR_BASE + 0x1000)

/* The SYS block is best addressed by byte/word offset from SYS_BASE because
 * its registers are a mix of 8-, 16- and 32-bit views of the same addresses.
 * The important ones are named here. */
#define R32_SAFE_ACCESS			(SYS_BASE + 0x40)
#define R8_SAFE_ACCESS_SIG		(SYS_BASE + 0x40)
#define R8_SAFE_ACCESS_ID		(SYS_BASE + 0x42)
#define R32_CLK_SYS_CFG			(SYS_BASE + 0x08)
#define R16_CLK_SYS_CFG			(SYS_BASE + 0x08)
#define R8_HFCK_PWR_CTRL		(SYS_BASE + 0x0a)
#define R32_SLEEP_CONTROL		(SYS_BASE + 0x0c)
#define R16_PIN_ALTERNATE		(SYS_BASE + 0x18)
#define R16_PIN_ANALOG_IE		(SYS_BASE + 0x1a)
#define R32_POWER_MANAG			(SYS_BASE + 0x20)
#define R16_POWER_PLAN			(SYS_BASE + 0x20)
#define R32_OSC32K_CTRL			(SYS_BASE + 0x2c)
#define R32_RTC_CTRL			(SYS_BASE + 0x30)
#define R32_RTC_TRIG			(SYS_BASE + 0x34)
#define R32_RTC_CNT_32K			(SYS_BASE + 0x38)
#define R32_MISC_CTRL			(SYS_BASE + 0x48)
#define R8_PLL_CONFIG			(SYS_BASE + 0x4b)
#define R32_OSC32M_CTRL			(SYS_BASE + 0x4c)
#define R8_XT32M_TUNE			(SYS_BASE + 0x4e)
#define R8_CHIP_ID			(SYS_BASE + 0x46)
#define R8_GLOB_RESET_KEEP		(SYS_BASE + 0x44)
#define R8_RST_WDOG_CTRL		(SYS_BASE + 0x46)

/* GPIO interrupt banks */
#define R16_PA_INT_EN			(SYS_BASE + 0x90)
#define R16_PB_INT_EN			(SYS_BASE + 0x92)
#define R16_PA_INT_MODE			(SYS_BASE + 0x94)
#define R16_PB_INT_MODE			(SYS_BASE + 0x96)
#define R16_PA_INT_IF			(SYS_BASE + 0x9c)
#define R16_PB_INT_IF			(SYS_BASE + 0x9e)

/* Flash controller (ROM control registers) */
#define ROM_CTRL_BASE			(SYS_BASE + 0x800)
#define R8_FLASH_CFG			(ROM_CTRL_BASE + 0x07)

/* --- GPIO ---------------------------------------------------------------- */
#define GPIOA_BASE			(SFR_BASE + 0x10a0)
#define GPIOB_BASE			(SFR_BASE + 0x10c0)

/* Per-port register layout, 0x20 bytes per port. */
#define GPIO_DIR_OFFSET			0x00	/**< direction, 1 = output */
#define GPIO_PIN_OFFSET			0x04	/**< input value (read only) */
#define GPIO_OUT_OFFSET			0x08	/**< output value */
#define GPIO_CLR_OFFSET			0x0c	/**< write 1 to clear (write only) */
#define GPIO_PU_OFFSET			0x10	/**< pull-up enable */
#define GPIO_PD_DRV_OFFSET		0x14	/**< input pull-down / output drive */

/* --- Peripherals --------------------------------------------------------- */
#define TMR0_BASE			(SFR_BASE + 0x2000)
#define TMR1_BASE			(SFR_BASE + 0x2400)
#define TMR2_BASE			(SFR_BASE + 0x2800)
#define TMR3_BASE			(SFR_BASE + 0x2c00)

#define UART0_BASE			(SFR_BASE + 0x3000)
#define UART1_BASE			(SFR_BASE + 0x3400)
#define UART2_BASE			(SFR_BASE + 0x3800)
#define UART3_BASE			(SFR_BASE + 0x3c00)

#define SPI0_BASE			(SFR_BASE + 0x4000)
#define SPI1_BASE			(SFR_BASE + 0x4400)

#define I2C1_BASE			(SFR_BASE + 0x4800)

#define PWMX_BASE			(SFR_BASE + 0x5000)

#define USB_BASE			(SFR_BASE + 0x8000)
#define USB2_BASE			(SFR_BASE + 0x8400)

#define BLE_BASE			(SFR_BASE + 0xc000)

/* --- Factory information ------------------------------------------------- */
#define INFO_BASE			(0x00000000U)
#define ROM_CFG_BASE			(0x0007f000U)
#define ROM_CFG_MAC_ADDR		(ROM_CFG_BASE + 0x018)
#define ROM_CFG_TMP_25C			(ROM_CFG_BASE + 0x014)
#define BOOT_LOAD_CFG			(0x0007e000U)

/* Data flash (EEPROM emulation) */
#define DATA_FLASH_ADDR			(0x00070000U)
#define DATA_FLASH_SIZE			(0x8000U)

#endif
/**@}*/
