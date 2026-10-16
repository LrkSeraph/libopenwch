/** @defgroup esig_defines ESIG Defines

@brief <b>Electronic signature for the CH32V00x</b>

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

#ifndef LIBOPENWCH_ESIG_H
#define LIBOPENWCH_ESIG_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/ch32v0/memorymap.h>

/*
 * The electronic-signature block is factory-programmed read-only data.  The
 * CH32V003RM documents the flash-capacity word at 0x1ffff7e0 and a 96-bit
 * unique ID at 0x1ffff7e8.  See common/esig_common_v1.h for the API.
 */

#define ESIG ESIG_BASE
#define ESIG_FLACAP MMIO16(ESIG_BASE + 0x00u)
#define ESIG_UNIID1 MMIO32(ESIG_BASE + 0x08u)
#define ESIG_UNIID2 MMIO32(ESIG_BASE + 0x0cu)
#define ESIG_UNIID3 MMIO32(ESIG_BASE + 0x10u)

#include <libopenwch/ch32v0/common/esig_common_v1.h>

#endif
/**@}*/
