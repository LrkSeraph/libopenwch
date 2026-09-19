/** @addtogroup iwdg_defines IWDG Defines

@brief <b>Defined Constants and Types for the CH32V00x IWDG</b>

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA IWDG.H
The order of header inclusion is important: iwdg.h includes the device
specific memorymap.h header before including this header file. */

/** @cond */
#if defined(LIBOPENWCH_IWDG_H) || defined(LIBOPENWCH_IWDG_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_IWDG_COMMON_V1_H
#define LIBOPENWCH_IWDG_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * The independent watchdog runs from the internal LSI oscillator and is
 * programmed through the key register: 0x5555 opens PSCR/RLDR for writing,
 * 0xAAAA reloads the counter and 0xCCCC starts the watchdog (after which the
 * prescaler and reload registers are frozen until the next reset).
 *
 * The counter timeout is:
 *
 *	t = (4 * 2^PR) * RL / fLSI
 */

/* --- Register accessors -------------------------------------------------- */

/* Key register (IWDG_CTLR) */
#define IWDG_CTLR(iwdg) MMIO32((iwdg) + 0x00)
/* Prescaler register (IWDG_PSCR) */
#define IWDG_PSCR(iwdg) MMIO32((iwdg) + 0x04)
/* Reload register (IWDG_RLDR) */
#define IWDG_RLDR(iwdg) MMIO32((iwdg) + 0x08)
/* Status register (IWDG_STATR) */
#define IWDG_STATR(iwdg) MMIO32((iwdg) + 0x0c)

/* --- Key values ---------------------------------------------------------- */

#define IWDG_KEY_WRITE_ACCESS_ENABLE ((uint16_t)0x5555)
#define IWDG_KEY_WRITE_ACCESS_DISABLE ((uint16_t)0x0000)
#define IWDG_KEY_RELOAD ((uint16_t)0xaaaa)
#define IWDG_KEY_ENABLE ((uint16_t)0xcccc)

/* --- Prescaler values ---------------------------------------------------- */

/** @defgroup iwdg_prescaler IWDG Prescaler Dividers
@ingroup iwdg_defines

@{*/
#define IWDG_PSCR_DIV4 0x0
#define IWDG_PSCR_DIV8 0x1
#define IWDG_PSCR_DIV16 0x2
#define IWDG_PSCR_DIV32 0x3
#define IWDG_PSCR_DIV64 0x4
#define IWDG_PSCR_DIV128 0x5
#define IWDG_PSCR_DIV256 0x6
/**@}*/

#define IWDG_PSCR_PR_MASK 0x7u
#define IWDG_RLDR_RL_MASK 0x0fffu

/* --- IWDG_STATR bits ----------------------------------------------------- */

#define IWDG_STATR_PVU (1 << 0) /**< prescaler update */
#define IWDG_STATR_RVU (1 << 1) /**< reload update */

/* --- Flag masks ---------------------------------------------------------- */

#define IWDG_FLAG_PVU IWDG_STATR_PVU
#define IWDG_FLAG_RVU IWDG_STATR_RVU

BEGIN_DECLS

void iwdg_write_access_enable(uint32_t iwdg);
void iwdg_write_access_disable(uint32_t iwdg);
void iwdg_set_prescaler(uint32_t iwdg, uint8_t prescaler);
void iwdg_set_reload(uint32_t iwdg, uint16_t reload);
void iwdg_reload_counter(uint32_t iwdg);
void iwdg_enable(uint32_t iwdg);
uint32_t iwdg_get_flag(uint32_t iwdg, uint16_t flag);

END_DECLS

#endif
/** @cond */
#else
#warning "iwdg_common_v1.h should not be included explicitly, only via iwdg.h"
#endif
/** @endcond */
/**@}*/
