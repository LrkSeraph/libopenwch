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
 * Device dispatch for the generated per-family IRQ definitions.  Include this
 * only through <libopenwch/qingke/nvic.h>.
 *
 * The family is selected by the -D<FAMILY> define that genlink-config.mk adds
 * to CPPFLAGS (for applications) or that the family Makefile adds when the
 * library itself is built.
 */

#ifndef LIBOPENWCH_QINGKE_NVIC_H
#error You should not be including this file directly, but <libopenwch/qingke/nvic.h>
#endif

#if defined(CH32V0)
#include <libopenwch/ch32v0/nvic.h>
#elif defined(CH5XX58X)
#include <libopenwch/ch5xx58x/nvic.h>
#elif defined(CH5XX57X)
#include <libopenwch/ch5xx57x/nvic.h>
#elif defined(CH5XX59X)
#include <libopenwch/ch5xx59x/nvic.h>
#elif defined(CH32V0V4)
#include <libopenwch/ch32v0v4/nvic.h>
#elif defined(CH32X0)
#include <libopenwch/ch32x0/nvic.h>
#elif defined(CH32L1)
#include <libopenwch/ch32l1/nvic.h>
#else
#warning                                                                       \
    "no IRQ definitions for the selected device; interrupts are unavailable"
#define OPENWCH_IRQ_COUNT 0
#define OPENWCH_IRQ_FIRST 16
#define OPENWCH_IRQ_LAST 15
#endif
