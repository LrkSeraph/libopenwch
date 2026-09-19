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

/** @addtogroup spi_file SPI
 *
 * @ingroup CH32V0
 *
 * @brief <b>Serial Peripheral Interface for the CH32V00x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * SPI1 is the classic STM32-style block: CTLR1 holds the mode, frame format,
 * clock phase/polarity, baud rate prescaler and CRC control; CTLR2 the DMA,
 * slave-select and interrupt enables; STATR the flags and DATAR the 8/16-bit
 * shift register.  The CH32 version spaces the 16-bit registers out to 32-bit
 * boundaries, so every accessor is MMIO16 at a stride of 4.
 *
 * The driver deliberately does not touch the RCC, AFIO or GPIO blocks: the
 * caller enables the APB2 clock, routes the pins with gpio_set_mode() and, if
 * needed, selects the remap with gpio_primary_remap().
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch32v0/spi.h>
#include <libopenwch/qingke/assert.h>

/* --- Initialisation ------------------------------------------------------ */

void spi_init_master(
	uint32_t spi,
	uint32_t br,
	uint32_t cpol,
	uint32_t cpha,
	uint32_t dff,
	uint32_t lsbfirst
) {
	uint16_t reg = SPI_CTLR1(spi);

	openwch_assert(br <= (SPI_CTLR1_BR_MASK >> SPI_CTLR1_BR_SHIFT));
	openwch_assert((cpol & (uint32_t)~SPI_CTLR1_CPOL) == 0);
	openwch_assert((cpha & (uint32_t)~SPI_CTLR1_CPHA) == 0);
	openwch_assert((dff & (uint32_t)~SPI_CTLR1_DFF) == 0);
	openwch_assert((lsbfirst & (uint32_t)~SPI_CTLR1_LSBFIRST) == 0);

	reg = (uint16_t)((reg & ~(SPI_CTLR1_BR_MASK | SPI_CTLR1_CPOL
					| SPI_CTLR1_CPHA | SPI_CTLR1_DFF
					| SPI_CTLR1_LSBFIRST))
			| ((br << SPI_CTLR1_BR_SHIFT) & SPI_CTLR1_BR_MASK)
			| (cpol & SPI_CTLR1_CPOL)
			| (cpha & SPI_CTLR1_CPHA)
			| (dff & SPI_CTLR1_DFF)
			| (lsbfirst & SPI_CTLR1_LSBFIRST)
			| SPI_CTLR1_MSTR);
	SPI_CTLR1(spi) = reg;
}

void spi_init_slave(
	uint32_t spi,
	uint32_t cpol,
	uint32_t cpha,
	uint32_t dff,
	uint32_t lsbfirst
) {
	uint16_t reg = SPI_CTLR1(spi);

	openwch_assert((cpol & (uint32_t)~SPI_CTLR1_CPOL) == 0);
	openwch_assert((cpha & (uint32_t)~SPI_CTLR1_CPHA) == 0);
	openwch_assert((dff & (uint32_t)~SPI_CTLR1_DFF) == 0);
	openwch_assert((lsbfirst & (uint32_t)~SPI_CTLR1_LSBFIRST) == 0);

	/*
	 * The baud rate prescaler is meaningless in slave mode (the clock is
	 * driven by the master), so it is cleared along with MSTR to leave the
	 * peripheral in a well-defined state.
	 */
	reg = (uint16_t)((reg & ~(SPI_CTLR1_BR_MASK | SPI_CTLR1_CPOL
					| SPI_CTLR1_CPHA | SPI_CTLR1_DFF
					| SPI_CTLR1_LSBFIRST | SPI_CTLR1_MSTR))
			| (cpol & SPI_CTLR1_CPOL)
			| (cpha & SPI_CTLR1_CPHA)
			| (dff & SPI_CTLR1_DFF)
			| (lsbfirst & SPI_CTLR1_LSBFIRST));
	SPI_CTLR1(spi) = reg;
}

/* --- Enable -------------------------------------------------------------- */

