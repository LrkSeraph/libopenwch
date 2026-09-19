/** @defgroup uart_defines UART Defines

@brief <b>Universal Asynchronous Receiver/Transmitters for the CH58x</b>

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

#ifndef LIBOPENWCH_UART_H
#define LIBOPENWCH_UART_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/ch5xx58x/memorymap.h>

/*
 * Four identical UART blocks, UART0..UART3.  Which pins carry RXD/TXD is not
 * decided here: it is the R16_PIN_ALTERNATE remap bit (GPIO_REMAP_UARTn in
 * gpio.h) plus the pin configuration.  Each UART has a default pin pair and
 * one alternate pair; see the CH58x datasheet.
 */
#define UART0 UART0_BASE
#define UART1 UART1_BASE
#define UART2 UART2_BASE
#define UART3 UART3_BASE

#include <libopenwch/ch5xx58x/common/uart_common_v1.h>

#endif
/**@}*/
