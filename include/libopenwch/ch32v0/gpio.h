/** @defgroup gpio_defines GPIO Defines

@brief <b>Defined Constants and Types for the CH32V00x General Purpose I/O</b>

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

#ifndef LIBOPENWCH_GPIO_H
#define LIBOPENWCH_GPIO_H

#include <libopenwch/ch32v0/memorymap.h>

/*
 * The CH32V00x only exposes ports A, C and D, and they are 8 bits wide.
 * There is no GPIOB.
 */
#define GPIOA GPIOA_BASE
#define GPIOC GPIOC_BASE
#define GPIOD GPIOD_BASE

/* GPIO alternate-function / remap register (AFIO_PCFR1) */
#define AFIO_PCFR1 MMIO32(AFIO_BASE + 0x04)
/* External interrupt configuration register (AFIO_EXTICR) */
#define AFIO_EXTICR MMIO32(AFIO_BASE + 0x08)

/*
 * AFIO_PCFR1 bits.
 *
 * Taken from the CH32V003 SVD (misc/CH32V003xx.svd, register AFIO.PCFR1),
 * which is the machine-readable form of the reference manual and the most
 * reliable source for this register: WCH's own EVT encodes the remap options
 * as packed 32-bit tokens whose bit layout is easy to get wrong.
 *
 * USART1 and I2C1 each have two remap bits which together form a 2-bit code:
 *
 *	code 0b00  no remap
 *	code 0b01  partial remap 1
 *	code 0b10  partial remap 2
 *	code 0b11  full remap
 *
 * (For I2C1 only 0b01 and 0b11 are meaningful; 0b10 is not documented.)
 */
#define AFIO_PCFR1_SPI1_RM (1 << 0)
#define AFIO_PCFR1_I2C1_RM (1 << 1)
#define AFIO_PCFR1_USART1_RM (1 << 2)
#define AFIO_PCFR1_TIM1_RM_SHIFT 6
#define AFIO_PCFR1_TIM1_RM_MASK (0x3u << 6)
#define AFIO_PCFR1_TIM2_RM_SHIFT 8
#define AFIO_PCFR1_TIM2_RM_MASK (0x3u << 8)
/* Port A1/A2 mapped onto OSCIN/OSCOUT instead of PD0/PD1. */
#define AFIO_PCFR1_PA12_RM (1 << 15)
#define AFIO_PCFR1_ADC1_ETRGINJ_RM (1 << 17)
#define AFIO_PCFR1_ADC1_ETRGREG_RM (1 << 18)
/* Second remap bit of the USART1 / I2C1 two-bit code. */
#define AFIO_PCFR1_USART1_REMAP1 (1 << 21)
#define AFIO_PCFR1_I2C1_REMAP1 (1 << 22)
/* TIM1_CH1 channel selection. */
#define AFIO_PCFR1_TIM1_1_RM (1 << 23)
#define AFIO_PCFR1_SWCFG_SHIFT 24
#define AFIO_PCFR1_SWCFG_MASK (0x7u << 24)

/*
 * Debug-interface (SWD/SDI) disable and LSI calibration output.  The SVD
 * does not name these as separate fields: WCH's EVT maps SDI disable to
 * SWCFG = 0b100 (bit 26) and LSI calibration to bit 23.  They are kept as
 * aliases so callers can use the EVT names without duplicating token magic.
 */
#define AFIO_PCFR1_SDI_DISABLE (1u << 26)
#define AFIO_PCFR1_LSI_CAL AFIO_PCFR1_TIM1_1_RM

/* AFIO_EXTICR: two bits per EXTI line selecting the port. */
#define AFIO_EXTICR_PORTA 0x0
#define AFIO_EXTICR_PORTC 0x2
#define AFIO_EXTICR_PORTD 0x3

#include <libopenwch/ch32v0/common/gpio_common_v1.h>

#endif
/**@}*/
