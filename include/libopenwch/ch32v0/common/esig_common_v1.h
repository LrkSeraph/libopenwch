/** @addtogroup esig_defines ESIG Defines

@brief <b>Electronic signature for the CH32V00x</b>

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA ESIG.H */

/** @cond */
#if defined(LIBOPENWCH_ESIG_H) || defined(LIBOPENWCH_ESIG_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_ESIG_COMMON_V1_H
#define LIBOPENWCH_ESIG_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

BEGIN_DECLS

/** Return the factory-programmed flash capacity in KiB. */
uint16_t esig_get_flash_size_kb(void);

/**
 * Return one 32-bit word of the 96-bit unique ID.
 *
 * @param index 0..2; any other value is a programming error.
 */
uint32_t esig_get_unique_id_word(uint8_t index);

/**
 * Copy the 96-bit unique ID into @p buf as twelve little-endian bytes.
 *
 * @p buf must have room for 12 bytes.  The byte order matches the
 * WCH EVT helpers used on this family.
 */
void esig_get_unique_id(uint8_t *buf);

END_DECLS

#endif
/** @cond */
#else
#warning "esig_common_v1.h should not be included directly, only via esig.h"
#endif
/** @endcond */
/**@}*/
