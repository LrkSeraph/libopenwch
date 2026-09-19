/** @defgroup dbgmcu_defines DBGMCU Defines

@brief <b>Debug MCU for the CH32V00x</b>

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

#ifndef LIBOPENWCH_DBGMCU_H
#define LIBOPENWCH_DBGMCU_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/ch32v0/memorymap.h>

/*
 * The CH32V00x debug unit is not memory-mapped.  Its control register is the
 * machine-mode CSR 0x7c0 and its identification word is in the factory block
 * at 0x1ffff7c4, so the API takes no base address -- there is only one
 * instance.  See common/dbgmcu_common_v1.h for the details.
 */

#include <libopenwch/ch32v0/common/dbgmcu_common_v1.h>

#endif
/**@}*/
