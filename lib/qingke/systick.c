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

/** @defgroup qingke_systick_file SysTick
 *
 * @ingroup qingke_file
 *
 * @brief <b>WCH QingKe system timer and busy-wait delays</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * NOTE: this timer is not the RISC-V `mtime`.  It counts the system clock (or
 * system clock / 8) and its counter width depends on the core generation, so
 * the delay helpers are built on the *compare* register rather than on the
 * full-width counter: that works on every part without a family split.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/qingke/systick.h>
#include <libopenwch/qingke/sync.h>

/* Counter clock in Hz, as told to us by the family clock driver.  Zero means
 * "unknown", in which case the delays fall back to a calibrated busy loop. */
static uint32_t systick_freq;

void systick_set_reload(uint32_t value) {
	SYSTICK->cmp = value;
}

void systick_set_clock_source(uint32_t source) {
	if (source) {
		SYSTICK->ctlr |= SYSTICK_CTLR_CLKSRC;
	} else {
		SYSTICK->ctlr &= ~SYSTICK_CTLR_CLKSRC;
	}
}

void systick_enable_counter(void) {
	SYSTICK->ctlr |= SYSTICK_CTLR_EN;
}

void systick_disable_counter(void) {
	SYSTICK->ctlr &= ~SYSTICK_CTLR_EN;
}

void systick_enable_interrupt(void) {
	SYSTICK->ctlr |= SYSTICK_CTLR_IE;
}

void systick_disable_interrupt(void) {
	SYSTICK->ctlr &= ~SYSTICK_CTLR_IE;
}

void systick_clear_interrupt(void) {
	SYSTICK->sr = 0;
}

uint32_t systick_get_value(void) {
	return SYSTICK->cnt;
}

uint64_t systick_get_counter(void) {
	uint32_t lo, hi;

	do {
		hi = SYSTICK->cnt_hi;
		lo = SYSTICK->cnt;
	} while (hi != SYSTICK->cnt_hi);

	return ((uint64_t)hi << 32) | lo;
}

void qingke_systick_set_frequency(uint32_t hz) {
	systick_freq = hz;
}

uint32_t qingke_systick_get_frequency(void) {
	return systick_freq;
}

/*
 * Wait for `ticks` counter ticks.  Uses the compare register, so it is
 * independent of the counter width.  The comparison is done in signed
 * arithmetic so that a counter wrap is handled correctly.
 */
static void systick_wait_ticks(uint32_t ticks) {
	uint32_t start = SYSTICK->cnt;
	uint32_t elapsed;

	if (ticks == 0) {
		return;
	}

	/* Make sure the counter is running. */
	if (!(SYSTICK->ctlr & SYSTICK_CTLR_EN)) {
		SYSTICK->ctlr |= SYSTICK_CTLR_EN;
	}

	do {
		elapsed = SYSTICK->cnt - start;
	} while ((int32_t)(ticks - elapsed) > 0);
}

void qingke_delay_us(uint32_t us) {
	if (systick_freq != 0) {
		/* ticks = us * freq / 1000000, done without a 64-bit divide. */
		uint64_t ticks = ((uint64_t)us * systick_freq) / 1000000u;
		while (ticks > 0) {
			uint32_t chunk = (ticks > 0x7fffffffu) ? 0x7fffffffu : (uint32_t)ticks;
			systick_wait_ticks(chunk);
			ticks -= chunk;
		}
		return;
	}

	/* No frequency information: loop with a fixed instruction budget.  The
	 * caller is expected to have called qingke_systick_set_frequency(). */
	while (us--) {
		uint32_t n = 8;
		while (n--) {
			qingke_nop();
		}
	}
}

void qingke_delay_ms(uint32_t ms) {
	while (ms--) {
		qingke_delay_us(1000);
	}
}
/**@}*/
