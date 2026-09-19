/** @addtogroup dma_defines DMA Defines

@brief <b>Defined Constants and Types for the CH32V00x DMA</b>

@ingroup CH32V0_defines

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA DMA.H
The order of header inclusion is important: dma.h includes the device
specific memorymap.h header before including this header file. */

/** @cond */
#if defined(LIBOPENWCH_DMA_H) || defined(LIBOPENWCH_DMA_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_DMA_COMMON_V1_H
#define LIBOPENWCH_DMA_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * The CH32V00x has a single DMA controller with seven channels.  The global
 * interrupt flag registers sit at the base of the controller and the channel
 * register blocks start at offset 0x08, one every 0x14 bytes.  Each channel
 * block is four 32-bit registers:
 *
 *	CFGR	configuration
 *	CNTR	number of data to transfer (16 significant bits)
 *	PADDR	peripheral address
 *	MADDR	memory address
 *
 * The four bytes of padding between blocks are part of the memory map, so a
 * channel's registers are addressed as base + 0x08 + 0x14 * (channel - 1).
 * Every register in the block is accessed 32 bits wide, exactly as the WCH
 * device header describes it.
 */

/* --- Register accessors -------------------------------------------------- */

/** DMA interrupt flag register (global, one nibble per channel). */
#define DMA_INTFR(dma) MMIO32((dma) + 0x00)
/** DMA interrupt flag clear register (global, one nibble per channel). */
#define DMA_INTFCR(dma) MMIO32((dma) + 0x04)

/** DMA channel configuration register. */
#define DMA_CFGR(dma, channel) MMIO32((dma) + 0x08 + 0x14 * ((channel) - 1))
/** DMA channel number-of-data register. */
#define DMA_CNTR(dma, channel) MMIO32((dma) + 0x0c + 0x14 * ((channel) - 1))
/** DMA channel peripheral address register. */
#define DMA_PADDR(dma, channel) MMIO32((dma) + 0x10 + 0x14 * ((channel) - 1))
/** DMA channel memory address register. */
#define DMA_MADDR(dma, channel) MMIO32((dma) + 0x14 + 0x14 * ((channel) - 1))

/* --- INTFR / INTFCR bits ------------------------------------------------- */

/*
 * The global interrupt registers hold one four-bit group per channel, with
 * the same bit order inside every group.  The group offset is computed from
 * the one-based channel number by DMA_FLAG_OFFSET().
 */

#define DMA_GIF (1 << 0)  /**< global interrupt flag */
#define DMA_TCIF (1 << 1) /**< transfer complete flag */
#define DMA_HTIF (1 << 2) /**< half transfer flag */
#define DMA_TEIF (1 << 3) /**< transfer error flag */

/** Mask of the four flags belonging to one channel. */
#define DMA_FLAG_MASK 0x0000000fu

/** Bit offset of @p channel's four-bit flag group in INTFR / INTFCR. */
#define DMA_FLAG_OFFSET(channel) (4 * ((channel) - 1))

/* --- CFGR bits ----------------------------------------------------------- */

#define DMA_CFGR_EN (1 << 0)   /**< channel enable */
#define DMA_CFGR_TCIE (1 << 1) /**< transfer complete IE */
#define DMA_CFGR_HTIE (1 << 2) /**< half transfer IE */
#define DMA_CFGR_TEIE (1 << 3) /**< transfer error IE */
#define DMA_CFGR_DIR (1 << 4)  /**< 1 = read from memory */
#define DMA_CFGR_CIRC (1 << 5) /**< circular mode */
#define DMA_CFGR_PINC (1 << 6) /**< peripheral increment */
#define DMA_CFGR_MINC (1 << 7) /**< memory increment */

#define DMA_CFGR_PSIZE_SHIFT 8
#define DMA_CFGR_PSIZE_MASK (0x3u << 8)
#define DMA_CFGR_MSIZE_SHIFT 10
#define DMA_CFGR_MSIZE_MASK (0x3u << 10)
#define DMA_CFGR_PL_SHIFT 12
#define DMA_CFGR_PL_MASK (0x3u << 12)

#define DMA_CFGR_MEM2MEM (1 << 14) /**< memory to memory mode */

/* --- Configuration enumerations ------------------------------------------ */

