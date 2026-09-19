/** @defgroup debugging Debugging

@brief <b>Assertion macros for libopenwch</b>

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

#ifndef LIBOPENWCH_QINGKE_ASSERT_H
#define LIBOPENWCH_QINGKE_ASSERT_H

#include <libopenwch/qingke/common.h>

/*
 * Semantics identical to libopencm3's cm3_assert(): asserts are compiled out
 * by defining NDEBUG, and the failure hooks are weak so an application can
 * override them to print diagnostics.
 */

#define OPENWCH_LIKELY(expr)	(__builtin_expect(!!(expr), 1))

#ifdef NDEBUG
# define openwch_assert(expr)		((void)0)
# define openwch_assert_not_reached()	do { } while (1)
#else
# ifdef OPENWCH_ASSERT_VERBOSE
#  define openwch_assert(expr) \
	do { \
		if (OPENWCH_LIKELY(expr)) { \
			(void)0; \
		} else { \
			openwch_assert_failed_verbose( \
				__FILE__, __LINE__, \
				__func__, #expr); \
		} \
	} while (0)
#  define openwch_assert_not_reached() \
	openwch_assert_failed_verbose( \
			__FILE__, __LINE__, \
			__func__, 0)
# else
#  define openwch_assert(expr) \
	do { \
		if (OPENWCH_LIKELY(expr)) { \
			(void)0; \
		} else { \
			openwch_assert_failed(); \
		} \
	} while (0)
#  define openwch_assert_not_reached()	openwch_assert_failed()
# endif
#endif

BEGIN_DECLS

void openwch_assert_failed(void) __attribute__((__noreturn__));
void openwch_assert_failed_verbose(const char *file, int line, const char *func,
		const char *assert_expr) __attribute__((__noreturn__));

END_DECLS

#endif
/**@}*/
