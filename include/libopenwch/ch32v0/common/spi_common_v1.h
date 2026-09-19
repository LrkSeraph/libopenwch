/** @addtogroup spi_defines SPI Defines

@brief <b>Defined Constants and Types for the CH32V00x SPI</b>

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA SPI.H
The order of header inclusion is important: spi.h includes the device
specific memorymap.h header before including this header file. */

/** @cond */
#if defined(LIBOPENWCH_SPI_H) || defined(LIBOPENWCH_SPI_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_SPI_COMMON_V1_H
#define LIBOPENWCH_SPI_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * The CH32V00x SPI1 is the classic STM32-style block, but the CH32 version
 * spaces the 16-bit registers out to 32-bit boundaries.  Every register is
 * therefore accessed as MMIO16 at a stride of 4.
 *
 * The only instance is SPI1, on APB2, with its default pins PA5 (SCK),
 * PA6 (MISO), PA7 (MOSI) and PA4 (NSS); the remap bit AFIO_PCFR1_SPI1_REMAP
 * moves it to PC5..PC7/PC4 style positions (see the vendor header).  The pins
 * are configured with gpio_set_mode(..., GPIO_MODE_AF_PP) and the clock is
 * enabled through the RCC driver.
 */

/* --- Register accessors -------------------------------------------------- */

#define SPI_CTLR1(spi)			MMIO16((spi) + 0x00)	/**< control register 1 */
#define SPI_CTLR2(spi)			MMIO16((spi) + 0x04)	/**< control register 2 */
#define SPI_STATR(spi)			MMIO16((spi) + 0x08)	/**< status register */
#define SPI_DATAR(spi)			MMIO16((spi) + 0x0c)	/**< data register */
#define SPI_CRCR(spi)			MMIO16((spi) + 0x10)	/**< CRC polynomial register */
#define SPI_RCRCR(spi)			MMIO16((spi) + 0x14)	/**< RX CRC register */
#define SPI_TCRCR(spi)			MMIO16((spi) + 0x18)	/**< TX CRC register */
#define SPI_HSCR(spi)			MMIO16((spi) + 0x24)	/**< high-speed read control register */

/* --- CTLR1 bits ---------------------------------------------------------- */

#define SPI_CTLR1_CPHA			(1 << 0)	/**< clock phase */
#define SPI_CTLR1_CPOL			(1 << 1)	/**< clock polarity */
#define SPI_CTLR1_MSTR			(1 << 2)	/**< master selection */
#define SPI_CTLR1_BR_SHIFT		3
#define SPI_CTLR1_BR_MASK		(0x7u << SPI_CTLR1_BR_SHIFT)
#define SPI_CTLR1_SPE			(1 << 6)	/**< SPI enable */
#define SPI_CTLR1_LSBFIRST		(1 << 7)	/**< frame format: 1 = LSB first */
#define SPI_CTLR1_SSI			(1 << 8)	/**< internal slave select */
#define SPI_CTLR1_SSM			(1 << 9)	/**< software slave management */
#define SPI_CTLR1_RXONLY		(1 << 10)	/**< receive only */
#define SPI_CTLR1_DFF			(1 << 11)	/**< data frame format: 1 = 16 bit */
#define SPI_CTLR1_CRCNEXT		(1 << 12)	/**< transmit CRC next */
#define SPI_CTLR1_CRCEN			(1 << 13)	/**< hardware CRC enable */
#define SPI_CTLR1_BIDIOE		(1 << 14)	/**< output enable in bidirectional mode */
#define SPI_CTLR1_BIDIMODE		(1 << 15)	/**< bidirectional data mode enable */

/* --- CTLR2 bits ---------------------------------------------------------- */

#define SPI_CTLR2_RXDMAEN		(1 << 0)	/**< RX buffer DMA enable */
#define SPI_CTLR2_TXDMAEN		(1 << 1)	/**< TX buffer DMA enable */
#define SPI_CTLR2_SSOE			(1 << 2)	/**< SS output enable */
#define SPI_CTLR2_ERRIE			(1 << 5)	/**< error interrupt enable */
#define SPI_CTLR2_RXNEIE		(1 << 6)	/**< RX buffer not empty interrupt enable */
#define SPI_CTLR2_TXEIE			(1 << 7)	/**< TX buffer empty interrupt enable */

/* --- STATR bits ---------------------------------------------------------- */

#define SPI_STATR_RXNE			(1 << 0)	/**< receive buffer not empty */
#define SPI_STATR_TXE			(1 << 1)	/**< transmit buffer empty */
#define SPI_STATR_CHSIDE		(1 << 2)	/**< channel side */
#define SPI_STATR_UDR			(1 << 3)	/**< underrun flag */
#define SPI_STATR_CRCERR		(1 << 4)	/**< CRC error flag */
#define SPI_STATR_MODF			(1 << 5)	/**< mode fault */
#define SPI_STATR_OVR			(1 << 6)	/**< overrun flag */
#define SPI_STATR_BSY			(1 << 7)	/**< busy flag */

/* --- DATAR --------------------------------------------------------------- */

#define SPI_DATAR_DR_MASK		0xffffu

/* --- HSCR ---------------------------------------------------------------- */

#define SPI_HSCR_HSRXEN			(1 << 0)	/**< high-speed read enable */

/* --- Configuration enumerations ------------------------------------------ */

/** @defgroup spi_cpol SPI Clock Polarity
@ingroup spi_defines
@{*/
#define SPI_CPOL_LOW			0x0		/**< clock idle low */
#define SPI_CPOL_HIGH			SPI_CTLR1_CPOL	/**< clock idle high */
/**@}*/

