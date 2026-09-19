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

/** @addtogroup pwr_file PWR
 *
 * @ingroup CH32V0
 *
 * @brief <b>Power Control for the CH32V00x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * The PVD compares VDD against one of eight thresholds and mirrors the result
 * in CSR.PVDO, which can also be routed to EXTI line 8.  The auto-wakeup unit
 * counts LSI cycles and wakes the part from standby; its event is available on
 * EXTI line 9.
 *
 * pwr_enter_standby_mode() sets PDDS, sets SLEEPDEEP in the PFIC system
 * control register and executes WFI.  A standby wakeup behaves like a reset,
 * so the SLEEPDEEP clear only matters if the part is woken before standby is
 * entered.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch32v0/pwr.h>
#include <libopenwch/qingke/pfic.h>
#include <libopenwch/qingke/assert.h>

void pwr_enable_pvd(uint32_t pwr) {
	PWR_CTLR(pwr) |= PWR_CTLR_PVDE;
}

void pwr_disable_pvd(uint32_t pwr) {
	PWR_CTLR(pwr) &= ~PWR_CTLR_PVDE;
}

void pwr_set_pvd_level(uint32_t pwr, enum pwr_pvd_level level) {
	PWR_CTLR(pwr) = (PWR_CTLR(pwr) & ~PWR_CTLR_PLS_MASK)
			| ((uint32_t)level & PWR_CTLR_PLS_MASK);
}

void pwr_enable_auto_wakeup(uint32_t pwr) {
	PWR_AWUCSR(pwr) |= PWR_AWUCSR_AWUEN;
}

void pwr_disable_auto_wakeup(uint32_t pwr) {
	PWR_AWUCSR(pwr) &= ~PWR_AWUCSR_AWUEN;
}

void pwr_set_awu_prescaler(uint32_t pwr, enum pwr_awu_prescaler prescaler) {
	PWR_AWUPSC(pwr) = (PWR_AWUPSC(pwr) & ~PWR_AWUPSC_MASK)
			| ((uint32_t)prescaler & PWR_AWUPSC_MASK);
}

void pwr_set_awu_window(uint32_t pwr, uint8_t window) {
	openwch_assert((window & ~PWR_AWUWR_MASK) == 0);

	PWR_AWUWR(pwr) = (PWR_AWUWR(pwr) & ~PWR_AWUWR_MASK)
			| ((uint32_t)window & PWR_AWUWR_MASK);
}

void pwr_enter_standby_mode(uint32_t pwr) {
	PWR_CTLR(pwr) |= PWR_CTLR_PDDS;

	/* SLEEPDEEP selects standby rather than sleep on the WFI below. */
	PFIC->sctlr |= PFIC_SCTLR_SLEEPDEEP;
	__asm__ volatile ("wfi");
	PFIC->sctlr &= ~PFIC_SCTLR_SLEEPDEEP;
}

uint32_t pwr_get_flag(uint32_t pwr, uint32_t flag) {
	return PWR_CSR(pwr) & flag;
}
/**@}*/
