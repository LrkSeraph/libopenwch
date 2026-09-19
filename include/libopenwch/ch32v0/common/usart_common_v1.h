/** @addtogroup usart_defines USART Defines

@brief <b>Defined Constants and Types for the CH32V00x USART</b>

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA USART.H
The order of header inclusion is important: usart.h includes the device
specific memorymap.h header before including this header file. */

/** @cond */
#if defined(LIBOPENWCH_USART_H) || defined(LIBOPENWCH_USART_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_USART_COMMON_V1_H
#define LIBOPENWCH_USART_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * The CH32V00x USART is the classic STM32-style block, but the CH32 version
 * spaces the 16-bit registers out to 32-bit boundaries.  Every register is
 * therefore accessed as MMIO16 at an offset of 4 * n.
 */

/* --- Register accessors -------------------------------------------------- */

#define USART_STATR(usart)		MMIO16((usart) + 0x00)
#define USART_DATAR(usart)		MMIO16((usart) + 0x04)
#define USART_BRR(usart)		MMIO16((usart) + 0x08)
#define USART_CTLR1(usart)		MMIO16((usart) + 0x0c)
#define USART_CTLR2(usart)		MMIO16((usart) + 0x10)
#define USART_CTLR3(usart)		MMIO16((usart) + 0x14)
#define USART_GPR(usart)		MMIO16((usart) + 0x18)

/* --- STATR bits ---------------------------------------------------------- */

#define USART_STATR_PE			(1 << 0)	/**< parity error */
#define USART_STATR_FE			(1 << 1)	/**< framing error */
#define USART_STATR_NE			(1 << 2)	/**< noise error */
#define USART_STATR_ORE			(1 << 3)	/**< overrun error */
#define USART_STATR_IDLE		(1 << 4)	/**< idle line detected */
#define USART_STATR_RXNE		(1 << 5)	/**< read data register not empty */
#define USART_STATR_TC			(1 << 6)	/**< transmission complete */
#define USART_STATR_TXE			(1 << 7)	/**< transmit data register empty */
#define USART_STATR_LBD			(1 << 8)	/**< LIN break detected */
#define USART_STATR_CTS			(1 << 9)	/**< CTS changed */

/* --- DATAR --------------------------------------------------------------- */

#define USART_DATAR_DR_MASK		0x01ffu

/* --- BRR ----------------------------------------------------------------- */

#define USART_BRR_DIV_FRACTION_SHIFT	0
#define USART_BRR_DIV_FRACTION_MASK	0x000fu
#define USART_BRR_DIV_MANTISSA_SHIFT	4
#define USART_BRR_DIV_MANTISSA_MASK	0xfff0u

/* --- CTLR1 bits ---------------------------------------------------------- */

#define USART_CTLR1_SBK			(1 << 0)	/**< send break */
#define USART_CTLR1_RWU			(1 << 1)	/**< receiver wakeup */
#define USART_CTLR1_RE			(1 << 2)	/**< receiver enable */
#define USART_CTLR1_TE			(1 << 3)	/**< transmitter enable */
#define USART_CTLR1_IDLEIE		(1 << 4)	/**< idle interrupt enable */
#define USART_CTLR1_RXNEIE		(1 << 5)	/**< RXNE interrupt enable */
#define USART_CTLR1_TCIE		(1 << 6)	/**< transmission complete IE */
#define USART_CTLR1_TXEIE		(1 << 7)	/**< TXE interrupt enable */
#define USART_CTLR1_PEIE		(1 << 8)	/**< parity error IE */
#define USART_CTLR1_PS			(1 << 9)	/**< parity selection: 0 even */
#define USART_CTLR1_PCE			(1 << 10)	/**< parity control enable */
#define USART_CTLR1_WAKE		(1 << 11)	/**< wakeup method */
#define USART_CTLR1_M			(1 << 12)	/**< word length: 1 = 9 bits */
#define USART_CTLR1_UE			(1 << 13)	/**< USART enable */

/* --- CTLR2 bits ---------------------------------------------------------- */

#define USART_CTLR2_ADD_MASK		0x000fu
#define USART_CTLR2_LBDL		(1 << 5)
#define USART_CTLR2_LBDIE		(1 << 6)
#define USART_CTLR2_LBCL		(1 << 8)
#define USART_CTLR2_CPHA		(1 << 9)
#define USART_CTLR2_CPOL		(1 << 10)
#define USART_CTLR2_CLKEN		(1 << 11)
#define USART_CTLR2_STOP_SHIFT		12
#define USART_CTLR2_STOP_MASK		(0x3u << 12)
#define USART_CTLR2_LINEN		(1 << 14)

/* --- CTLR3 bits ---------------------------------------------------------- */

#define USART_CTLR3_EIE			(1 << 0)
#define USART_CTLR3_IREN		(1 << 1)
#define USART_CTLR3_IRLP		(1 << 2)
#define USART_CTLR3_HDSEL		(1 << 3)
#define USART_CTLR3_NACK		(1 << 4)
#define USART_CTLR3_SCEN		(1 << 5)
#define USART_CTLR3_DMAR		(1 << 6)	/**< DMA enable receiver */
#define USART_CTLR3_DMAT		(1 << 7)	/**< DMA enable transmitter */
#define USART_CTLR3_RTSE		(1 << 8)
#define USART_CTLR3_CTSE		(1 << 9)
#define USART_CTLR3_CTSIE		(1 << 10)

