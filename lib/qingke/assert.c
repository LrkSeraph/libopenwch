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

/** @defgroup qingke_assert_file Assertions
 *
 * @ingroup qingke_file
 *
 * @brief <b>Weak assertion failure hooks</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * An application overrides these to print diagnostics; the defaults just stop.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/qingke/assert.h>

__attribute__((weak)) void openwch_assert_failed(void) {
	while (1) {
		;
	}
}

__attribute__((weak)) void
openwch_assert_failed_verbose(const char *file __attribute__((unused)),
			      int line __attribute__((unused)),
			      const char *func __attribute__((unused)),
			      const char *assert_expr __attribute__((unused))) {
	openwch_assert_failed();
}
/**@}*/
