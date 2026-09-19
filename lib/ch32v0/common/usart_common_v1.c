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

/** @addtogroup usart_file USART
 *
 * @ingroup CH32V0
 *
 * @brief <b>USART for the CH32V00x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * The baud rate generator is the STM32-style 12.4 fixed-point divider:
 *
 *	USARTDIV = fPCLK / (16 * baud)
 *	BRR = (mantissa << 4) | round(fraction * 16)
 *
 * Note that the CH32V00x BRR register holds the *divider*, not the baud rate
 * value the way WCH's EVT names its initialisation field, so the conversion is
 * done here from the clock frequency reported by the RCC driver.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch32v0/usart.h>
#include <libopenwch/ch32v0/rcc.h>
#include <libopenwch/qingke/assert.h>

/** Largest value the 12.4 divider can represent. */
#define USART_BRR_MAX_DIVIDER		0xffffu

void usart_set_baudrate(uint32_t usart, uint32_t baud) {
	struct rcc_clock_scale clocks;
	uint32_t divider;

	openwch_assert(baud != 0);

	/* USART1 is on APB2. */
	rcc_get_clocks_freq(&clocks);

	/*
	 * The 12.4 divider is round(fPCLK / baud): the four fraction bits are
	 * the sixteenths of a bit period.  Round to nearest.
	 */
	divider = (clocks.pclk2 + (baud / 2u)) / baud;

	/*
	 * A divider above 0xffff cannot be represented, which happens when the
	 * requested baud rate is too low for the current APB2 clock.  Truncating
	 * would silently give a completely wrong rate, so trap instead: the
	 * caller either raises the clock or lowers the baud rate.
	 */
	openwch_assert(divider <= USART_BRR_MAX_DIVIDER);

	/*
	 * BRR = mantissa[15:4] | fraction[3:0].  The divider is already in
	 * units of 1/16 bit, so the low four bits are the fraction as-is.
	 */
	USART_BRR(usart) = (uint16_t)divider;
}

void usart_set_databits(uint32_t usart, uint8_t bits) {
	uint16_t reg = USART_CTLR1(usart);

	if (bits == 9) {
		reg |= USART_CTLR1_M;
	} else {
		openwch_assert(bits == 8);
		reg &= ~USART_CTLR1_M;
	}

	USART_CTLR1(usart) = reg;
}

void usart_set_stopbits(uint32_t usart, uint8_t stopbits) {
	uint16_t reg = USART_CTLR2(usart);

	openwch_assert((stopbits & ~0x3u) == 0);

	reg = (uint16_t)((reg & ~USART_CTLR2_STOP_MASK)
			| ((stopbits << USART_CTLR2_STOP_SHIFT) & USART_CTLR2_STOP_MASK));
	USART_CTLR2(usart) = reg;
}

void usart_set_parity(uint32_t usart, uint8_t parity) {
	uint16_t reg = USART_CTLR1(usart);

	switch (parity) {
	case USART_PARITY_NONE:
		reg &= ~(USART_CTLR1_PCE | USART_CTLR1_PS);
		break;
	case USART_PARITY_EVEN:
		reg = (uint16_t)((reg | USART_CTLR1_PCE) & ~USART_CTLR1_PS);
		break;
	case USART_PARITY_ODD:
		reg |= USART_CTLR1_PCE | USART_CTLR1_PS;
		break;
	default:
		openwch_assert_not_reached();
		break;
	}

	USART_CTLR1(usart) = reg;
}

void usart_set_mode(uint32_t usart, uint32_t mode) {
	uint16_t reg = USART_CTLR1(usart);

	reg = (uint16_t)((reg & ~(USART_CTLR1_RE | USART_CTLR1_TE))
			| (mode & (USART_CTLR1_RE | USART_CTLR1_TE)));
	USART_CTLR1(usart) = reg;
}

