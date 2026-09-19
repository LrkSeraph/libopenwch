/** @addtogroup wwdg_defines WWDG Defines

@brief <b>Defined Constants and Types for the CH32V00x WWDG</b>

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA WWDG.H
The order of header inclusion is important: wwdg.h includes the device
specific memorymap.h header before including this header file. */

/** @cond */
#if defined(LIBOPENWCH_WWDG_H) || defined(LIBOPENWCH_WWDG_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_WWDG_COMMON_V1_H
#define LIBOPENWCH_WWDG_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * The window watchdog is a 7-bit down-counter clocked from PCLK1/4096 divided
 * by the WDGTB prescaler.  The counter must be refreshed while it is between
 * the window value and 0x3f: refreshing too early or letting it fall past
 * 0x3f both reset the part.
 *
 * The counter and window fields are the low seven bits of CTLR and CFGR; the
 * prescaler lives in CFGR bits [8:7] and the early-wakeup interrupt enable in
 * bit 9.
 */

/* --- Register accessors -------------------------------------------------- */

/* Control register (WWDG_CTLR) */
#define WWDG_CTLR(wwdg)			MMIO32((wwdg) + 0x00)
/* Configuration register (WWDG_CFGR) */
#define WWDG_CFGR(wwdg)			MMIO32((wwdg) + 0x04)
/* Status register (WWDG_STATR) */
#define WWDG_STATR(wwdg)		MMIO32((wwdg) + 0x08)

/* --- WWDG_CTLR bits ------------------------------------------------------ */

#define WWDG_CTLR_T_MASK		0x7fu		/**< counter field */
#define WWDG_CTLR_WDGA			(1 << 7)	/**< activation */

/* --- WWDG_CFGR bits ------------------------------------------------------ */

#define WWDG_CFGR_W_MASK		0x7fu		/**< window field */
#define WWDG_CFGR_WDGTB_MASK		(0x3u << 7)	/**< prescaler field */
#define WWDG_CFGR_WDGTB_0		(1 << 7)
#define WWDG_CFGR_WDGTB_1		(1 << 8)
#define WWDG_CFGR_EWI			(1 << 9)	/**< early wakeup IE */

/* --- WWDG_STATR bits ----------------------------------------------------- */

#define WWDG_STATR_EWIF			(1 << 0)	/**< early wakeup flag */

/* --- Prescaler values ---------------------------------------------------- */

/** @defgroup wwdg_prescaler WWDG Prescaler Dividers
@ingroup wwdg_defines

@{*/
#define WWDG_PRESCALER_1		0x0u
#define WWDG_PRESCALER_2		WWDG_CFGR_WDGTB_0
#define WWDG_PRESCALER_4		WWDG_CFGR_WDGTB_1
#define WWDG_PRESCALER_8		(WWDG_CFGR_WDGTB_0 | WWDG_CFGR_WDGTB_1)
/**@}*/

BEGIN_DECLS

void wwdg_set_prescaler(uint32_t wwdg, uint32_t prescaler);
void wwdg_set_window(uint32_t wwdg, uint8_t window);
void wwdg_enable_interrupt(uint32_t wwdg);
void wwdg_set_counter(uint32_t wwdg, uint8_t counter);
void wwdg_enable(uint32_t wwdg, uint8_t counter);
uint8_t wwdg_get_flag(uint32_t wwdg);
void wwdg_clear_flag(uint32_t wwdg);

END_DECLS

#endif
/** @cond */
#else
#warning "wwdg_common_v1.h should not be included explicitly, only via wwdg.h"
#endif
/** @endcond */
/**@}*/
