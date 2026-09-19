/** @defgroup qingke_sync_defines Synchronisation

@brief <b>Critical sections and interrupt state helpers</b>

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

#ifndef LIBOPENWCH_QINGKE_SYNC_H
#define LIBOPENWCH_QINGKE_SYNC_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/qingke/csr.h>

/*
 * Names mirror libopencm3's cm3/sync.h (cm3_irq_save / cm3_irq_restore), with
 * the qingke_ prefix.  On RISC-V the global interrupt enable is mstatus.MIE.
 */

BEGIN_DECLS

/* Enable global interrupts (sets mstatus.MIE). */
static inline void qingke_irq_enable(void) {
	OPENWCH_CSR_SET_NUM(0x300, CSR_MSTATUS_MIE);
}

/* Disable global interrupts (clears mstatus.MIE) and synchronise the
 * instruction stream, because the QingKe pipeline may have already fetched
 * instructions from the interrupt path. */
static inline void qingke_irq_disable(void) {
	OPENWCH_CSR_CLEAR_NUM(0x300, CSR_MSTATUS_MIE);
	qingke_fence_i();
}

/* Return the current mstatus value; pass it to qingke_irq_restore(). */
static inline uint32_t qingke_irq_save(void) {
	return mstatus_read();
}

/* Restore mstatus as saved by qingke_irq_save(). */
static inline void qingke_irq_restore(uint32_t state) {
	mstatus_write(state);
}

/* Disable interrupts and return the previous state. */
static inline uint32_t qingke_irq_lock(void) {
	uint32_t state = qingke_irq_save();
	qingke_irq_disable();
	return state;
}

/* Re-enable interrupts if they were enabled before qingke_irq_lock(). */
static inline void qingke_irq_unlock(uint32_t state) {
	if (state & CSR_MSTATUS_MIE) {
		qingke_irq_enable();
	}
}

/* Test whether global interrupts are currently enabled. */
static inline bool qingke_irq_enabled(void) {
	return (mstatus_read() & CSR_MSTATUS_MIE) != 0;
}

END_DECLS

#endif
/**@}*/
