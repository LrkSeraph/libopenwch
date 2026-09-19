/** @defgroup tmr_defines TMR Defines

@brief <b>Timer / PWM / capture units for the CH58x</b>

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

#ifndef LIBOPENWCH_TMR_H
#define LIBOPENWCH_TMR_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/ch5xx58x/memorymap.h>

/* Four identical timer blocks, each with one PWM/capture pin. */
#define TMR0 TMR0_BASE
#define TMR1 TMR1_BASE
#define TMR2 TMR2_BASE
#define TMR3 TMR3_BASE

#include <libopenwch/ch5xx58x/common/tmr_common_v1.h>

#endif
/**@}*/
