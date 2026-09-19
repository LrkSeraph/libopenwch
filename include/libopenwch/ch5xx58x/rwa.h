/** @defgroup rwa_defines RWA Defines

@brief <b>Safe-access unlocking for the CH58x write-protected registers</b>

@ingroup CH5XX58X_defines

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

#ifndef LIBOPENWCH_RWA_H
#define LIBOPENWCH_RWA_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/qingke/csr.h>
#include <libopenwch/ch5xx58x/memorymap.h>

/*
 * On the CH58x most system, clock, power, GPIO and flash-controller registers
 * are marked RWA in the reference manual: reads always work, but writes are
 * ignored unless the part has been unlocked first.
 *
 * Unlocking means writing 0x57 then 0xA8, in that order, to the byte register
 * R8_SAFE_ACCESS_SIG.  The window that opens is the thing to be careful about:
 *
 *	"After entering safe access mode, about 16 system frequency cycles are
 *	 in safe mode, and one or more secure registers can be rewritten within
 *	 the valid period.  The safe mode will be automatically terminated after
 *	 the above validity period is exceeded."
 *					-- WCH CH58x reference manual
 *
 * Sixteen system clocks is not enough time to do a read-modify-write of a
 * register plus anything else, so a single unlock/lock pair is only valid for
 * ONE register operation.  The RWA_* macros below enforce that: each one wraps
 * exactly one access in its own window.
 *
 * Interrupts must be masked across the window, because an ISR that itself
 * touches an RWA register would extend or corrupt it.  rwa_unlock() disables
 * them and returns the previous mstatus, and rwa_lock() restores it.
 *
 * NOTE: the CH58x and CH32V00x share the programming model but not the timing,
 * and ch32fun's single-macro equivalent does not mask interrupts.  libopenwch
 * follows WCH's own EVT, which does.
 */

/* Safe-access signature bytes, written in this order. */
#define SAFE_ACCESS_SIG1		0x57
#define SAFE_ACCESS_SIG2		0xa8
#define SAFE_ACCESS_SIG0		0x00

/* Status bits readable back from R8_SAFE_ACCESS_SIG. */
#define RWA_STATUS_MODE			0x03	/**< 0b11 = unlocked */
#define RWA_STATUS_ACT			0x08	/**< 1 = writes accepted */
#define RWA_STATUS_TIMER		0x70	/**< remaining window */

/*
 * always_inline is REQUIRED, not an optimisation hint.
 *
 * The window opened by the 0xA8 signature lasts about sixteen system clock
 * cycles.  If these helpers were emitted as real functions, the window would
 * have to survive a `ret`, a return to the caller and the address computation
 * before the register store even issued -- which is not guaranteed to fit, and
 * on a slow clock certainly does not.  Inlining keeps the store a few
 * instructions behind the signature.
 */
#define RWA_INLINE	__attribute__((always_inline)) static inline

BEGIN_DECLS

/*
 * Open a safe-access window and return the previous mstatus so that
 * rwa_lock() can restore it.  Returns 0 if interrupts were already disabled,
 * in which case rwa_lock() must not re-enable them.
 */
RWA_INLINE uint32_t rwa_unlock(void) {
	uint32_t saved = 0;
	uint32_t mstatus = mstatus_read();

	if (mstatus & CSR_MSTATUS_MIE) {
		saved = mstatus;
		OPENWCH_CSR_CLEAR_NUM(0x300, CSR_MSTATUS_MIE);
	}

	/* The store must reach the peripheral before the signature completes. */
	qingke_fence_i();

	/* The order matters: 0x57 then 0xA8.  Any other sequence is ignored. */
	MMIO8(R8_SAFE_ACCESS_SIG) = SAFE_ACCESS_SIG1;
	MMIO8(R8_SAFE_ACCESS_SIG) = SAFE_ACCESS_SIG2;

	qingke_fence_i();

	return saved;
}

/** Close the window opened by rwa_unlock() and restore the interrupt state. */
RWA_INLINE void rwa_lock(uint32_t saved) {
	MMIO8(R8_SAFE_ACCESS_SIG) = SAFE_ACCESS_SIG0;

	if (saved & CSR_MSTATUS_MIE) {
		OPENWCH_CSR_SET_NUM(0x300, CSR_MSTATUS_MIE);
	}

	qingke_fence_i();
}

/** True while a safe-access window is open. */
RWA_INLINE bool rwa_is_unlocked(void) {
	return (MMIO8(R8_SAFE_ACCESS_SIG) & RWA_STATUS_MODE) == RWA_STATUS_MODE;
}

END_DECLS

/*
 * Non-inline entry points for callers that are NOT timing sensitive, i.e.
 * that only want to observe a status bit.  Never use these around a register
 * write: the window would expire during the call.
 */
void rwa_open(void);
void rwa_close(void);

/*
 * One RWA register operation per window.
 *
 * These are macros rather than functions on purpose: a function call would eat
 * most of the sixteen-clock budget before the register write happened, and a
 * caller-supplied callback could not be bounded at all.
 */

/** Write `value` to the RWA register `reg`. */
#define RWA_WRITE(reg, value) \
	do { \
		uint32_t rwa_saved_ = rwa_unlock(); \
		(reg) = (value); \
		rwa_lock(rwa_saved_); \
	} while (0)

/** Set the bits of `bits` in the RWA register `reg`. */
#define RWA_SET_BITS(reg, bits) \
	do { \
		uint32_t rwa_saved_ = rwa_unlock(); \
		(reg) |= (bits); \
		rwa_lock(rwa_saved_); \
	} while (0)

/** Clear the bits of `bits` in the RWA register `reg`. */
#define RWA_CLEAR_BITS(reg, bits) \
	do { \
		uint32_t rwa_saved_ = rwa_unlock(); \
		(reg) &= ~(bits); \
		rwa_lock(rwa_saved_); \
	} while (0)

/** Read-modify-write: replace `mask` in `reg` with `value`. */
#define RWA_MODIFY(reg, mask, value) \
	do { \
		uint32_t rwa_saved_ = rwa_unlock(); \
		uint32_t rwa_tmp_ = (reg); \
		rwa_tmp_ = (rwa_tmp_ & ~(uint32_t)(mask)) | \
			   ((uint32_t)(value) & (uint32_t)(mask)); \
		(reg) = rwa_tmp_; \
		rwa_lock(rwa_saved_); \
	} while (0)

/** 8-bit RWA write, for the byte-wide views of the same registers. */
#define RWA_WRITE8(reg, value) \
	do { \
		uint32_t rwa_saved_ = rwa_unlock(); \
		(reg) = (value); \
		rwa_lock(rwa_saved_); \
	} while (0)

#endif
/**@}*/
