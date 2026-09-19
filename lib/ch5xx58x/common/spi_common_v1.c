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
 * @ingroup CH5XX58X
 *
 * @brief <b>Serial Peripheral Interface for the CH58x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * Two instances, SPI0 and SPI1, sharing one register layout.  There is no
 * enable bit: a master is enabled by turning on its SCK/MOSI output drivers
 * and a slave by turning on its MISO driver, which is what spi_enable() and
 * spi_disable() do.
 *
 * The block gives the firmware three ways to move bytes -- the single-byte
 * BUFFER with RB_SPI_FREE handshaking, the eight-byte FIFO, and a 12-bit
 * TOTAL_CNT engine -- and the helpers here use the first for single bytes and
 * the FIFO plus TOTAL_CNT for block transfers.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch5xx58x/spi.h>
#include <libopenwch/qingke/assert.h>

/** Reject anything that is not one of the two SPI blocks. */
static void spi_assert_valid(uint32_t spi) {
	openwch_assert(spi == SPI0_BASE || spi == SPI1_BASE);
}

/*
 * The two blocks are the same silicon: SPI1's SDO_OE/SDI_OE are the same bits
 * as SPI0's MOSI_OE/MISO_OE, so the stored mode bit is the only thing that
 * tells the two roles apart.
 */
static bool spi_is_slave(uint32_t spi) {
	return (SPI_CTRL_MOD(spi) & RB_SPI_MODE_SLAVE) != 0;
}

/* --- Configuration ------------------------------------------------------- */

void spi_set_clock_divider(uint32_t spi, uint8_t div) {
	spi_assert_valid(spi);
	openwch_assert(div != 0);

	/*
	 * A divisor of two is special: SCK is fast enough that the master has
	 * to sample MISO one cycle late, which WCH's own driver expresses with
	 * RB_SPI_MST_DLY_EN.  Every other divisor leaves the delay off.
	 */
	if (div == 2) {
		SPI_CTRL_CFG(spi) |= RB_SPI_MST_DLY_EN;
	} else {
		SPI_CTRL_CFG(spi) &= (uint8_t)~RB_SPI_MST_DLY_EN;
	}

	SPI_CLOCK_DIV(spi) = div;
}

/** Apply the mode-0/mode-3 and bit-order part of the data mode. */
static void spi_set_mode(uint32_t spi, spi_mode_t mode) {
	openwch_assert(mode >= SPI_MODE0_LSB && mode <= SPI_MODE3_MSB);

	if (mode == SPI_MODE3_LSB || mode == SPI_MODE3_MSB) {
		SPI_CTRL_MOD(spi) |= RB_SPI_MST_SCK_MOD;
	} else {
		SPI_CTRL_MOD(spi) &= (uint8_t)~RB_SPI_MST_SCK_MOD;
	}

	if (mode == SPI_MODE0_LSB || mode == SPI_MODE3_LSB) {
		SPI_CTRL_CFG(spi) |= RB_SPI_BIT_ORDER;
	} else {
		SPI_CTRL_CFG(spi) &= (uint8_t)~RB_SPI_BIT_ORDER;
	}
}

void spi_init_master(uint32_t spi, uint8_t clock_div, spi_mode_t mode) {
	spi_assert_valid(spi);
	openwch_assert(clock_div != 0);

	/* ALL_CLEAR forces the FIFO and both counters back to empty before any
	 * other bit in CTRL_MOD is set. */
	SPI_CTRL_MOD(spi) = RB_SPI_ALL_CLEAR;
	SPI_CTRL_MOD(spi) = SPI_CTRL_MOD_MASTER_OE;

	SPI_CTRL_CFG(spi) |= RB_SPI_AUTO_IF;
	SPI_CTRL_CFG(spi) &= (uint8_t)~RB_SPI_DMA_ENABLE;

	spi_set_clock_divider(spi, clock_div);
	spi_set_mode(spi, mode);
}

