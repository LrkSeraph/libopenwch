/** @defgroup tim_defines TIM Defines

@brief <b>Advanced-control and general-purpose timers for the CH32V00x</b>

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

#ifndef LIBOPENWCH_TIM_H
#define LIBOPENWCH_TIM_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/ch32v0/memorymap.h>

/*
 * The CH32V00x has two 16-bit timers:
 *
 *   TIM1  advanced-control timer on APB2, four channels with complementary
 *         outputs on channels 1..3, a break input and a dead-time generator.
 *   TIM2  general-purpose timer on APB1, four independent channels and no
 *         break/dead-time block.
 *
 * The channel pins are alternate functions; configure them with
 * gpio_set_mode(..., GPIO_MODE_AF_PP) and pick the pinout with
 * gpio_primary_remap(GPIO_REMAP_TIM1_*) / (GPIO_REMAP_TIM2_*).
 */
#define TIM1 TIM1_BASE
#define TIM2 TIM2_BASE

#include <libopenwch/ch32v0/common/tim_common_v1.h>

#endif
/**@}*/