/** @defgroup dma_channel DMA Channel Number
@ingroup dma_defines
@{*/
#define DMA_CHANNEL1 1
#define DMA_CHANNEL2 2
#define DMA_CHANNEL3 3
#define DMA_CHANNEL4 4
#define DMA_CHANNEL5 5
#define DMA_CHANNEL6 6
#define DMA_CHANNEL7 7
/**@}*/

/** @defgroup dma_priority DMA Channel Priority
@ingroup dma_defines
@{*/
#define DMA_PRIORITY_LOW 0x0
#define DMA_PRIORITY_MEDIUM 0x1
#define DMA_PRIORITY_HIGH 0x2
#define DMA_PRIORITY_VERY_HIGH 0x3
#define DMA_PRIORITY_MASK 0x3u
/**@}*/

/** @defgroup dma_size DMA Transfer Word Size
@ingroup dma_defines

The same two-bit encoding is used for the peripheral and the memory side, so
one constant works with both dma_set_peripheral_size() and
dma_set_memory_size().
@{*/
#define DMA_SIZE_8BIT 0x0
#define DMA_SIZE_16BIT 0x1
#define DMA_SIZE_32BIT 0x2
#define DMA_SIZE_MASK 0x3u
/**@}*/

/* --- Function prototypes ------------------------------------------------- */

BEGIN_DECLS

/* --- Channel setup ------------------------------------------------------- */

void dma_channel_reset(uint32_t dma, uint8_t channel);
void dma_set_peripheral_address(uint32_t dma,
				uint8_t channel,
				uint32_t address);
void dma_set_memory_address(uint32_t dma, uint8_t channel, uint32_t address);
void dma_set_number_of_data(uint32_t dma, uint8_t channel, uint16_t number);
uint16_t dma_get_number_of_data(uint32_t dma, uint8_t channel);

/* --- Transfer direction -------------------------------------------------- */

void dma_set_read_from_peripheral(uint32_t dma, uint8_t channel);
void dma_set_read_from_memory(uint32_t dma, uint8_t channel);

/* --- Address generation -------------------------------------------------- */

void dma_enable_memory_increment_mode(uint32_t dma, uint8_t channel);
void dma_disable_memory_increment_mode(uint32_t dma, uint8_t channel);
void dma_enable_peripheral_increment_mode(uint32_t dma, uint8_t channel);
void dma_disable_peripheral_increment_mode(uint32_t dma, uint8_t channel);

/* --- Word size ----------------------------------------------------------- */

void dma_set_peripheral_size(uint32_t dma, uint8_t channel, uint32_t size);
void dma_set_memory_size(uint32_t dma, uint8_t channel, uint32_t size);

/* --- Transfer mode ------------------------------------------------------- */

void dma_enable_circular_mode(uint32_t dma, uint8_t channel);
void dma_enable_mem2mem_mode(uint32_t dma, uint8_t channel);
void dma_disable_mem2mem_mode(uint32_t dma, uint8_t channel);

/* --- Priority ------------------------------------------------------------ */

void dma_set_priority(uint32_t dma, uint8_t channel, uint32_t priority);

/* --- Interrupt enables --------------------------------------------------- */

void dma_enable_transfer_complete_interrupt(uint32_t dma, uint8_t channel);
void dma_disable_transfer_complete_interrupt(uint32_t dma, uint8_t channel);
void dma_enable_half_transfer_interrupt(uint32_t dma, uint8_t channel);
void dma_disable_half_transfer_interrupt(uint32_t dma, uint8_t channel);
void dma_enable_transfer_error_interrupt(uint32_t dma, uint8_t channel);
void dma_disable_transfer_error_interrupt(uint32_t dma, uint8_t channel);

/* --- Channel enable ------------------------------------------------------ */

void dma_channel_enable(uint32_t dma, uint8_t channel);
void dma_channel_disable(uint32_t dma, uint8_t channel);

/* --- Flags and interrupt status ------------------------------------------ */

uint32_t dma_get_flag(uint32_t dma, uint8_t channel, uint32_t flag);
void dma_clear_flag(uint32_t dma, uint8_t channel, uint32_t flag);
uint32_t dma_get_interrupt_status(uint32_t dma, uint8_t channel);
void dma_clear_interrupt_pending_bit(uint32_t dma, uint8_t channel);

END_DECLS

#endif
/** @cond */
#else
#warning "dma_common_v1.h should not be included explicitly, only via dma.h"
#endif
/** @endcond */
/**@}*/
