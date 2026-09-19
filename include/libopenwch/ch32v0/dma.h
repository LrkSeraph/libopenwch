/** @defgroup dma_defines DMA Defines

@brief <b>Defined Constants and Types for the CH32V00x DMA</b>

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

#ifndef LIBOPENWCH_DMA_H
#define LIBOPENWCH_DMA_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/ch32v0/memorymap.h>

/*
 * The CH32V00x has a single DMA controller, DMA1, with seven channels.
 * Peripheral DMA requests are hard-wired to channels: for example USART1 TX
 * uses channel 4 and USART1 RX uses channel 5.  The channel is configured
 * through the common API with DMA1 as the controller base address:
 *
 *	dma_set_peripheral_address(DMA1, DMA_CHANNEL4, (uint32_t)&USART_DATAR);
 *	dma_set_memory_address(DMA1, DMA_CHANNEL4, (uint32_t)buf);
 *	dma_channel_enable(DMA1, DMA_CHANNEL4);
 *
 * Remember to enable the DMA clock gate with rcc_periph_clock_enable() and to
 * route the request from the peripheral side.
 */
#define DMA1				DMA1_BASE

#include <libopenwch/ch32v0/common/dma_common_v1.h>

#endif
/**@}*/
