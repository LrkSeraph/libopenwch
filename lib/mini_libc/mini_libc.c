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

/*
 * A minimal freestanding C library: the string and memory routines an
 * application is most likely to need, and nothing else.
 *
 * Build note: these objects MUST be compiled with -fno-builtin (see the
 * MINILIB rules in lib/Makefile.include).  GCC recognises a hand-written
 * copying loop as memcpy() and would otherwise emit a call to memcpy(),
 * turning the function into infinite recursion.
 *
 * No header beyond our own and <stddef.h> is included, and nothing here calls
 * any other library function, so the archive is self-contained.
 */

#include <libopenwch/mini_libc.h>

void *memcpy(void *dest, const void *src, size_t n) {
	unsigned char *d = dest;
	const unsigned char *s = src;

	while (n--) {
		*d++ = *s++;
	}

	return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
	unsigned char *d = dest;
	const unsigned char *s = src;

	if (d < s) {
		while (n--) {
			*d++ = *s++;
		}
	} else if (d > s) {
		d += n;
		s += n;
		while (n--) {
			*--d = *--s;
		}
	}

	return dest;
}

void *memset(void *s, int c, size_t n) {
	unsigned char *p = s;

	while (n--) {
		*p++ = (unsigned char)c;
	}

	return s;
}

int memcmp(const void *s1, const void *s2, size_t n) {
	const unsigned char *a = s1;
	const unsigned char *b = s2;

	while (n--) {
		if (*a != *b) {
			return (int)*a - (int)*b;
		}
		a++;
		b++;
	}

	return 0;
}

void *memchr(const void *s, int c, size_t n) {
	const unsigned char *p = s;

	while (n--) {
		if (*p == (unsigned char)c) {
			return (void *)p;
		}
		p++;
	}

	return 0;
}

size_t strlen(const char *s) {
	const char *p = s;

	while (*p) {
		p++;
	}

	return (size_t)(p - s);
}

size_t strnlen(const char *s, size_t maxlen) {
	size_t n = 0;

	while (n < maxlen && s[n]) {
		n++;
	}

	return n;
}

int strcmp(const char *s1, const char *s2) {
	while (*s1 && (*s1 == *s2)) {
		s1++;
		s2++;
	}

	return (int)(unsigned char)*s1 - (int)(unsigned char)*s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
	while (n && *s1 && (*s1 == *s2)) {
		s1++;
		s2++;
		n--;
	}

	if (n == 0) {
		return 0;
	}

	return (int)(unsigned char)*s1 - (int)(unsigned char)*s2;
}

char *strcpy(char *dest, const char *src) {
	char *d = dest;

	while ((*d++ = *src++) != '\0') {
		;
	}

	return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
	char *d = dest;

	while (n && *src) {
		*d++ = *src++;
		n--;
	}

	while (n--) {
		*d++ = '\0';
	}

	return dest;
}

char *strchr(const char *s, int c) {
	for (;; s++) {
		if (*s == (char)c) {
			return (char *)s;
		}
		if (!*s) {
			return 0;
		}
	}
}
