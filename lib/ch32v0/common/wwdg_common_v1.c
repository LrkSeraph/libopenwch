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

/** @addtogroup wwdg_file WWDG
 *
 * @ingroup CH32V0
 *
 * @brief <b>Window Watchdog for the CH32V00x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * WWDG_CTLR is not protected by a key: writing any value loads the counter,
 * and setting WDGA starts the watchdog.  The window value must be programmed
 * before wwdg_enable(), because once the watchdog is running only the counter
 * may be refreshed.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch32v0/wwdg.h>
#include <libopenwch/qingke/assert.h>

void wwdg_set_prescaler(uint32_t wwdg, uint32_t prescaler)
{
	openwch_assert((prescaler & ~WWDG_CFGR_WDGTB_MASK) == 0);

	WWDG_CFGR(wwdg) = (WWDG_CFGR(wwdg) & ~WWDG_CFGR_WDGTB_MASK)
			| (prescaler & WWDG_CFGR_WDGTB_MASK);
}

void wwdg_set_window(uint32_t wwdg, uint8_t window)
{
	openwch_assert((window & ~WWDG_CFGR_W_MASK) == 0);

	WWDG_CFGR(wwdg) = (WWDG_CFGR(wwdg) & ~WWDG_CFGR_W_MASK)
			| ((uint32_t)window & WWDG_CFGR_W_MASK);
}

void wwdg_enable_interrupt(uint32_t wwdg)
{
	WWDG_CFGR(wwdg) |= WWDG_CFGR_EWI;
}

void wwdg_set_counter(uint32_t wwdg, uint8_t counter)
{
	openwch_assert((counter & ~WWDG_CTLR_T_MASK) == 0);

	WWDG_CTLR(wwdg) = counter;
}

void wwdg_enable(uint32_t wwdg, uint8_t counter)
{
	openwch_assert((counter & ~WWDG_CTLR_T_MASK) == 0);

	WWDG_CTLR(wwdg) = WWDG_CTLR_WDGA | counter;
}

uint8_t wwdg_get_flag(uint32_t wwdg)
{
	return (uint8_t)(WWDG_STATR(wwdg) & WWDG_STATR_EWIF);
}

void wwdg_clear_flag(uint32_t wwdg)
{
	WWDG_STATR(wwdg) = 0;
}
/**@}*/
