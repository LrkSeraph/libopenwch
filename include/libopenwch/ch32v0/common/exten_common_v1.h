/** @addtogroup exten_defines EXTEN Defines

@brief <b>Extended configuration unit for the CH32V00x</b>

@ingroup CH32V0_defines

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA EXTEN.H */

/** @cond */
#if defined(LIBOPENWCH_EXTEN_H) || defined(LIBOPENWCH_EXTEN_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_EXTEN_COMMON_V1_H
#define LIBOPENWCH_EXTEN_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/** LDO core-voltage mode. */
typedef enum {
	EXTEN_LDO_MODE_NORMAL = 0, /**< normal core voltage */
	EXTEN_LDO_MODE_BOOST = 1,  /**< boosted core voltage */
} exten_ldo_mode_t;

BEGIN_DECLS

/** Enable lockup monitoring; a lockup will then reset the part. */
void exten_enable_lockup_reset(uint32_t exten);
/** Disable lockup monitoring. */
void exten_disable_lockup_reset(uint32_t exten);
/** Return non-zero when the last reset was caused by a lockup. */
uint32_t exten_get_lockup_reset_flag(uint32_t exten);
/** Clear the sticky lockup-reset flag. */
void exten_clear_lockup_reset_flag(uint32_t exten);

/** Select the LDO core-voltage mode. */
void exten_set_ldo_mode(uint32_t exten, exten_ldo_mode_t mode);
/** Return the current LDO core-voltage mode. */
exten_ldo_mode_t exten_get_ldo_mode(uint32_t exten);

END_DECLS

#endif
/** @cond */
#else
#warning "exten_common_v1.h should not be included directly, only via exten.h"
#endif
/** @endcond */
/**@}*/
