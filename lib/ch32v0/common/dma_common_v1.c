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

/** @addtogroup dma_file DMA
 *
 * @ingroup CH32V0
 *
 * @brief <b>Direct Memory Access controller for the CH32V00x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * The CH32V00x DMA1 controller has seven independent channels.  Each channel
 * moves data between a fixed peripheral address (PADDR) and a memory address
 * (MADDR), for CNTR items of the configured word size, in whichever direction
 * CFGR.DIR selects, optionally incrementing either address after every item.
 *
 * The four interrupt flags of a channel (GIF, TCIF, HTIF, TEIF) live in the
 * controller-wide INTFR register as one four-bit group per channel; the group
 * offset is derived from the channel number.  INTFCR is the matching
 * write-1-to-clear register.
 *
 * All configuration functions take the controller base address (DMA1) and the
 * one-based channel number, so a single set of functions drives every channel.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch32v0/dma.h>
#include <libopenwch/qingke/assert.h>

/** Validate the one-based channel number shared by every entry point. */
static void dma_assert_channel(uint8_t channel) {
	openwch_assert((channel >= DMA_CHANNEL1) && (channel <= DMA_CHANNEL7));
}

void dma_channel_reset(uint32_t dma, uint8_t channel) {
	dma_assert_channel(channel);

	/* Clearing CFGR disables the channel and drops its configuration. */
	DMA_CFGR(dma, channel) = 0;
	DMA_CNTR(dma, channel) = 0;
	DMA_PADDR(dma, channel) = 0;
	DMA_MADDR(dma, channel) = 0;
	dma_clear_interrupt_pending_bit(dma, channel);
}

void dma_set_peripheral_address(uint32_t dma,
				uint8_t channel,
				uint32_t address) {
	dma_assert_channel(channel);
	DMA_PADDR(dma, channel) = address;
}

void dma_set_memory_address(uint32_t dma, uint8_t channel, uint32_t address) {
	dma_assert_channel(channel);
	DMA_MADDR(dma, channel) = address;
}

void dma_set_number_of_data(uint32_t dma, uint8_t channel, uint16_t number) {
	dma_assert_channel(channel);
	DMA_CNTR(dma, channel) = number;
}

uint16_t dma_get_number_of_data(uint32_t dma, uint8_t channel) {
	dma_assert_channel(channel);
	return (uint16_t)(DMA_CNTR(dma, channel) & 0xffffu);
}

void dma_set_read_from_peripheral(uint32_t dma, uint8_t channel) {
	dma_assert_channel(channel);
	DMA_CFGR(dma, channel) &= ~DMA_CFGR_DIR;
}

void dma_set_read_from_memory(uint32_t dma, uint8_t channel) {
	dma_assert_channel(channel);
	DMA_CFGR(dma, channel) |= DMA_CFGR_DIR;
}

void dma_enable_memory_increment_mode(uint32_t dma, uint8_t channel) {
	dma_assert_channel(channel);
	DMA_CFGR(dma, channel) |= DMA_CFGR_MINC;
}

void dma_disable_memory_increment_mode(uint32_t dma, uint8_t channel) {
	dma_assert_channel(channel);
	DMA_CFGR(dma, channel) &= ~DMA_CFGR_MINC;
}

void dma_enable_peripheral_increment_mode(uint32_t dma, uint8_t channel) {
	dma_assert_channel(channel);
	DMA_CFGR(dma, channel) |= DMA_CFGR_PINC;
}

void dma_disable_peripheral_increment_mode(uint32_t dma, uint8_t channel) {
	dma_assert_channel(channel);
	DMA_CFGR(dma, channel) &= ~DMA_CFGR_PINC;
}

void dma_set_peripheral_size(uint32_t dma, uint8_t channel, uint32_t size) {
	uint32_t reg;

	dma_assert_channel(channel);
	openwch_assert((size & ~DMA_SIZE_MASK) == 0);

	reg = DMA_CFGR(dma, channel);
	reg = (reg & ~DMA_CFGR_PSIZE_MASK) |
	      ((size & DMA_SIZE_MASK) << DMA_CFGR_PSIZE_SHIFT);
	DMA_CFGR(dma, channel) = reg;
}

