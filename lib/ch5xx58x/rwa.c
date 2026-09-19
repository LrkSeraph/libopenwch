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

/** @defgroup rwa_file RWA
 *
 * @ingroup CH5XX58X
 *
 * @brief <b>Safe-access unlocking for the CH58x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * The real work is in the header, as static inline functions, because the
 * sixteen-clock window cannot absorb a call.  These non-inline wrappers exist
 * for code that wants to hold a window open across more than one instruction
 * (for example a driver that must re-read a status register mid-sequence
 * without the compiler reordering), and for a stable symbol to set a
 * breakpoint on.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch5xx58x/rwa.h>

/*
 * These share the same static storage as the inlined form, so a header-only
 * user and a library user cannot disagree about the interrupt state.
 */
static uint32_t rwa_saved_state;

void rwa_open(void) {
	rwa_saved_state = rwa_unlock();
}

void rwa_close(void) {
	rwa_lock(rwa_saved_state);
	rwa_saved_state = 0;
}
/**@}*/
