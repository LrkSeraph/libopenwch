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

/** @defgroup mini_libc_file Mini libc
 *
 * @brief A handful of freestanding C string/memory routines.
 *
 * libopenwch itself needs no C library at all: the archives reference only
 * libgcc (for `__mulsi3`, `__udivsi3` and friends).  Applications, however,
 * commonly call `memcpy()`, `memset()` and `strlen()`, and some toolchains --
 * notably Debian's `gcc-riscv64-unknown-elf` -- ship no newlib for the
 * `rv32e`/`rv32imac` multilibs at all, so those calls cannot be satisfied.
 *
 * These routines fill that gap.  They are compiled into one archive per
 * family, `lib/libopenwch_mini_libc_<family>.a`, and are linked only when the
 * application template is asked for with `LIBOPENWCH_NOSTDLIB=1`.  Without
 * that switch the toolchain's own C library is used and this archive is
 * ignored, so the two never shadow each other by accident.
 *
 * The implementations are deliberately simple byte loops.  They are compiled
 * with `-fno-builtin`: without it GCC recognises the loop in `memcpy()` and
 * helpfully replaces it with a call to `memcpy()`, which is infinite
 * recursion.
 *
 * @ingroup libopenwch
 * @{
 */

#ifndef LIBOPENWCH_MINI_LIBC_H
#define LIBOPENWCH_MINI_LIBC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memchr(const void *s, int c, size_t n);

size_t strlen(const char *s);
size_t strnlen(const char *s, size_t maxlen);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
char *strchr(const char *s, int c);

#ifdef __cplusplus
}
#endif

/**@}*/

#endif /* LIBOPENWCH_MINI_LIBC_H */
