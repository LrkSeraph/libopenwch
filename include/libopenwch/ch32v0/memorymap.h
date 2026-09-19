/** @defgroup ch32v0_memorymap_defines Memory map

@brief <b>Peripheral memory map for the CH32V00x family</b>

@ingroup CH32V0_defines

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

/* --- CH32V00x specific peripheral definitions ---------------------------- */

/* The CH32V00x keeps the STM32-style three-bus memory map. */
#define FLASH_BASE			(0x00000000U)
#define INFO_BASE			(0x1ffff000U)
#define PERIPH_BASE			(0x40000000U)
#define PERIPH_BASE_APB1		(PERIPH_BASE + 0x00000)
#define PERIPH_BASE_APB2		(PERIPH_BASE + 0x10000)
#define PERIPH_BASE_AHB			(PERIPH_BASE + 0x20000)

/* Factory-programmed information blocks. */
#define ESIG_BASE			(0x1ffff7e0U)
#define OB_BASE				(0x1ffff800U)
#define VENDOR_CFG0_BASE		(0x1ffff7d4U)

/* --- APB1 ---------------------------------------------------------------- */
#define TIM2_BASE			(PERIPH_BASE_APB1 + 0x0000)
#define WWDG_BASE			(PERIPH_BASE_APB1 + 0x2c00)
#define IWDG_BASE			(PERIPH_BASE_APB1 + 0x3000)
#define I2C1_BASE			(PERIPH_BASE_APB1 + 0x5400)
#define PWR_BASE			(PERIPH_BASE_APB1 + 0x7000)

/* --- APB2 ---------------------------------------------------------------- */
#define AFIO_BASE			(PERIPH_BASE_APB2 + 0x0000)
#define EXTI_BASE			(PERIPH_BASE_APB2 + 0x0400)
#define GPIOA_BASE			(PERIPH_BASE_APB2 + 0x0800)
/* PERIPH_BASE_APB2 + 0x0c00 is reserved (no GPIOB on this family) */
#define GPIOC_BASE			(PERIPH_BASE_APB2 + 0x1000)
#define GPIOD_BASE			(PERIPH_BASE_APB2 + 0x1400)
#define ADC1_BASE			(PERIPH_BASE_APB2 + 0x2400)
#define TIM1_BASE			(PERIPH_BASE_APB2 + 0x2c00)
#define SPI1_BASE			(PERIPH_BASE_APB2 + 0x3000)
#define USART1_BASE			(PERIPH_BASE_APB2 + 0x3800)

/* --- AHB ----------------------------------------------------------------- */
#define DMA1_BASE			(PERIPH_BASE_AHB + 0x0000)
#define DMA1_CHANNEL1_BASE		(PERIPH_BASE_AHB + 0x0008)
#define DMA1_CHANNEL2_BASE		(PERIPH_BASE_AHB + 0x001c)
#define DMA1_CHANNEL3_BASE		(PERIPH_BASE_AHB + 0x0030)
#define DMA1_CHANNEL4_BASE		(PERIPH_BASE_AHB + 0x0044)
#define DMA1_CHANNEL5_BASE		(PERIPH_BASE_AHB + 0x0058)
#define DMA1_CHANNEL6_BASE		(PERIPH_BASE_AHB + 0x006c)
#define DMA1_CHANNEL7_BASE		(PERIPH_BASE_AHB + 0x0080)
#define RCC_BASE			(PERIPH_BASE_AHB + 0x1000)
#define FLASH_R_BASE			(PERIPH_BASE_AHB + 0x2000)
#define EXTEN_BASE			(PERIPH_BASE_AHB + 0x3800)
/*
 * There is no memory-mapped DBGMCU block on this family: the debug control
 * register (which WCH's EVT calls CFGR0) lives in CSR 0x7c0 and is reached
 * with csrr/csrw, and the chip revision/device id live in the factory block
 * at 0x1ffff7c4.  See include/libopenwch/ch32v0/dbgmcu.h.
 */
#define DBGMCU_ID_BASE			(0x1ffff7c4U)

#endif
/**@}*/
