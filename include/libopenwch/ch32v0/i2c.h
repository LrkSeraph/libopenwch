/** @defgroup i2c_defines I2C Defines

@brief <b>Inter-Integrated Circuit interface for the CH32V00x</b>

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

#ifndef LIBOPENWCH_I2C_H
#define LIBOPENWCH_I2C_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/ch32v0/memorymap.h>

/*
 * The CH32V00x has a single I2C controller, I2C1, on APB1.  Its default pins
 * are PC1 (SDA) and PC2 (SCL); the remap options move it to PC2/PC1 (partial)
 * or PD0/PD1 (full).  Configure the pins with gpio_set_mode(..., GPIO_MODE_AF_OD)
 * and select the remap with gpio_primary_remap(GPIO_REMAP_I2C1_*) -- the bus
 * requires open-drain outputs, so never use GPIO_MODE_AF_PP here.
 */
#define I2C1 I2C1_BASE

#include <libopenwch/ch32v0/common/i2c_common_v1.h>

#endif
/**@}*/
