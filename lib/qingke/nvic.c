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

/** @defgroup qingke_nvic_file NVIC
 *
 * @ingroup qingke_file
 *
 * @brief <b>PFIC-backed interrupt control for WCH QingKe RISC-V cores</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * The API names are libopencm3's nvic_*() on purpose: the WCH PFIC register
 * block mirrors the ARM NVIC layout, so code written against libopencm3 works
 * unchanged.  "NVIC" here means "PFIC".
 *
 * Interrupt numbering
 * -------------------
 *
 * A QingKe PFIC interrupt identifier is simultaneously
 *
 *   - the index of the handler in the vector table, and
 *   - the bit position inside the IENR/IRER/IPSR/IPRR/IACTR arrays and the
 *     byte index inside IPRIOR.
 *
 * RISC-V exceptions occupy 0..15, therefore the first external interrupt is
 * 16.  This is exactly WCH's own IRQn numbering, so the generated
 * `<PERIPH>_IRQ` values can be fed straight into nvic_enable_irq() and used as
 * `vector_table.handler[]` indices.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/qingke/nvic.h>
#include <libopenwch/qingke/pfic.h>
#include <libopenwch/qingke/csr.h>
#include <libopenwch/qingke/assert.h>

/* PFIC interrupt id -> bit inside a 32-bit bank. */
#define PFIC_BANK(id)	((id) >> 5)
#define PFIC_BIT(id)	(1u << ((id) & 0x1f))

static inline void pfic_check(uint32_t irqn) {
	openwch_assert(irqn >= OPENWCH_IRQ_FIRST && irqn <= OPENWCH_IRQ_LAST);
}

void nvic_enable_irq(uint8_t irqn) {
	pfic_check(irqn);
	PFIC->ienr[PFIC_BANK(irqn)] = PFIC_BIT(irqn);
}

void nvic_disable_irq(uint8_t irqn) {
	pfic_check(irqn);
	PFIC->irer[PFIC_BANK(irqn)] = PFIC_BIT(irqn);
	/* The QingKe pipeline may already have fetched the handler. */
	qingke_fence_i();
}

uint8_t nvic_get_irq_enabled(uint8_t irqn) {
	pfic_check(irqn);
	/*
	 * ienr is write-only on the PFIC; the enable state is read back through
	 * the interrupt-state register, which reports the same bits.
	 */
	return (PFIC->isr[PFIC_BANK(irqn)] & PFIC_BIT(irqn)) ? 1 : 0;
}

void nvic_set_pending_irq(uint8_t irqn) {
	pfic_check(irqn);
	PFIC->ipsr[PFIC_BANK(irqn)] = PFIC_BIT(irqn);
}

void nvic_clear_pending_irq(uint8_t irqn) {
	pfic_check(irqn);
	PFIC->iprr[PFIC_BANK(irqn)] = PFIC_BIT(irqn);
}

uint8_t nvic_get_pending_irq(uint8_t irqn) {
	pfic_check(irqn);
	return (PFIC->ipr[PFIC_BANK(irqn)] & PFIC_BIT(irqn)) ? 1 : 0;
}

uint8_t nvic_get_active_irq(uint8_t irqn) {
	pfic_check(irqn);
	return (PFIC->iactr[PFIC_BANK(irqn)] & PFIC_BIT(irqn)) ? 1 : 0;
}

void nvic_set_priority(uint8_t irqn, uint8_t priority) {
	pfic_check(irqn);
	/* iprior is a byte array indexed by interrupt id; bit 7 is the
	 * pre-emption level and bits 6:4 the sub-priority. */
	PFIC->iprior[irqn] = priority;
}

uint8_t nvic_get_priority(uint8_t irqn) {
	pfic_check(irqn);
	return PFIC->iprior[irqn];
}

void nvic_enable_irq_nesting(void) {
	OPENWCH_CSR_SET_NUM(CSR_INTSYSCR, INTSYSCR_INEST);
}

void nvic_disable_irq_nesting(void) {
	OPENWCH_CSR_CLEAR_NUM(CSR_INTSYSCR, INTSYSCR_INEST);
}

void nvic_set_irq_threshold(uint8_t threshold) {
	PFIC->ithresdr = threshold;
}

void nvic_enable_fast_irq(
	uint8_t channel,
	uint8_t irqn,
	void (*handler)(void)
) {
	openwch_assert(channel < 4);
	pfic_check(irqn);
	PFIC->vtcfgr[channel] = irqn;
	/* Bit 0 must be set: the entry point is a 2-byte-aligned address. */
	PFIC->vtaddr[channel] = ((uint32_t)handler & ~1u) | 1u;
}

void nvic_disable_fast_irq(uint8_t channel) {
	openwch_assert(channel < 4);
	PFIC->vtaddr[channel] = 0;
}

void nvic_generate_software_interrupt(void) {
	PFIC->sctlr |= (1u << 31);
}

void qingke_system_reset(void) {
	PFIC->cfgr = PFIC_CFGR_SYS_RESET;
	/* The reset is asynchronous; make sure nothing else executes. */
	for (;;) {
		;
	}
}
/**@}*/
