/** @addtogroup dbgmcu_defines DBGMCU Defines

@brief <b>Defined Constants and Types for the CH32V00x DBGMCU</b>

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA DBGMCU.H
The order of header inclusion is important: dbgmcu.h includes the device
specific memorymap.h header before including this header file. */

/** @cond */
#if defined(LIBOPENWCH_DBGMCU_H) || defined(LIBOPENWCH_DBGMCU_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_DBGMCU_COMMON_V1_H
#define LIBOPENWCH_DBGMCU_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * The debug unit is part of the core-private region at DBGMCU_BASE.  The EVT
 * device header names its two words CFGR0 and CFGR1; the first carries the
 * device/revision identifier and the second the low-power debug configuration,
 * so they are exposed here under the conventional DBGMCU_IDCODE / DBGMCU_CR
 * names.
 *
 * The configuration bits keep the watchdog and timer counters running (or
 * stopped) while the core is halted in the debugger.
 */

/* --- Register accessors -------------------------------------------------- */

/*
 * DBGMCU on the CH32V00x is NOT a memory-mapped peripheral.
 *
 * WCH's own EVT driver for this part reaches the debug control register with
 *
 *	csrr rd, 0x7c0
 *	csrw 0x7c0, rs
 *
 * and reads the revision/device id from the factory information word at
 * 0x1ffff7c4.  The CH32V003 SVD confirms this: it contains no DBGMCU
 * peripheral at all.
 *
 * NOTE on the bit layout: ch32fun's ch32v003hw.h carries a
 * DBGMCU_IWDG_STOP/TIM1_STOP set at bits 8..13, but those defines are dead
 * code copied from the memory-mapped CH32V20x/V30x DBGMCU_CR and are never
 * referenced.  The values used here are the ones WCH's EVT documents for
 * CSR 0x7c0, and they have not been confirmed on silicon.
 */

/** Factory chip id word: [15:0] device id, [31:16] revision id. */
#define DBGMCU_ID_WORD MMIO32(DBGMCU_ID_BASE)
#define DBGMCU_ID_DEVICE_MASK 0x0000ffffu
#define DBGMCU_ID_REVISION_SHIFT 16

/* --- CSR 0x7c0 (debug control) bits -------------------------------------- */
/*
 * These are the values WCH's EVT passes straight to the CSR.  Pass them to
 * dbgmcu_stop_peripheral() rather than writing the register by hand.
 */
#define DBGMCU_IWDG_STOP (1 << 0) /**< IWDG stopped when halted */
#define DBGMCU_WWDG_STOP (1 << 1) /**< WWDG stopped when halted */
#define DBGMCU_TIM1_STOP (1 << 4) /**< TIM1 stopped when halted */
#define DBGMCU_TIM2_STOP (1 << 5) /**< TIM2 stopped when halted */

BEGIN_DECLS

/* Identification (from the factory information block). */
uint32_t dbgmcu_get_revision_id(void);
uint32_t dbgmcu_get_device_id(void);

/* Raw access to the debug control register (CSR 0x7c0). */
uint32_t dbgmcu_get_control(void);
void dbgmcu_set_control(uint32_t value);

/*
 * Stop the given peripheral while the core is halted.  `mask` is one or more
 * DBGMCU_*_STOP bits; pass DBGMCU_ALL_STOP to stop every debuggable one.
 */
#define DBGMCU_ALL_STOP                                                        \
	(DBGMCU_IWDG_STOP | DBGMCU_WWDG_STOP | DBGMCU_TIM1_STOP |              \
	 DBGMCU_TIM2_STOP)

void dbgmcu_stop_peripheral(uint32_t mask);
void dbgmcu_resume_peripheral(uint32_t mask);

END_DECLS

#endif
/** @cond */
#else
#warning                                                                       \
    "dbgmcu_common_v1.h should not be included explicitly, only via dbgmcu.h"
#endif
/** @endcond */
/**@}*/
