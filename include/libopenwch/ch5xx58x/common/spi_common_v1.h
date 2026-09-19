/** @addtogroup spi_defines SPI Defines

@brief <b>Defined Constants and Types for the CH58x SPI</b>

@ingroup CH5XX58X_defines

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA SPI.H */

/** @cond */
#if defined(LIBOPENWCH_SPI_H) || defined(LIBOPENWCH_SPI_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_SPI_COMMON_V1_H
#define LIBOPENWCH_SPI_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * The CH58x has two identical SPI blocks, SPI0 and SPI1, at 0x400-aligned
 * addresses.  Unlike the CH32V00x block there is no enable bit and no NSS
 * management: a master is "enabled" by turning on its SCK and MOSI output
 * drivers, a slave by turning on its MISO driver, and the chip select is an
 * ordinary GPIO the firmware drives itself.
 *
 * The data path has three faces, all of them plain read/write registers:
 *
 *	BUFFER		the single-byte holding register, used for one byte at a
 *			time.  Writing it starts the exchange; RB_SPI_FREE in
 *			INT_FLAG tells the firmware when the byte is done.
 *	FIFO		an eight-byte queue at 0x10, used by the block transfer
 *			helpers.  RB_SPI_FIFO_DIR (in CTRL_MOD) selects whether
 *			the FIFO is filled (output) or drained (input).
 *	TOTAL_CNT	a 12-bit transfer length.  When the input direction is
 *			selected, writing a non-zero length starts the receive
 *			clock, which is why the read helpers program it.
 *
 * None of these registers is RWA: no safe-access window is required, so the
 * RWA_* macros are deliberately not used here.
 *
 * SPI1 shares SPI0's register layout.  The only named difference in the
 * reference header is that SPI1 calls the MOSI/MISO output enables SDO_OE and
 * SDI_OE, but they are the same bits (0x40 and 0x80), so a single set of
 * accessors serves both instances.
 */

/* --- Register accessors -------------------------------------------------- */

#define SPI_CTRL_MOD(spi) MMIO8((spi) + 0x00)	  /**< mode control */
#define SPI_CTRL_CFG(spi) MMIO8((spi) + 0x01)	  /**< configuration control */
#define SPI_INTER_EN(spi) MMIO8((spi) + 0x02)	  /**< interrupt enable */
#define SPI_CLOCK_DIV(spi) MMIO8((spi) + 0x03)	  /**< master clock divisor */
#define SPI_SLAVE_PRESET(spi) MMIO8((spi) + 0x03) /**< slave preset byte */
#define SPI_BUFFER(spi) MMIO8((spi) + 0x04)	  /**< single-byte buffer */
#define SPI_RUN_FLAG(spi) MMIO8((spi) + 0x05)	  /**< work flag */
#define SPI_INT_FLAG(spi) MMIO8((spi) + 0x06)	  /**< interrupt flags, RW1 */
#define SPI_FIFO_COUNT(spi) MMIO8((spi) + 0x07)	  /**< FIFO occupancy */
#define SPI_TOTAL_CNT(spi) MMIO16((spi) + 0x0c)	  /**< total byte count */
#define SPI_FIFO(spi) MMIO8((spi) + 0x10)	  /**< FIFO data port */
#define SPI_DMA_NOW(spi) MMIO32((spi) + 0x14)	  /**< DMA current address */
#define SPI_DMA_BEG(spi) MMIO32((spi) + 0x18)	  /**< DMA begin address */
#define SPI_DMA_END(spi) MMIO32((spi) + 0x1c)	  /**< DMA end address */

/* --- CTRL_MOD bits ------------------------------------------------------- */

#define RB_SPI_MODE_SLAVE 0x01 /**< 0 = master, 1 = slave */
#define RB_SPI_ALL_CLEAR 0x02  /**< force-clear FIFO and counters */
#define RB_SPI_2WIRE_MOD 0x04  /**< slave 2-wire (SCK + SDX) mode */
#define RB_SPI_MST_SCK_MOD                                                     \
	0x08 /**< master clock mode: 0 = mode 0, 1 = mode 3 */
#define RB_SPI_SLV_CMD_MOD 0x08 /**< slave command mode (same bit as above) */
#define RB_SPI_FIFO_DIR 0x10	/**< FIFO direction: 0 = out, 1 = in */
#define RB_SPI_SCK_OE 0x20	/**< SCK output enable */
#define RB_SPI_MOSI_OE 0x40	/**< MOSI output enable (SPI1: SDO_OE) */
#define RB_SPI_MISO_OE 0x80	/**< MISO output enable (SPI1: SDI_OE) */