void dma_set_memory_size(uint32_t dma, uint8_t channel, uint32_t size) {
	uint32_t reg;

	dma_assert_channel(channel);
	openwch_assert((size & ~DMA_SIZE_MASK) == 0);

	reg = DMA_CFGR(dma, channel);
	reg = (reg & ~DMA_CFGR_MSIZE_MASK) |
	      ((size & DMA_SIZE_MASK) << DMA_CFGR_MSIZE_SHIFT);
	DMA_CFGR(dma, channel) = reg;
}

void dma_enable_circular_mode(uint32_t dma, uint8_t channel) {
	dma_assert_channel(channel);
	DMA_CFGR(dma, channel) |= DMA_CFGR_CIRC;
}

void dma_enable_mem2mem_mode(uint32_t dma, uint8_t channel) {
	dma_assert_channel(channel);
	DMA_CFGR(dma, channel) |= DMA_CFGR_MEM2MEM;
}

void dma_disable_mem2mem_mode(uint32_t dma, uint8_t channel) {
	dma_assert_channel(channel);
	DMA_CFGR(dma, channel) &= ~DMA_CFGR_MEM2MEM;
}

void dma_set_priority(uint32_t dma, uint8_t channel, uint32_t priority) {
	uint32_t reg;

	dma_assert_channel(channel);
	openwch_assert((priority & ~DMA_PRIORITY_MASK) == 0);

	reg = DMA_CFGR(dma, channel);
	reg = (reg & ~DMA_CFGR_PL_MASK) |
	      ((priority & DMA_PRIORITY_MASK) << DMA_CFGR_PL_SHIFT);
	DMA_CFGR(dma, channel) = reg;
}

void dma_enable_transfer_complete_interrupt(uint32_t dma, uint8_t channel) {
	dma_assert_channel(channel);
	DMA_CFGR(dma, channel) |= DMA_CFGR_TCIE;
}

void dma_disable_transfer_complete_interrupt(uint32_t dma, uint8_t channel) {
	dma_assert_channel(channel);
	DMA_CFGR(dma, channel) &= ~DMA_CFGR_TCIE;
}

void dma_enable_half_transfer_interrupt(uint32_t dma, uint8_t channel) {
	dma_assert_channel(channel);
	DMA_CFGR(dma, channel) |= DMA_CFGR_HTIE;
}

void dma_disable_half_transfer_interrupt(uint32_t dma, uint8_t channel) {
	dma_assert_channel(channel);
	DMA_CFGR(dma, channel) &= ~DMA_CFGR_HTIE;
}

void dma_enable_transfer_error_interrupt(uint32_t dma, uint8_t channel) {
	dma_assert_channel(channel);
	DMA_CFGR(dma, channel) |= DMA_CFGR_TEIE;
}

void dma_disable_transfer_error_interrupt(uint32_t dma, uint8_t channel) {
	dma_assert_channel(channel);
	DMA_CFGR(dma, channel) &= ~DMA_CFGR_TEIE;
}

void dma_channel_enable(uint32_t dma, uint8_t channel) {
	dma_assert_channel(channel);
	DMA_CFGR(dma, channel) |= DMA_CFGR_EN;
}

void dma_channel_disable(uint32_t dma, uint8_t channel) {
	dma_assert_channel(channel);
	DMA_CFGR(dma, channel) &= ~DMA_CFGR_EN;
}

uint32_t dma_get_flag(uint32_t dma, uint8_t channel, uint32_t flag) {
	dma_assert_channel(channel);
	return (DMA_INTFR(dma) >> DMA_FLAG_OFFSET(channel)) &
	       (flag & DMA_FLAG_MASK);
}

void dma_clear_flag(uint32_t dma, uint8_t channel, uint32_t flag) {
	dma_assert_channel(channel);
	DMA_INTFCR(dma) = (flag & DMA_FLAG_MASK) << DMA_FLAG_OFFSET(channel);
}

uint32_t dma_get_interrupt_status(uint32_t dma, uint8_t channel) {
	dma_assert_channel(channel);
	return (DMA_INTFR(dma) >> DMA_FLAG_OFFSET(channel)) & DMA_FLAG_MASK;
}

void dma_clear_interrupt_pending_bit(uint32_t dma, uint8_t channel) {
	dma_assert_channel(channel);
	DMA_INTFCR(dma) = DMA_FLAG_MASK << DMA_FLAG_OFFSET(channel);
}
/**@}*/
