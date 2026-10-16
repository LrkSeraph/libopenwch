/** @defgroup esig_defines ESIG Defines

@brief <b>Electronic signature and factory information for the CH58x</b>

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

#ifndef LIBOPENWCH_ESIG_H
#define LIBOPENWCH_ESIG_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/ch5xx58x/memorymap.h>

/*
 * The CH58x identity data is split between the SYS chip-ID byte and the
 * factory information page in InfoFlash.  CH583DS1 sections 4.3 and 4.5
 * document R8_CHIP_ID (0x83 on CH583, 0x82 on CH582) and the factory
 * MAC/unique-ID data at ROM_CFG_MAC_ADDR (0x0007f018).
 */
#define ESIG_CHIP_ID_ADDR (SYS_BASE + 0x41u)
#define ESIG_CHIP_ID MMIO8(ESIG_CHIP_ID_ADDR)
#define ESIG_MAC_ADDR ROM_CFG_MAC_ADDR
#define ESIG_TEMP_CAL_ADDR ROM_CFG_TMP_25C

/** CH583 chip-ID value (CH583DS1 section 4.3). */
#define ESIG_CHIP_ID_CH583 0x83u
/** CH582 chip-ID value (CH583DS1 section 4.3). */
#define ESIG_CHIP_ID_CH582 0x82u

#include <libopenwch/ch5xx58x/common/esig_common_v1.h>

#endif
/**@}*/
