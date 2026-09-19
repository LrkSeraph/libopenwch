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

/** @addtogroup uart_file UART
 *
 * @ingroup CH5XX58X
 *
 * @brief <b>UARTs for the CH58x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * UART0..UART3 share one register map, so the whole driver is parameterised by
 * the block base address and every entry point takes it first.
 *
 * The baud-rate generator is a plain 16-bit divisor, unlike the CH32V00x 12.4
 * fixed-point divider, and WCH expresses the divisor as
 *
 *	x = 10 * Fsys / 8 / baud	then	x = (x + 5) / 10
 *
 * which uart_set_baudrate() reproduces from the live system clock.
 *
 * All registers are plain read/write: none of them is RWA, so no safe-access
 * window is involved.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch5xx58x/uart.h>
#include <libopenwch/ch5xx58x/clk.h>
#include <libopenwch/qingke/assert.h>

/** True for the four UART block base addresses this family implements. */
static inline bool uart_is_valid(uint32_t uart)
{
	return (uart == UART0) || (uart == UART1) ||
	       (uart == UART2) || (uart == UART3);
}

/* --- Line format --------------------------------------------------------- */

void uart_set_baudrate(uint32_t uart, uint32_t baud)
{
	uint32_t fsys;
	uint32_t divisor;

	openwch_assert(uart_is_valid(uart));
	openwch_assert(baud != 0);

	fsys = clk_get_sys_clock();

	/*
	 * WCH's 10 * Fsys would already overflow a uint32_t if the clock were
	 * the full 480 MHz PLL, which clk_get_sys_clock() only reports for the
	 * illegal divide-by-zero setting.  Evaluating the quotient as
	 * 5 * Fsys / 4 gives exactly the same integer result and stays below
	 * 2^31 for every clock the part can produce.
	 */
	divisor = (5u * fsys) / 4u / baud;
	divisor = (divisor + 5u) / 10u;

	/*
	 * A divisor of zero is not meaningful, and one above 0xffff cannot be
	 * represented by the 16-bit latch; both mean the requested baud rate
	 * is out of range for the current system clock.
	 */
	openwch_assert(divisor != 0);
	openwch_assert(divisor <= 0xffffu);

	UART_DL(uart) = (uint16_t)divisor;
}

void uart_set_databits(uint32_t uart, uint8_t bits)
{
	uint8_t lcr;

	openwch_assert(uart_is_valid(uart));
	openwch_assert((bits >= UART_DATA_5BITS) && (bits <= UART_DATA_8BITS));

	/*
	 * RB_LCR_WORD_SZ is a plain binary count offset by five: 00 = 5 bits
	 * through 11 = 8 bits.
	 */
	lcr = UART_LCR(uart);
	lcr = (uint8_t)((lcr & ~RB_LCR_WORD_SZ) |
			((uint8_t)(bits - UART_DATA_5BITS) & RB_LCR_WORD_SZ));
	UART_LCR(uart) = lcr;
}

void uart_set_stopbits(uint32_t uart, uint8_t bits)
{
	uint8_t lcr;

	openwch_assert(uart_is_valid(uart));
	openwch_assert((bits == UART_STOPBITS_1) || (bits == UART_STOPBITS_2));

	lcr = UART_LCR(uart);

	if (bits == UART_STOPBITS_2) {
		lcr |= RB_LCR_STOP_BIT;
	} else {
		lcr = (uint8_t)(lcr & ~RB_LCR_STOP_BIT);
	}

	UART_LCR(uart) = lcr;
}

void uart_set_parity(uint32_t uart, uart_parity_t parity)
{
	uint8_t lcr;

	openwch_assert(uart_is_valid(uart));
	openwch_assert(parity <= UART_PARITY_SPACE);

	lcr = UART_LCR(uart);

	switch (parity) {
	case UART_PARITY_NONE:
		lcr = (uint8_t)(lcr & ~(RB_LCR_PAR_EN | RB_LCR_PAR_MOD));
		break;
	case UART_PARITY_ODD:
		lcr = (uint8_t)((lcr & ~RB_LCR_PAR_MOD) | RB_LCR_PAR_EN |
				RB_LCR_PAR_MOD_ODD);
		break;
	case UART_PARITY_EVEN:
		lcr = (uint8_t)((lcr & ~RB_LCR_PAR_MOD) | RB_LCR_PAR_EN |
				RB_LCR_PAR_MOD_EVEN);
		break;
	case UART_PARITY_MARK:
		lcr = (uint8_t)((lcr & ~RB_LCR_PAR_MOD) | RB_LCR_PAR_EN |
				RB_LCR_PAR_MOD_MARK);
		break;
	case UART_PARITY_SPACE:
		lcr = (uint8_t)((lcr & ~RB_LCR_PAR_MOD) | RB_LCR_PAR_EN |
				RB_LCR_PAR_MOD_SPACE);
		break;
	default:
		openwch_assert_not_reached();
		break;
	}

	UART_LCR(uart) = lcr;
}

/* --- Enable -------------------------------------------------------------- */

void uart_enable(uint32_t uart)
{
	openwch_assert(uart_is_valid(uart));

	/* No global enable exists: this is the TXD pin driver. */
	UART_IER(uart) |= RB_IER_TXD_EN;
}

