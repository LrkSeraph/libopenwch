/** @defgroup flash_defines FLASH Defines

@brief <b>Internal Flash Memory Controller for the CH32V00x</b>

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

#ifndef LIBOPENWCH_FLASH_H
#define LIBOPENWCH_FLASH_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/ch32v0/memorymap.h>

/*
 * The CH32V00x program/erase controller lives at FLASH_R_BASE on the AHB bus.
 * The flash itself is visible at the start of the address space (and aliased
 * at 0x08000000); the option byte block is at OB_BASE.
 */
#define FLASH				FLASH_R_BASE

#include <libopenwch/ch32v0/common/flash_common_v1.h>

#endif
/**@}*/