void spi_enable(uint32_t spi) {
	SPI_CTLR1(spi) |= SPI_CTLR1_SPE;
}

void spi_disable(uint32_t spi) {
	SPI_CTLR1(spi) &= (uint16_t)~SPI_CTLR1_SPE;
}

/* --- Data ---------------------------------------------------------------- */

void spi_send(uint32_t spi, uint16_t data) {
	while (!(SPI_STATR(spi) & SPI_STATR_TXE)) {
		;
	}

	SPI_DATAR(spi) = (uint16_t)(data & SPI_DATAR_DR_MASK);
}

uint16_t spi_recv(uint32_t spi) {
	while (!(SPI_STATR(spi) & SPI_STATR_RXNE)) {
		;
	}

	return (uint16_t)(SPI_DATAR(spi) & SPI_DATAR_DR_MASK);
}

uint16_t spi_xfer(uint32_t spi, uint16_t data) {
	spi_send(spi, data);

	while (!(SPI_STATR(spi) & SPI_STATR_RXNE)) {
		;
	}

	return (uint16_t)(SPI_DATAR(spi) & SPI_DATAR_DR_MASK);
}

/* --- Configuration ------------------------------------------------------- */

void spi_set_baudrate_prescaler(uint32_t spi, uint32_t br) {
	uint16_t reg = SPI_CTLR1(spi);

	openwch_assert(br <= (SPI_CTLR1_BR_MASK >> SPI_CTLR1_BR_SHIFT));

	reg = (uint16_t)((reg & ~SPI_CTLR1_BR_MASK)
			| ((br << SPI_CTLR1_BR_SHIFT) & SPI_CTLR1_BR_MASK));
	SPI_CTLR1(spi) = reg;
}

void spi_set_clock_polarity(uint32_t spi, uint32_t cpol) {
	uint16_t reg = SPI_CTLR1(spi);

	openwch_assert((cpol & (uint32_t)~SPI_CTLR1_CPOL) == 0);

	reg = (uint16_t)((reg & ~SPI_CTLR1_CPOL) | (cpol & SPI_CTLR1_CPOL));
	SPI_CTLR1(spi) = reg;
}

void spi_set_clock_phase(uint32_t spi, uint32_t cpha) {
	uint16_t reg = SPI_CTLR1(spi);

	openwch_assert((cpha & (uint32_t)~SPI_CTLR1_CPHA) == 0);

	reg = (uint16_t)((reg & ~SPI_CTLR1_CPHA) | (cpha & SPI_CTLR1_CPHA));
	SPI_CTLR1(spi) = reg;
}

void spi_set_dff(uint32_t spi, uint32_t dff) {
	uint16_t reg = SPI_CTLR1(spi);

	openwch_assert((dff & (uint32_t)~SPI_CTLR1_DFF) == 0);

	reg = (uint16_t)((reg & ~SPI_CTLR1_DFF) | (dff & SPI_CTLR1_DFF));
	SPI_CTLR1(spi) = reg;
}

void spi_set_bit_order(uint32_t spi, uint32_t order) {
	uint16_t reg = SPI_CTLR1(spi);

	openwch_assert((order & (uint32_t)~SPI_CTLR1_LSBFIRST) == 0);

	reg = (uint16_t)((reg & ~SPI_CTLR1_LSBFIRST)
			| (order & SPI_CTLR1_LSBFIRST));
	SPI_CTLR1(spi) = reg;
}

/* --- Slave select -------------------------------------------------------- */

void spi_enable_software_slave_management(uint32_t spi) {
	SPI_CTLR1(spi) |= SPI_CTLR1_SSM;
}

void spi_set_nss_high(uint32_t spi) {
	SPI_CTLR1(spi) |= SPI_CTLR1_SSI;
}

void spi_set_nss_low(uint32_t spi) {
	SPI_CTLR1(spi) &= (uint16_t)~SPI_CTLR1_SSI;
}

void spi_enable_ss_output(uint32_t spi) {
	SPI_CTLR2(spi) |= SPI_CTLR2_SSOE;
}

