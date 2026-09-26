/** @defgroup spi_defines SPI Defines

@brief <b>Serial Peripheral Interface for the CH32V00x</b>

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

#ifndef LIBOPENWCH_SPI_H
#define LIBOPENWCH_SPI_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/ch32v0/memorymap.h>

/*
 * The CH32V00x has a single SPI instance, SPI1, on APB2.  Its default pins
 * are PC5 (SCK), PC6 (MOSI), PC7 (MISO) and PC1 (NSS); the remap bit
 * AFIO_PCFR1_SPI1_REMAP selects the alternate pin mapping documented by WCH.
 * Configure the pins with gpio_set_mode(..., GPIO_MODE_AF_PP), enable the
 * clock with the RCC driver, and leave the remap bit clear for the PC pins.
 */
#define SPI1 SPI1_BASE

#include <libopenwch/ch32v0/common/spi_common_v1.h>

#endif
/**@}*/
