/** @defgroup pwm_defines PWM Defines

@brief <b>PWM4..PWM11 for the CH58x</b>

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

#ifndef LIBOPENWCH_PWM_H
#define LIBOPENWCH_PWM_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/ch5xx58x/memorymap.h>

/* The PWMX block is a single instance; there is no PWM0..PWM3 here.  The
 * TMR0..TMR3 blocks each drive one pin that is also called PWM0..PWM3, and
 * those live in tmr.h. */
#define PWMX				PWMX_BASE

#include <libopenwch/ch5xx58x/common/pwm_common_v1.h>

#endif
/**@}*/