/** Output drivers a master must switch on. */
#define SPI_CTRL_MOD_MASTER_OE (RB_SPI_SCK_OE | RB_SPI_MOSI_OE)
/** Output driver a slave must switch on. */
#define SPI_CTRL_MOD_SLAVE_OE (RB_SPI_MISO_OE)
/** Every output driver, for spi_disable(). */
#define SPI_CTRL_MOD_OE_MASK (RB_SPI_SCK_OE | RB_SPI_MOSI_OE | RB_SPI_MISO_OE)

/* --- CTRL_CFG bits ------------------------------------------------------- */

#define RB_SPI_DMA_ENABLE 0x01 /**< DMA mode enable */
#define RB_SPI_DMA_LOOP 0x04   /**< DMA address loop enable */
#define RB_SPI_AUTO_IF 0x10    /**< buffer/FIFO access clears BYTE_END */
#define RB_SPI_BIT_ORDER 0x20  /**< 0 = MSB first, 1 = LSB first */
#define RB_SPI_MST_DLY_EN 0x40 /**< master input delay enable */

/* --- INTER_EN bits ------------------------------------------------------- */

#define RB_SPI_IE_CNT_END 0x01	/**< total byte count reached */
#define RB_SPI_IE_BYTE_END 0x02 /**< one byte exchanged */
#define RB_SPI_IE_FIFO_HF 0x04	/**< FIFO half full / half empty */
#define RB_SPI_IE_DMA_END 0x08	/**< DMA complete */
#define RB_SPI_IE_FIFO_OV 0x10	/**< FIFO overrun */
#define RB_SPI_IE_FST_BYTE 0x80 /**< slave first byte / command received */

/* --- RUN_FLAG bits ------------------------------------------------------- */

#define RB_SPI_SLV_CMD_ACT 0x10 /**< slave first byte / command flag */
#define RB_SPI_FIFO_READY 0x20	/**< FIFO ready status */
#define RB_SPI_SLV_CS_LOAD 0x40 /**< slave chip-select loading status */
#define RB_SPI_SLV_SELECT 0x80	/**< slave selection status */

/* --- INT_FLAG bits ------------------------------------------------------- */

#define RB_SPI_IF_CNT_END 0x01	/**< RW1: total byte count reached */
#define RB_SPI_IF_BYTE_END 0x02 /**< RW1: one byte exchanged */
#define RB_SPI_IF_FIFO_HF 0x04	/**< RW1: FIFO half full / half empty */
#define RB_SPI_IF_DMA_END 0x08	/**< RW1: DMA complete */
#define RB_SPI_IF_FIFO_OV 0x10	/**< RW1: FIFO overrun */
#define RB_SPI_FREE 0x40	/**< RO: interface idle, byte finished */
#define RB_SPI_IF_FST_BYTE 0x80 /**< RW1: slave first byte received */

/** FIFO depth, in bytes. */
#define SPI_FIFO_SIZE 8
/** Largest transfer a single TOTAL_CNT write can describe (12 bits). */
#define SPI_TOTAL_CNT_MAX 0x0fffu

/* --- Data mode ----------------------------------------------------------- */

/** @defgroup spi_mode SPI Data Modes
@ingroup spi_defines

`mode` selects the clock polarity/phase pair and the bit order in one value,
mirroring WCH's ModeBitOrderTypeDef.  The enumerator values are kept identical
to WCH's so that a ported call site needs no translation:

	SPI_MODE0_LSB = 0  <- Mode0_LowBitINFront
	SPI_MODE0_MSB = 1  <- Mode0_HighBitINFront
	SPI_MODE3_LSB = 2  <- Mode3_LowBitINFront
	SPI_MODE3_MSB = 3  <- Mode3_HighBitINFront

Mode 0 idles SCK low and samples on the first edge; mode 3 idles SCK high and
samples on the second edge.
@{*/
typedef enum {
	SPI_MODE0_MSB = 1, /**< mode 0, MSB first */
	SPI_MODE0_LSB = 0, /**< mode 0, LSB first */
	SPI_MODE3_MSB = 3, /**< mode 3, MSB first */
	SPI_MODE3_LSB = 2, /**< mode 3, LSB first */
} spi_mode_t;
/**@}*/

/* --- Interrupt sources --------------------------------------------------- */