void uart_disable(uint32_t uart)
{
	openwch_assert(uart_is_valid(uart));

	UART_IER(uart) &= (uint8_t)~RB_IER_TXD_EN;
}

/* --- Data ---------------------------------------------------------------- */

void uart_send(uint32_t uart, uint8_t data)
{
	openwch_assert(uart_is_valid(uart));

	UART_THR(uart) = data;
}

uint8_t uart_recv(uint32_t uart)
{
	openwch_assert(uart_is_valid(uart));

	return UART_RBR(uart);
}

void uart_send_blocking(uint32_t uart, uint8_t data)
{
	openwch_assert(uart_is_valid(uart));

	/* Wait until the transmitter FIFO has drained, then queue the byte. */
	while ((UART_LSR(uart) & RB_LSR_TX_FIFO_EMP) == 0) {
		;
	}

	uart_send(uart, data);
}

uint8_t uart_recv_blocking(uint32_t uart)
{
	openwch_assert(uart_is_valid(uart));

	while ((UART_LSR(uart) & RB_LSR_DATA_RDY) == 0) {
		;
	}

	return uart_recv(uart);
}

void uart_write(uint32_t uart, const uint8_t *buf, uint32_t len)
{
	uint32_t i;

	openwch_assert(uart_is_valid(uart));
	openwch_assert((buf != NULL) || (len == 0));

	/*
	 * Unlike uart_send_blocking(), this only waits for FIFO space, so a
	 * whole buffer streams out with the FIFO kept busy instead of draining
	 * between every byte.
	 */
	for (i = 0; i < len; i++) {
		while (UART_TFC(uart) == UART_FIFO_SIZE) {
			;
		}

		UART_THR(uart) = buf[i];
	}
}

void uart_read(uint32_t uart, uint8_t *buf, uint32_t len)
{
	uint32_t i;

	openwch_assert(uart_is_valid(uart));
	openwch_assert((buf != NULL) || (len == 0));

	for (i = 0; i < len; i++) {
		buf[i] = uart_recv_blocking(uart);
	}
}

/* --- Interrupts ---------------------------------------------------------- */

/*
 * RB_MCR_INT_OE gates the UART's interrupt output towards the PFIC.  WCH's own
 * driver sets it whenever an interrupt source is enabled, so the enable paths
 * do the same; the disable paths only clear their own enable bit.
 */

void uart_enable_rx_interrupt(uint32_t uart)
{
	openwch_assert(uart_is_valid(uart));

	UART_IER(uart) |= RB_IER_RECV_RDY;
	UART_MCR(uart) |= RB_MCR_INT_OE;
}

void uart_disable_rx_interrupt(uint32_t uart)
{
	openwch_assert(uart_is_valid(uart));

	UART_IER(uart) &= (uint8_t)~RB_IER_RECV_RDY;
}

void uart_enable_tx_interrupt(uint32_t uart)
{
	openwch_assert(uart_is_valid(uart));

	UART_IER(uart) |= RB_IER_THR_EMPTY;
	UART_MCR(uart) |= RB_MCR_INT_OE;
}

void uart_disable_tx_interrupt(uint32_t uart)
{
	openwch_assert(uart_is_valid(uart));

	UART_IER(uart) &= (uint8_t)~RB_IER_THR_EMPTY;
}

/* --- DMA ----------------------------------------------------------------- */

/*
 * There is nothing to do here.  The CH582/CH583 UART has no DMA request
 * enable: the on-chip DMA controller is wired only to TMR1/TMR2, SPI0/SPI1 and
 * the ADC.  The functions exist so that portable code written against the
 * other WCH families still compiles, and they are deliberately not backed by
 * an invented register bit.
 */

void uart_enable_rx_dma(uint32_t uart)
{
	openwch_assert(uart_is_valid(uart));

	(void)uart;
}

void uart_disable_rx_dma(uint32_t uart)
{
	openwch_assert(uart_is_valid(uart));

	(void)uart;
}

/* --- FIFO ---------------------------------------------------------------- */

void uart_clear_rx_fifo(uint32_t uart)
{
	openwch_assert(uart_is_valid(uart));

	/* Self-clearing strobe. */
	UART_FCR(uart) |= RB_FCR_RX_FIFO_CLR;
}

void uart_clear_tx_fifo(uint32_t uart)
{
	openwch_assert(uart_is_valid(uart));

	UART_FCR(uart) |= RB_FCR_TX_FIFO_CLR;
}

void uart_set_fifo_trigger(uint32_t uart, uart_fifo_trigger_t level)
{
	uint8_t fcr;

	openwch_assert(uart_is_valid(uart));
	openwch_assert(level <= UART_FIFO_TRIGGER_7);

	fcr = UART_FCR(uart);
	fcr = (uint8_t)((fcr & ~RB_FCR_FIFO_TRIG) |
			(((uint8_t)level << 6) & RB_FCR_FIFO_TRIG));
	UART_FCR(uart) = fcr;
}

/* --- Status -------------------------------------------------------------- */

uint8_t uart_get_flag(uint32_t uart, uint8_t flag)
{
	openwch_assert(uart_is_valid(uart));

	return (uint8_t)(UART_LSR(uart) & flag);
}
/**@}*/
