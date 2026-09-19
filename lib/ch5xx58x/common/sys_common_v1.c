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

/** @addtogroup sys_file SYS
 *
 * @ingroup CH5XX58X
 *
 * @brief <b>System control block for the CH58x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * Chip identity, reset cause, the reset-surviving byte and the PFIC interrupt
 * mask/unmask pair.
 *
 * sys_reset() goes through the PFIC software reset (qingke_system_reset())
 * rather than R8_RST_WDOG_CTRL.RB_SOFTWARE_RESET.  Both reset the part, but the
 * PFIC write needs no RWA window and is the same sequence libopenwch already
 * uses for the core reset.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch5xx58x/sys.h>
#include <libopenwch/qingke/assert.h>
#include <libopenwch/qingke/csr.h>
#include <libopenwch/qingke/pfic.h>
#include <libopenwch/qingke/systick.h>

void sys_reset(void)
{
	qingke_system_reset();
}

uint32_t sys_get_systick_count(void)
{
	return systick_get_value();
}

uint8_t sys_get_chip_id(void)
{
	return SYS_CHIP_ID;
}

sys_reset_status_t sys_get_reset_status(void)
{
	return (sys_reset_status_t)(SYS_RESET_STATUS & RB_RESET_FLAG);
}

uint8_t sys_reset_keep(void)
{
	return SYS_GLOB_RESET_KEEP;
}

void sys_set_reset_keep(uint8_t value)
{
	SYS_GLOB_RESET_KEEP = value;
}

void sys_disable_all_irq(uint32_t *saved)
{
	openwch_assert(saved != NULL);

	/*
	 * The PFIC enable state is the ISR array; bit 8 of ISR[0] is the first
	 * interrupt this part exposes after the core exceptions.  Pack the
	 * same window WCH's SYS_DisableAllIrq() does so that the state round
	 * trips through one uint32_t.
	 */
	*saved = (PFIC->isr[0] >> 8) | (PFIC->isr[1] << 24);

	PFIC->irer[0] = 0xffffffffu;
	PFIC->irer[1] = 0xffffffffu;

	qingke_fence_i();
}

void sys_recover_irq(uint32_t saved)
{
	/* IENR is write-1-to-set, so this cannot clear anything. */
	PFIC->ienr[0] = saved << 8;
	PFIC->ienr[1] = saved >> 24;
}
/**@}*/