/** @defgroup spi_irq SPI Interrupt Sources
@ingroup spi_defines

Masks for spi_enable_irq() / spi_disable_irq().
@{*/
#define SPI_IRQ_CNT_END RB_SPI_IE_CNT_END
#define SPI_IRQ_BYTE_END RB_SPI_IE_BYTE_END
#define SPI_IRQ_FIFO_HALF RB_SPI_IE_FIFO_HF
#define SPI_IRQ_DMA_END RB_SPI_IE_DMA_END
#define SPI_IRQ_FIFO_OVERRUN RB_SPI_IE_FIFO_OV
#define SPI_IRQ_FIRST_BYTE RB_SPI_IE_FST_BYTE
/** Every interrupt bit INTER_EN defines. */
#define SPI_IRQ_MASK                                                           \
	(RB_SPI_IE_CNT_END | RB_SPI_IE_BYTE_END | RB_SPI_IE_FIFO_HF |          \
	 RB_SPI_IE_DMA_END | RB_SPI_IE_FIFO_OV | RB_SPI_IE_FST_BYTE)
/**@}*/

/* --- Flags --------------------------------------------------------------- */

/** @defgroup spi_flags SPI Status Flags
@ingroup spi_defines

Masks for spi_get_flag() / spi_clear_flag(), taken from INT_FLAG.  Only the
RW1 bits can be cleared by spi_clear_flag(); RB_SPI_FREE is read-only and is
therefore excluded from SPI_FLAG_CLEAR_MASK.
@{*/
#define SPI_FLAG_CNT_END RB_SPI_IF_CNT_END
#define SPI_FLAG_BYTE_END RB_SPI_IF_BYTE_END
#define SPI_FLAG_FIFO_HALF RB_SPI_IF_FIFO_HF
#define SPI_FLAG_DMA_END RB_SPI_IF_DMA_END
#define SPI_FLAG_FIFO_OVERRUN RB_SPI_IF_FIFO_OV
#define SPI_FLAG_FREE RB_SPI_FREE
#define SPI_FLAG_FIRST_BYTE RB_SPI_IF_FST_BYTE
/** Every readable INT_FLAG bit. */
#define SPI_FLAG_MASK                                                          \
	(RB_SPI_IF_CNT_END | RB_SPI_IF_BYTE_END | RB_SPI_IF_FIFO_HF |          \
	 RB_SPI_IF_DMA_END | RB_SPI_IF_FIFO_OV | RB_SPI_FREE |                 \
	 RB_SPI_IF_FST_BYTE)
/** The write-one-to-clear subset of SPI_FLAG_MASK. */
#define SPI_FLAG_CLEAR_MASK                                                    \
	(RB_SPI_IF_CNT_END | RB_SPI_IF_BYTE_END | RB_SPI_IF_FIFO_HF |          \
	 RB_SPI_IF_DMA_END | RB_SPI_IF_FIFO_OV | RB_SPI_IF_FST_BYTE)
/**@}*/

BEGIN_DECLS

/* --- Configuration ------------------------------------------------------- */

void spi_init_master(uint32_t spi, uint8_t clock_div, spi_mode_t mode);
void spi_init_slave(uint32_t spi, spi_mode_t mode);
void spi_set_clock_divider(uint32_t spi, uint8_t div);

/* --- Enable -------------------------------------------------------------- */

void spi_enable(uint32_t spi);
void spi_disable(uint32_t spi);

/* --- Single byte --------------------------------------------------------- */

void spi_send(uint32_t spi, uint8_t data);
uint8_t spi_recv(uint32_t spi);
uint8_t spi_xfer(uint32_t spi, uint8_t data);

/* --- Block transfer ------------------------------------------------------ */

void spi_write(uint32_t spi, const uint8_t *buf, uint16_t len);
void spi_read(uint32_t spi, uint8_t *buf, uint16_t len);
void spi_master_write(uint32_t spi, const uint8_t *buf, uint16_t len);
void spi_master_read(uint32_t spi, uint8_t *buf, uint16_t len);

/* --- DMA ----------------------------------------------------------------- */

void spi_enable_rx_dma(uint32_t spi);
void spi_enable_tx_dma(uint32_t spi);

/* --- Interrupts and flags ------------------------------------------------ */

void spi_enable_irq(uint32_t spi, uint32_t irq);
void spi_disable_irq(uint32_t spi, uint32_t irq);
uint8_t spi_get_flag(uint32_t spi, uint32_t flag);
void spi_clear_flag(uint32_t spi, uint32_t flag);

END_DECLS

#endif
/** @cond */
#else
#warning "spi_common_v1.h should not be included explicitly, only via spi.h"
#endif
/** @endcond */
/**@}*/