void usart_set_flow_control(uint32_t usart, uint32_t flowcontrol) {
	uint16_t reg = USART_CTLR3(usart);

	reg = (uint16_t)((reg & ~(USART_CTLR3_RTSE | USART_CTLR3_CTSE))
			| (flowcontrol & (USART_CTLR3_RTSE | USART_CTLR3_CTSE)));
	USART_CTLR3(usart) = reg;
}

void usart_set_clock(uint32_t usart, uint32_t clock) {
	uint16_t reg = USART_CTLR2(usart);

	reg = (uint16_t)((reg & ~(USART_CTLR2_CLKEN | USART_CTLR2_CPOL
					| USART_CTLR2_CPHA | USART_CTLR2_LBCL))
			| (clock & (USART_CTLR2_CLKEN | USART_CTLR2_CPOL
					| USART_CTLR2_CPHA | USART_CTLR2_LBCL)));
	USART_CTLR2(usart) = reg;
}

void usart_enable(uint32_t usart) {
	USART_CTLR1(usart) |= USART_CTLR1_UE;
}

void usart_disable(uint32_t usart) {
	USART_CTLR1(usart) &= ~USART_CTLR1_UE;
}

void usart_send(uint32_t usart, uint16_t data) {
	USART_DATAR(usart) = (uint16_t)(data & USART_DATAR_DR_MASK);
}

uint16_t usart_recv(uint32_t usart) {
	return (uint16_t)(USART_DATAR(usart) & USART_DATAR_DR_MASK);
}

void usart_wait_send_ready(uint32_t usart) {
	while (!(USART_STATR(usart) & USART_STATR_TXE)) {
		;
	}
}

void usart_send_blocking(uint32_t usart, uint16_t data) {
	usart_wait_send_ready(usart);
	usart_send(usart, data);
}

void usart_write(uint32_t usart, const uint8_t *data, uint32_t len) {
	uint32_t i;

	for (i = 0; i < len; i++) {
		usart_send_blocking(usart, data[i]);
	}
}

uint16_t usart_recv_blocking(uint32_t usart) {
	while (!(USART_STATR(usart) & USART_STATR_RXNE)) {
		;
	}

	return usart_recv(usart);
}

uint16_t usart_get_flag(uint32_t usart, uint16_t flag) {
	return (uint16_t)(USART_STATR(usart) & flag);
}

void usart_clear_flag(uint32_t usart, uint16_t flag) {
	/*
	 * STATR is a mix of read-only status bits and rc_w0 bits.  Only the
	 * clearable ones may be written; writing a 1 to the others is a no-op,
	 * so masking with the clearable set keeps this safe.
	 */
	USART_STATR(usart) = (uint16_t)(flag & (USART_STATR_CTS | USART_STATR_LBD));
}

void usart_enable_rx_interrupt(uint32_t usart) {
	USART_CTLR1(usart) |= USART_CTLR1_RXNEIE;
}

void usart_disable_rx_interrupt(uint32_t usart) {
	USART_CTLR1(usart) &= ~USART_CTLR1_RXNEIE;
}

void usart_enable_tx_interrupt(uint32_t usart) {
	USART_CTLR1(usart) |= USART_CTLR1_TXEIE;
}

void usart_disable_tx_interrupt(uint32_t usart) {
	USART_CTLR1(usart) &= ~USART_CTLR1_TXEIE;
}

void usart_enable_rx_dma(uint32_t usart) {
	USART_CTLR3(usart) |= USART_CTLR3_DMAR;
}

void usart_disable_rx_dma(uint32_t usart) {
	USART_CTLR3(usart) &= ~USART_CTLR3_DMAR;
}

void usart_enable_tx_dma(uint32_t usart) {
	USART_CTLR3(usart) |= USART_CTLR3_DMAT;
}

void usart_disable_tx_dma(uint32_t usart) {
	USART_CTLR3(usart) &= ~USART_CTLR3_DMAT;
}
/**@}*/
