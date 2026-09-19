/** @defgroup qingke_systick_defines SysTick

@brief <b>WCH QingKe system timer</b>

@ingroup qingke_defines

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

#ifndef LIBOPENWCH_QINGKE_SYSTICK_H
#define LIBOPENWCH_QINGKE_SYSTICK_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/qingke/memorymap.h>

/*
 * WCH's SysTick is NOT the RISC-V `mtime` and not the ARM SysTick either; it
 * is a vendor timer at 0xE000F000 with a 64-bit free-running counter and
 * compare value on the V4-class cores, and a 32-bit pair on the V2-class
 * cores.  Both views are exposed here.
 *
 * The counter is not tied to a fixed clock: SYSTICK_CTLR_CLKSRC selects either
 * HCLK or HCLK/8.  qingke_systick_set_frequency() takes the resulting rate so
 * that the delay helpers can convert to microseconds.
 */

typedef struct {
	volatile uint32_t ctlr;	  /**< 0x00 control */
	volatile uint32_t sr;	  /**< 0x04 status */
	volatile uint32_t cnt;	  /**< 0x08 counter (low word) */
	volatile uint32_t cnt_hi; /**< 0x0c counter high word */
	volatile uint32_t cmp;	  /**< 0x10 compare (low word) */
	volatile uint32_t cmp_hi; /**< 0x14 compare high word */
} systick_reg_t;

#define SYSTICK ((systick_reg_t *)SYS_TICK_BASE)

/* ctlr bits */
#define SYSTICK_CTLR_EN (1 << 0)	 /**< counter enable */
#define SYSTICK_CTLR_IE (1 << 1)	 /**< interrupt enable */
#define SYSTICK_CTLR_CLKSRC (1 << 2)	 /**< 0 = HCLK/8, 1 = HCLK */
#define SYSTICK_CTLR_AUTORELOAD (1 << 3) /**< auto reload on compare */
#define SYSTICK_CTLR_INIT (1 << 5)	 /**< initialise / reload */
#define SYSTICK_CTLR_MODE (1 << 31)	 /**< 1 = one-shot */

/* sr bits */
#define SYSTICK_SR_CMP (1 << 0) /**< compare match */
#define SYSTICK_SR_OV (1 << 1)	/**< counter overflow */

BEGIN_DECLS

void systick_set_reload(uint32_t value);
void systick_set_clock_source(uint32_t source);
void systick_enable_counter(void);
void systick_disable_counter(void);
void systick_enable_interrupt(void);
void systick_disable_interrupt(void);
void systick_clear_interrupt(void);

/* Current counter value as a 64-bit free-running tick. */
uint64_t systick_get_counter(void);

/* Raw compare register access. */
uint32_t systick_get_value(void);

/*
 * Tell the library how fast the counter runs, so that the delay helpers can
 * convert time to ticks.  `hz` is the counter clock, i.e. the system clock or
 * system clock / 8 depending on SYSTICK_CTLR_CLKSRC.
 */
void qingke_systick_set_frequency(uint32_t hz);
uint32_t qingke_systick_get_frequency(void);

void qingke_delay_us(uint32_t us);
void qingke_delay_ms(uint32_t ms);

END_DECLS

#endif
/**@}*/