void spi_init_slave(uint32_t spi, spi_mode_t mode) {
	spi_assert_valid(spi);

	SPI_CTRL_MOD(spi) = RB_SPI_ALL_CLEAR;
	SPI_CTRL_MOD(spi) = SPI_CTRL_MOD_SLAVE_OE | RB_SPI_MODE_SLAVE;

	SPI_CTRL_CFG(spi) |= RB_SPI_AUTO_IF;
	SPI_CTRL_CFG(spi) &= (uint8_t)~RB_SPI_DMA_ENABLE;

	/*
	 * A slave takes CPOL/CPHA from the bus master, so only the bit order is
	 * meaningful here.  RB_SPI_MST_SCK_MOD shares its bit with
	 * RB_SPI_SLV_CMD_MOD (slave command mode), so it must be left alone
	 * rather than driven from the mode.
	 */
	openwch_assert(mode >= SPI_MODE0_LSB && mode <= SPI_MODE3_MSB);

	if (mode == SPI_MODE0_LSB || mode == SPI_MODE3_LSB) {
		SPI_CTRL_CFG(spi) |= RB_SPI_BIT_ORDER;
	} else {
		SPI_CTRL_CFG(spi) &= (uint8_t)~RB_SPI_BIT_ORDER;
	}
}

/* --- Enable -------------------------------------------------------------- */

void spi_enable(uint32_t spi) {
	spi_assert_valid(spi);

	if (spi_is_slave(spi)) {
		SPI_CTRL_MOD(spi) |= SPI_CTRL_MOD_SLAVE_OE;
	} else {
		SPI_CTRL_MOD(spi) |= SPI_CTRL_MOD_MASTER_OE;
	}
}

void spi_disable(uint32_t spi) {
	spi_assert_valid(spi);

	SPI_CTRL_MOD(spi) &= (uint8_t)~SPI_CTRL_MOD_OE_MASK;
}

/* --- Single byte --------------------------------------------------------- */

void spi_send(uint32_t spi, uint8_t data) {
	spi_assert_valid(spi);

	SPI_CTRL_MOD(spi) &= (uint8_t)~RB_SPI_FIFO_DIR;

	if (spi_is_slave(spi)) {
		SPI_FIFO(spi) = data;
		while (SPI_FIFO_COUNT(spi) != 0) {
			;
		}
	} else {
		SPI_BUFFER(spi) = data;
		while (!(SPI_INT_FLAG(spi) & RB_SPI_FREE)) {
			;
		}
	}
}

uint8_t spi_recv(uint32_t spi) {
	uint8_t data;

	spi_assert_valid(spi);

	SPI_CTRL_MOD(spi) |= RB_SPI_FIFO_DIR;

	if (spi_is_slave(spi)) {
		while (SPI_FIFO_COUNT(spi) == 0) {
			;
		}
		data = SPI_FIFO(spi);
	} else {
		/* Writing BUFFER starts the receive clock; 0xff is the dummy
		 * the master shifts out while it clocks MISO in. */
		SPI_BUFFER(spi) = 0xff;
		while (!(SPI_INT_FLAG(spi) & RB_SPI_FREE)) {
			;
		}
		data = SPI_BUFFER(spi);
	}

	return data;
}

uint8_t spi_xfer(uint32_t spi, uint8_t data) {
	/*
	 * The CH58x FIFO is unidirectional for the duration of a transfer, so
	 * a byte cannot be shifted in and out of BUFFER at the same time.
	 * spi_xfer() therefore has the same meaning as everywhere else in
	 * libopencm3: transmit one byte, then receive one byte.
	 */
	spi_send(spi, data);
	return spi_recv(spi);
}

/* --- Block transfer ------------------------------------------------------ */

/** Push `len` bytes through the FIFO in the currently selected direction. */
static void spi_fifo_write(uint32_t spi, const uint8_t *buf, uint16_t len) {
	uint16_t sent = 0;

	while (sent < len) {
		if (SPI_FIFO_COUNT(spi) < SPI_FIFO_SIZE) {
			SPI_FIFO(spi) = buf[sent];
			sent++;
		}
	}

	/* Do not return until the shifter has drained the queue. */
	while (SPI_FIFO_COUNT(spi) != 0) {
		;
	}
}

/** Drain `len` bytes from the FIFO in the currently selected direction. */
static void spi_fifo_read(uint32_t spi, uint8_t *buf, uint16_t len) {
	uint16_t got = 0;

	while (got < len) {
		if (SPI_FIFO_COUNT(spi) != 0) {
			buf[got] = SPI_FIFO(spi);
			got++;
		}
	}
}