/** @defgroup spi_cpha SPI Clock Phase
@ingroup spi_defines
@{*/
#define SPI_CPHA_FIRST			0x0		/**< capture on first clock edge */
#define SPI_CPHA_SECOND			SPI_CTLR1_CPHA	/**< capture on second clock edge */
/**@}*/

/** @defgroup spi_dff SPI Data Frame Format
@ingroup spi_defines
@{*/
#define SPI_DFF_8BIT			0x0		/**< 8-bit data frame */
#define SPI_DFF_16BIT			SPI_CTLR1_DFF	/**< 16-bit data frame */
/**@}*/

/** @defgroup spi_bit_order SPI Bit Order
@ingroup spi_defines
@{*/
#define SPI_BIT_ORDER_MSB_FIRST		0x0		/**< MSB transmitted first */
#define SPI_BIT_ORDER_LSB_FIRST		SPI_CTLR1_LSBFIRST /**< LSB transmitted first */
/**@}*/

/** @defgroup spi_baudrate SPI Baud Rate Prescaler
@ingroup spi_defines

The three-bit field value (not yet shifted) for spi_set_baudrate_prescaler()
and spi_init_master().  The clock is the APB2 bus clock divided by the
prescaler.
@{*/
#define SPI_BAUDRATE_PRESCALER_2	0x0
#define SPI_BAUDRATE_PRESCALER_4	0x1
#define SPI_BAUDRATE_PRESCALER_8	0x2
#define SPI_BAUDRATE_PRESCALER_16	0x3
#define SPI_BAUDRATE_PRESCALER_32	0x4
#define SPI_BAUDRATE_PRESCALER_64	0x5
#define SPI_BAUDRATE_PRESCALER_128	0x6
#define SPI_BAUDRATE_PRESCALER_256	0x7
/**@}*/

/** @defgroup spi_crc_length SPI CRC Length
@ingroup spi_defines

On this SPI the CRC length follows the data frame format: there is no separate
CRC-length bit, so these aliases share the CTLR1 DFF encoding.
@{*/
#define SPI_CRC_LENGTH_8BIT		0x0
#define SPI_CRC_LENGTH_16BIT		SPI_CTLR1_DFF
/**@}*/

/** @defgroup spi_irq SPI Interrupt Sources
@ingroup spi_defines
@{*/
#define SPI_IRQ_TXE			SPI_CTLR2_TXEIE
#define SPI_IRQ_RXNE			SPI_CTLR2_RXNEIE
#define SPI_IRQ_ERR			SPI_CTLR2_ERRIE
#define SPI_IRQ_MASK			(SPI_IRQ_TXE | SPI_IRQ_RXNE | SPI_IRQ_ERR)
/**@}*/

BEGIN_DECLS

/* --- Initialisation ------------------------------------------------------ */

void spi_init_master(uint32_t spi, uint32_t br, uint32_t cpol, uint32_t cpha,
		uint32_t dff, uint32_t lsbfirst);
void spi_init_slave(uint32_t spi, uint32_t cpol, uint32_t cpha, uint32_t dff,
		uint32_t lsbfirst);

/* --- Enable -------------------------------------------------------------- */

void spi_enable(uint32_t spi);
void spi_disable(uint32_t spi);

/* --- Data ---------------------------------------------------------------- */

/** Blocking transmit of one frame, waiting for TXE. */
void spi_send(uint32_t spi, uint16_t data);
/** Blocking receive of one frame, waiting for RXNE. */
uint16_t spi_recv(uint32_t spi);
/** Blocking full-duplex exchange of one frame. */
uint16_t spi_xfer(uint32_t spi, uint16_t data);

/* --- Configuration ------------------------------------------------------- */

void spi_set_baudrate_prescaler(uint32_t spi, uint32_t br);
void spi_set_clock_polarity(uint32_t spi, uint32_t cpol);
void spi_set_clock_phase(uint32_t spi, uint32_t cpha);
void spi_set_dff(uint32_t spi, uint32_t dff);
void spi_set_bit_order(uint32_t spi, uint32_t order);

/* --- Slave select -------------------------------------------------------- */

void spi_enable_software_slave_management(uint32_t spi);
void spi_set_nss_high(uint32_t spi);
void spi_set_nss_low(uint32_t spi);
void spi_enable_ss_output(uint32_t spi);

/* --- Bidirectional (single-wire simplex) mode ---------------------------- */

void spi_set_bidirectional_mode(uint32_t spi);
void spi_set_bidirectional_transmit_only(uint32_t spi);

/* --- CRC ----------------------------------------------------------------- */

void spi_set_crc_length(uint32_t spi, uint32_t len);
void spi_enable_crc(uint32_t spi);
void spi_disable_crc(uint32_t spi);
void spi_set_crc_polynomial(uint32_t spi, uint16_t polynomial);
uint16_t spi_get_tx_crc(uint32_t spi);
uint16_t spi_get_rx_crc(uint32_t spi);

/* --- DMA ----------------------------------------------------------------- */

void spi_enable_rx_dma(uint32_t spi);
void spi_enable_tx_dma(uint32_t spi);

/* --- Interrupts and flags ------------------------------------------------ */

void spi_enable_irq(uint32_t spi, uint32_t irq);
void spi_disable_irq(uint32_t spi, uint32_t irq);
uint16_t spi_get_flag(uint32_t spi, uint16_t flag);
void spi_clear_flag(uint32_t spi, uint16_t flag);

END_DECLS

#endif
/** @cond */
#else
#warning "spi_common_v1.h should not be included explicitly, only via spi.h"
#endif
/** @endcond */
/**@}*/
