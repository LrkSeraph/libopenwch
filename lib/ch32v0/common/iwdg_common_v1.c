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

/** @addtogroup iwdg_file IWDG
 *
 * @ingroup CH32V0
 *
 * @brief <b>Independent Watchdog for the CH32V00x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * The prescaler and reload registers are only writable after the 0x5555 key,
 * and the writes only take effect once the matching STATR update flag clears.
 * The typical bring-up sequence is therefore:
 *
 *	iwdg_write_access_enable(IWDG);
 *	iwdg_set_prescaler(IWDG, IWDG_PSCR_DIV64);
 *	iwdg_set_reload(IWDG, 0x0fff);
 *	iwdg_reload_counter(IWDG);
 *	iwdg_enable(IWDG);
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch32v0/iwdg.h>
#include <libopenwch/qingke/assert.h>

void iwdg_write_access_enable(uint32_t iwdg)
{
	IWDG_CTLR(iwdg) = IWDG_KEY_WRITE_ACCESS_ENABLE;
}

void iwdg_write_access_disable(uint32_t iwdg)
{
	IWDG_CTLR(iwdg) = IWDG_KEY_WRITE_ACCESS_DISABLE;
}

void iwdg_set_prescaler(uint32_t iwdg, uint8_t prescaler)
{
	openwch_assert(prescaler <= IWDG_PSCR_DIV256);

	IWDG_PSCR(iwdg) = (uint32_t)prescaler & IWDG_PSCR_PR_MASK;
}

void iwdg_set_reload(uint32_t iwdg, uint16_t reload)
{
	openwch_assert(reload <= IWDG_RLDR_RL_MASK);

	IWDG_RLDR(iwdg) = (uint32_t)reload & IWDG_RLDR_RL_MASK;
}

void iwdg_reload_counter(uint32_t iwdg)
{
	IWDG_CTLR(iwdg) = IWDG_KEY_RELOAD;
}

void iwdg_enable(uint32_t iwdg)
{
	IWDG_CTLR(iwdg) = IWDG_KEY_ENABLE;
}

uint32_t iwdg_get_flag(uint32_t iwdg, uint16_t flag)
{
	return IWDG_STATR(iwdg) & (uint32_t)flag;
}
/**@}*/