void spi_master_write(uint32_t spi, const uint8_t *buf, uint16_t len) {
	spi_assert_valid(spi);
	openwch_assert(buf != NULL || len == 0);
	openwch_assert(len <= SPI_TOTAL_CNT_MAX);

	SPI_CTRL_MOD(spi) &= (uint8_t)~RB_SPI_FIFO_DIR;
	SPI_TOTAL_CNT(spi) = len;
	SPI_INT_FLAG(spi) = RB_SPI_IF_CNT_END;

	spi_fifo_write(spi, buf, len);
}

void spi_master_read(uint32_t spi, uint8_t *buf, uint16_t len) {
	spi_assert_valid(spi);
	openwch_assert(buf != NULL || len == 0);
	openwch_assert(len <= SPI_TOTAL_CNT_MAX);

	SPI_CTRL_MOD(spi) |= RB_SPI_FIFO_DIR;
	/* A non-zero length in the input direction starts the receive clock. */
	SPI_TOTAL_CNT(spi) = len;
	SPI_INT_FLAG(spi) = RB_SPI_IF_CNT_END;

	spi_fifo_read(spi, buf, len);
}

void spi_write(uint32_t spi, const uint8_t *buf, uint16_t len) {
	spi_assert_valid(spi);
	openwch_assert(buf != NULL || len == 0);
	openwch_assert(len <= SPI_TOTAL_CNT_MAX);

	if (!spi_is_slave(spi)) {
		spi_master_write(spi, buf, len);
		return;
	}

	/* A slave has no total-count engine to start: it just fills the FIFO
	 * in step with the master's clock. */
	SPI_CTRL_MOD(spi) &= (uint8_t)~RB_SPI_FIFO_DIR;
	SPI_INT_FLAG(spi) = RB_SPI_IF_CNT_END;

	spi_fifo_write(spi, buf, len);
}

void spi_read(uint32_t spi, uint8_t *buf, uint16_t len) {
	spi_assert_valid(spi);
	openwch_assert(buf != NULL || len == 0);
	openwch_assert(len <= SPI_TOTAL_CNT_MAX);

	if (!spi_is_slave(spi)) {
		spi_master_read(spi, buf, len);
		return;
	}

	SPI_CTRL_MOD(spi) |= RB_SPI_FIFO_DIR;
	SPI_INT_FLAG(spi) = RB_SPI_IF_CNT_END;

	spi_fifo_read(spi, buf, len);
}

/* --- DMA ----------------------------------------------------------------- */

void spi_enable_rx_dma(uint32_t spi) {
	spi_assert_valid(spi);

	/* There is one DMA engine and one enable bit; FIFO_DIR says which way
	 * the bytes travel. */
	SPI_CTRL_MOD(spi) |= RB_SPI_FIFO_DIR;
	SPI_CTRL_CFG(spi) |= RB_SPI_DMA_ENABLE;
}

void spi_enable_tx_dma(uint32_t spi) {
	spi_assert_valid(spi);

	SPI_CTRL_MOD(spi) &= (uint8_t)~RB_SPI_FIFO_DIR;
	SPI_CTRL_CFG(spi) |= RB_SPI_DMA_ENABLE;
}

/* --- Interrupts and flags ------------------------------------------------ */

void spi_enable_irq(uint32_t spi, uint32_t irq) {
	spi_assert_valid(spi);
	openwch_assert(irq != 0);
	openwch_assert((irq & ~(uint32_t)SPI_IRQ_MASK) == 0);

	SPI_INTER_EN(spi) |= (uint8_t)(irq & SPI_IRQ_MASK);
}

void spi_disable_irq(uint32_t spi, uint32_t irq) {
	spi_assert_valid(spi);
	openwch_assert((irq & ~(uint32_t)SPI_IRQ_MASK) == 0);

	SPI_INTER_EN(spi) &= (uint8_t)~(irq & SPI_IRQ_MASK);
}

uint8_t spi_get_flag(uint32_t spi, uint32_t flag) {
	spi_assert_valid(spi);
	openwch_assert((flag & ~(uint32_t)SPI_FLAG_MASK) == 0);

	return (uint8_t)(SPI_INT_FLAG(spi) & (flag & SPI_FLAG_MASK));
}

void spi_clear_flag(uint32_t spi, uint32_t flag) {
	spi_assert_valid(spi);
	openwch_assert((flag & ~(uint32_t)SPI_FLAG_MASK) == 0);

	/* INT_FLAG is RW1: writing a one clears that bit and leaves the rest. */
	SPI_INT_FLAG(spi) = (uint8_t)(flag & SPI_FLAG_CLEAR_MASK);
}
/**@}*/