/* --- Bidirectional (single-wire simplex) mode ---------------------------- */

void spi_set_bidirectional_mode(uint32_t spi) {
	SPI_CTLR1(spi) |= SPI_CTLR1_BIDIMODE;
}

void spi_set_bidirectional_transmit_only(uint32_t spi) {
	uint16_t reg = SPI_CTLR1(spi);

	/* Bidirectional mode with the data line driven (transmit only). */
	reg |= SPI_CTLR1_BIDIMODE | SPI_CTLR1_BIDIOE;
	SPI_CTLR1(spi) = reg;
}

/* --- CRC ----------------------------------------------------------------- */

void spi_set_crc_length(uint32_t spi, uint32_t len) {
	uint16_t reg = SPI_CTLR1(spi);

	/*
	 * There is no dedicated CRC-length bit on this SPI: the CRC is 8 bits
	 * when the data frame is 8 bits and 16 bits when it is 16 bits, so CRCL
	 * selects the same DFF encoding as spi_set_dff().
	 */
	openwch_assert((len & (uint32_t)~SPI_CTLR1_DFF) == 0);

	reg = (uint16_t)((reg & ~SPI_CTLR1_DFF) | (len & SPI_CTLR1_DFF));
	SPI_CTLR1(spi) = reg;
}

void spi_enable_crc(uint32_t spi) {
	SPI_CTLR1(spi) |= SPI_CTLR1_CRCEN;
}

void spi_disable_crc(uint32_t spi) {
	SPI_CTLR1(spi) &= (uint16_t)~SPI_CTLR1_CRCEN;
}

void spi_set_crc_polynomial(uint32_t spi, uint16_t polynomial) {
	SPI_CRCR(spi) = polynomial;
}

uint16_t spi_get_tx_crc(uint32_t spi) {
	return SPI_TCRCR(spi);
}

uint16_t spi_get_rx_crc(uint32_t spi) {
	return SPI_RCRCR(spi);
}

/* --- DMA ----------------------------------------------------------------- */

void spi_enable_rx_dma(uint32_t spi) {
	SPI_CTLR2(spi) |= SPI_CTLR2_RXDMAEN;
}

void spi_enable_tx_dma(uint32_t spi) {
	SPI_CTLR2(spi) |= SPI_CTLR2_TXDMAEN;
}

/* --- Interrupts and flags ------------------------------------------------ */

void spi_enable_irq(uint32_t spi, uint32_t irq) {
	openwch_assert((irq & (uint32_t)~SPI_IRQ_MASK) == 0);
	openwch_assert(irq != 0);

	SPI_CTLR2(spi) |= (uint16_t)(irq & SPI_IRQ_MASK);
}

void spi_disable_irq(uint32_t spi, uint32_t irq) {
	openwch_assert((irq & (uint32_t)~SPI_IRQ_MASK) == 0);

	SPI_CTLR2(spi) &= (uint16_t)~(irq & SPI_IRQ_MASK);
}

uint16_t spi_get_flag(uint32_t spi, uint16_t flag) {
	return (uint16_t)(SPI_STATR(spi) & flag);
}

void spi_clear_flag(uint32_t spi, uint16_t flag) {
	/*
	 * CRCERR is the only plain rc_w0 flag.  OVR is cleared by reading
	 * DATAR and then STATR, UDR by reading STATR and MODF by reading STATR
	 * and then writing CTLR1; those documented sequences are performed
	 * here so the caller does not have to know them.
	 */
	if (flag & (SPI_STATR_OVR | SPI_STATR_UDR | SPI_STATR_MODF)) {
		(void)SPI_DATAR(spi);
		(void)SPI_STATR(spi);
	}

	if (flag & SPI_STATR_MODF) {
		SPI_CTLR1(spi) = SPI_CTLR1(spi);
	}

	if (flag & SPI_STATR_CRCERR) {
		SPI_STATR(spi) = 0;
	}
}
/**@}*/
