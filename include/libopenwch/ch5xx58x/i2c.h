/** @defgroup i2c_defines I2C Defines

@brief <b>Inter-Integrated Circuit for the CH58x</b>

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

#ifndef LIBOPENWCH_I2C_H
#define LIBOPENWCH_I2C_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/ch5xx58x/memorymap.h>

/*
 * The CH58x has a single I2C controller, I2C1, at 0x40004800.  Its default
 * pins are PB13 (SCL) and PB12 (SDA); GPIO_REMAP_I2C moves them to PB21
 * (SCL) and PB20 (SDA).  Select the alternate function with
 * gpio_pin_remap(GPIO_REMAP_I2C); the pins then drive open-drain as the bus
 * requires, so they must not be configured as push-pull outputs.
 */
#define I2C1				I2C1_BASE

#include <libopenwch/ch5xx58x/common/i2c_common_v1.h>

#endif
/**@}*/
