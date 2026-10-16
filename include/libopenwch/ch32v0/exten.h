/** @defgroup exten_defines EXTEN Defines

@brief <b>Extended configuration unit for the CH32V00x</b>

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

#ifndef LIBOPENWCH_EXTEN_H
#define LIBOPENWCH_EXTEN_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/ch32v0/memorymap.h>

/*
 * The EXTEN block has one control register at 0x40023800.  CH32V003RM chapter
 * 17 documents LDOTRIM, the lockup monitor and the op-amp mux bits in it.
 * The OPA-specific bits remain in opa.h; this header owns the unit itself and
 * the lockup/LDO fields.
 */
#define EXTEN EXTEN_BASE
#define EXTEN_CTR(exten) MMIO32((exten) + 0x00u)

/* EXTEN_CTR bits (CH32V003RM section 17.2.1). */
#define EXTEN_CTR_LOCKUP_EN (1u << 6) /**< lockup monitor and reset enable */
#define EXTEN_CTR_LOCKUP_RSTF                                                  \
	(1u << 7) /**< lockup reset flag, write 1 to clear */
#define EXTEN_CTR_LDO_TRIM (1u << 10) /**< LDO trim: 0 normal, 1 boost */

#include <libopenwch/ch32v0/common/exten_common_v1.h>

#endif
/**@}*/
