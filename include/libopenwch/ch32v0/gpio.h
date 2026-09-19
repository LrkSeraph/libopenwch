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
#define GPIOA				GPIOA_BASE
#define GPIOC				GPIOC_BASE
#define GPIOD				GPIOD_BASE

/* GPIO alternate-function / remap register (AFIO_PCFR1) */
#define AFIO_PCFR1			MMIO32(AFIO_BASE + 0x00)
/* External interrupt configuration register (AFIO_EXTICR) */
#define AFIO_EXTICR			MMIO32(AFIO_BASE + 0x08)
/* Event output control register (AFIO_ECR) */
#define AFIO_ECR			MMIO32(AFIO_BASE + 0x14)

/* AFIO_PCFR1 bits */
#define AFIO_PCFR1_SPI1_REMAP		(1 << 0)
#define AFIO_PCFR1_I2C1_REMAP		(1 << 1)
#define AFIO_PCFR1_USART1_REMAP		(1 << 2)
#define AFIO_PCFR1_TIM1_REMAP		(3 << 6)
#define AFIO_PCFR1_TIM2_REMAP		(3 << 8)
#define AFIO_PCFR1_PA1_PA2_REMAP	(1 << 12)
#define AFIO_PCFR1_ADC1_ETRGINJ_REMAP	(1 << 13)
#define AFIO_PCFR1_ADC1_ETRGREG_REMAP	(1 << 14)
#define AFIO_PCFR1_LSI_CAL_REMAP	(1 << 15)
#define AFIO_PCFR1_SDI_DISABLE		(1 << 24)

/* AFIO_EXTICR: two bits per EXTI line selecting the port. */
#define AFIO_EXTICR_PORTA		0x0
#define AFIO_EXTICR_PORTC		0x2
#define AFIO_EXTICR_PORTD		0x3

#include <libopenwch/ch32v0/common/gpio_common_v1.h>

#endif
/**@}*/
