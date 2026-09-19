/** @defgroup spi_defines SPI Defines

@brief <b>Serial Peripheral Interface for the CH58x</b>

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

#ifndef LIBOPENWCH_SPI_H
#define LIBOPENWCH_SPI_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/ch5xx58x/memorymap.h>

/*
 * The CH58x has two SPI blocks, SPI0 and SPI1.
 *
 * SPI0's default pins are PA12 (SCS), PA13 (SCK), PA14 (MOSI) and PA15
 * (MISO); GPIO_REMAP_SPI0 moves SCK/MOSI/MISO to PB13/PB14/PB15.  SPI1 shares
 * its interrupt with PWMX and its pins are selected by the PWMX remap.  The
 * chip select is a plain GPIO in both cases: configure it with
 * gpio_set_mode() and drive it with gpio_set()/gpio_clear().  Select the
 * alternate function with gpio_pin_remap(GPIO_REMAP_SPI0) and enable the
 * SCK/MOSI (master) or MISO (slave) output drivers with spi_enable().
 */
#define SPI0				SPI0_BASE
#define SPI1				SPI1_BASE

#include <libopenwch/ch5xx58x/common/spi_common_v1.h>

#endif
/**@}*/