/* --- GPR ----------------------------------------------------------------- */

#define USART_GPR_PSC_SHIFT		0
#define USART_GPR_PSC_MASK		0x00ffu
#define USART_GPR_GT_SHIFT		8
#define USART_GPR_GT_MASK		0xff00u

/* --- Configuration enumerations ------------------------------------------ */

/** @defgroup usart_stop_bits USART Stop Bits
@ingroup usart_defines
@{*/
#define USART_STOPBITS_1		0x0
#define USART_STOPBITS_0_5		0x1
#define USART_STOPBITS_2		0x2
#define USART_STOPBITS_1_5		0x3
/**@}*/

/** @defgroup usart_parity USART Parity
@ingroup usart_defines
@{*/
#define USART_PARITY_NONE		0x0
#define USART_PARITY_EVEN		0x1
#define USART_PARITY_ODD		0x2
/**@}*/

/** @defgroup usart_databits USART Data Bits
@ingroup usart_defines
@{*/
#define USART_DATABITS_8		0x0
#define USART_DATABITS_9		0x1
/**@}*/

/** @defgroup usart_mode USART Mode
@ingroup usart_defines
@{*/
#define USART_MODE_RX			USART_CTLR1_RE
#define USART_MODE_TX			USART_CTLR1_TE
#define USART_MODE_TX_RX		(USART_CTLR1_RE | USART_CTLR1_TE)
/**@}*/

/** @defgroup usart_flow_control USART Flow Control
@ingroup usart_defines
@{*/
#define USART_FLOWCONTROL_NONE		0x0
#define USART_FLOWCONTROL_RTS		USART_CTLR3_RTSE
#define USART_FLOWCONTROL_CTS		USART_CTLR3_CTSE
#define USART_FLOWCONTROL_RTS_CTS	(USART_CTLR3_RTSE | USART_CTLR3_CTSE)
/**@}*/

/** @defgroup usart_clock USART Clock (synchronous mode)
@ingroup usart_defines
@{*/
#define USART_CLOCK_DISABLE		0x0
#define USART_CLOCK_ENABLE		USART_CTLR2_CLKEN
#define USART_CLOCK_CPOL_LOW		0x0
#define USART_CLOCK_CPOL_HIGH		USART_CTLR2_CPOL
#define USART_CLOCK_CPHA_1EDGE		0x0
#define USART_CLOCK_CPHA_2EDGE		USART_CTLR2_CPHA
#define USART_CLOCK_LASTBIT_DISABLE	0x0
#define USART_CLOCK_LASTBIT_ENABLE	USART_CTLR2_LBCL
/**@}*/

/** A standard baud rate, for convenience. */
#define USART_BAUDRATE_115200		115200u
#define USART_BAUDRATE_9600		9600u

BEGIN_DECLS

/* --- Configuration (call before usart_enable) ---------------------------- */

void usart_set_baudrate(uint32_t usart, uint32_t baud);
void usart_set_databits(uint32_t usart, uint8_t bits);
void usart_set_stopbits(uint32_t usart, uint8_t stopbits);
void usart_set_parity(uint32_t usart, uint8_t parity);
void usart_set_mode(uint32_t usart, uint32_t mode);
void usart_set_flow_control(uint32_t usart, uint32_t flowcontrol);
void usart_set_clock(uint32_t usart, uint32_t clock);

/* --- Enable ------------------------------------------------------------- */

void usart_enable(uint32_t usart);
void usart_disable(uint32_t usart);

/* --- Data --------------------------------------------------------------- */

void usart_send(uint32_t usart, uint16_t data);
uint16_t usart_recv(uint32_t usart);

/** Blocking transmit of one byte, waiting for TXE. */
void usart_wait_send_ready(uint32_t usart);
/** Blocking transmit of a whole buffer. */
void usart_send_blocking(uint32_t usart, uint16_t data);
void usart_write(uint32_t usart, const uint8_t *data, uint32_t len);

/** Blocking receive of one byte, waiting for RXNE. */
uint16_t usart_recv_blocking(uint32_t usart);

/* --- Flags and interrupts ------------------------------------------------ */

uint16_t usart_get_flag(uint32_t usart, uint16_t flag);
void usart_clear_flag(uint32_t usart, uint16_t flag);
void usart_enable_rx_interrupt(uint32_t usart);
void usart_disable_rx_interrupt(uint32_t usart);
void usart_enable_tx_interrupt(uint32_t usart);
void usart_disable_tx_interrupt(uint32_t usart);

/* --- DMA ---------------------------------------------------------------- */

void usart_enable_rx_dma(uint32_t usart);
void usart_disable_rx_dma(uint32_t usart);
void usart_enable_tx_dma(uint32_t usart);
void usart_disable_tx_dma(uint32_t usart);

END_DECLS

#endif
/** @cond */
#else
#warning "usart_common_v1.h should not be included explicitly, only via usart.h"
#endif
/** @endcond */
/**@}*/
