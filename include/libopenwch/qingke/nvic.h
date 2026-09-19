/** @defgroup qingke_nvic_defines NVIC

@brief <b>Nested interrupt control for WCH QingKe RISC-V cores (PFIC)</b>

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

#ifndef LIBOPENWCH_QINGKE_NVIC_H
#define LIBOPENWCH_QINGKE_NVIC_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/qingke/pfic.h>

/*
 * There is no ARM NVIC on a WCH RISC-V part -- the interrupt controller is the
 * QingKe PFIC.  Because the PFIC register block mirrors the NVIC layout, the
 * libopencm3 nvic_*() API is kept verbatim so that code written for
 * libopencm3 needs no changes.  Read "NVIC" as "PFIC" throughout.
 *
 * NVIC_IRQ_COUNT is provided by the per-device generated nvic.h, which is why
 * the dispatch header is included here rather than the other way round.
 */

BEGIN_DECLS

void nvic_enable_irq(uint8_t irqn);
void nvic_disable_irq(uint8_t irqn);
uint8_t nvic_get_pending_irq(uint8_t irqn);
void nvic_set_pending_irq(uint8_t irqn);
void nvic_clear_pending_irq(uint8_t irqn);
uint8_t nvic_get_active_irq(uint8_t irqn);
uint8_t nvic_get_irq_enabled(uint8_t irqn);

/*
 * Priority: the QingKe PFIC uses one byte per interrupt, where bit 7 is the
 * pre-emption level and bits 6:4 are the sub-priority.  nvic_set_priority()
 * therefore takes the raw byte; the LIBOPENWCH_NVIC_PRIO_* helpers below build
 * it.
 */
void nvic_set_priority(uint8_t irqn, uint8_t priority);
uint8_t nvic_get_priority(uint8_t irqn);

/*
 * Enable or disable interrupt nesting (the QingKe INTSYSCR.INEST bit).  With
 * nesting enabled, a higher-priority interrupt may pre-empt a running handler;
 * with it disabled, a running handler blocks every interrupt of the same or
 * lower pre-emption level.
 */
void nvic_enable_irq_nesting(void);
void nvic_disable_irq_nesting(void);

/* Interrupt threshold: interrupts at or below this pre-emption level are
 * blocked.  0 disables the threshold. */
void nvic_set_irq_threshold(uint8_t threshold);

/* Fast-interrupt channels 0..3: route an IRQ straight to a handler address
 * without going through the vector table. */
void nvic_enable_fast_irq(uint8_t channel, uint8_t irqn, void (*handler)(void));
void nvic_disable_fast_irq(uint8_t channel);

/* Trigger a software interrupt (IRQ 14). */
void nvic_generate_software_interrupt(void);

END_DECLS

/* Build a priority byte: pre-emption level 0..15, sub-priority 0..7. */
#define OPENWCH_NVIC_PRIO(preempt, sub)                                        \
	((uint8_t)((((preempt) & 0xf) << 4) | (((sub) & 0x7) << 1)))

/* Include the per-device generated IRQ numbers, if available.  The pattern
 * mirrors libopencm3's cm3/nvic.h -> dispatch/nvic.h chain. */
#include <libopenwch/dispatch/nvic.h>

#endif
/**@}*/
