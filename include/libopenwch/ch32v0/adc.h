/** @defgroup adc_defines ADC Defines

@brief <b>Analog to Digital Converter for the CH32V00x</b>

@ingroup CH32V0_defines

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
#include <libopenwch/ch32v0/memorymap.h>

/*
 * The CH32V00x family has a single 12-bit ADC, ADC1, on APB2.  There is no
 * dual mode and no ADC2.
 *
 * There is no separate "independent mode" selection: the block is always
 * independent.  The analog inputs are alternate functions, so the pin has to
 * be switched to analog input mode with gpio_set_mode(..., GPIO_MODE_ANALOG)
 * before it is converted.  Channel 8 is the internal Vrefint and channel 9
 * the internal Vcalint calibration voltage.
 *
 * External triggers are routed through AFIO; the ADC1_ETRGREG and ADC1_ETRGINJ
 * remap bits in AFIO_PCFR1 move the trigger inputs to alternate pins.
 */
#define ADC1 ADC1_BASE

#include <libopenwch/ch32v0/common/adc_common_v1.h>

#endif
/**@}*/
