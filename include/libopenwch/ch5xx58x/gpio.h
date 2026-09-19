/** @defgroup gpio_defines GPIO Defines

@brief <b>General Purpose I/O for the CH58x</b>

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

#ifndef LIBOPENWCH_GPIO_H
#define LIBOPENWCH_GPIO_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/ch5xx58x/memorymap.h>

/* The CH58x has two ports: GPIOA (16 pins) and GPIOB (24 pins). */
#define GPIOA				GPIOA_BASE
#define GPIOB				GPIOB_BASE

/*
 * Pin alternate function selection (R16_PIN_ALTERNATE) and analog input
 * enable (R16_PIN_ANALOG_IE).  Both are plain read/write.
 */
#define GPIO_PIN_ALTERNATE		MMIO16(R16_PIN_ALTERNATE)
#define GPIO_PIN_ANALOG_IE		MMIO16(R16_PIN_ANALOG_IE)

#include <libopenwch/ch5xx58x/common/gpio_common_v1.h>

#endif
/**@}*/
