/** @addtogroup esig_defines ESIG Defines

@brief <b>Electronic signature and factory information for the CH58x</b>

@ingroup CH5XX58X_defines

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

/** Return the one-byte SYS chip-ID register. */
uint8_t esig_get_chip_id(void);

/**
 * Copy the 64-bit factory unique ID / MAC block into @p buf.
 *
 * @p buf must have room for 8 bytes.  The WCH ROM helper returns six MAC
 * bytes and clears the final two bytes; this follows that contract.
 */
void esig_get_unique_id(uint8_t *buf);

/** Copy the six factory MAC/unique-ID bytes into @p buf. */
void esig_get_mac_address(uint8_t *buf);

/** Return the 25 C temperature-calibration word from InfoFlash. */
uint32_t esig_get_temp_cal_25c(void);

END_DECLS

#endif
/** @cond */
#else
#warning "esig_common_v1.h should not be included directly, only via esig.h"
#endif
/** @endcond */
/**@}*/
