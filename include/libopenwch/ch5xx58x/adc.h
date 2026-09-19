/** @defgroup adc_defines ADC Defines

@brief <b>Analog to Digital Converter and touch key for the CH58x</b>

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

#ifndef LIBOPENWCH_ADC_H
#define LIBOPENWCH_ADC_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/ch5xx58x/memorymap.h>

/*
 * The CH58x ADC is not a peripheral on its own bus: its registers are bytes
 * and halfwords inside the SYS window, so the "base address" passed to every
 * adc_*() function is SYS_BASE and the accessors add the documented offset.
 *
 * There is exactly one ADC, so ADC is the obvious name for it, but the base is
 * still passed explicitly to keep the driver usable from code that only knows
 * the SYS window.
 */
#define ADC_BASE			SYS_BASE
#define ADC				ADC_BASE

#include <libopenwch/ch5xx58x/common/adc_common_v1.h>

#endif
/**@}*/
