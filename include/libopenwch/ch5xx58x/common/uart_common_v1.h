/** @addtogroup uart_defines UART Defines

@brief <b>Defined Constants and Types for the CH58x UARTs</b>

@ingroup CH5XX58X_defines

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA UART.H */

/** @cond */
#if defined(LIBOPENWCH_UART_H) || defined(LIBOPENWCH_UART_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_UART_COMMON_V1_H
#define LIBOPENWCH_UART_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * The CH58x has four identical 16550-style UARTs, UART0..UART3.  Each has an
 * 8-byte transmit FIFO, an 8-byte receive FIFO, a 16-bit divisor latch, and
 * separate interrupt-enable, line-status and modem registers.  Every register
 * is byte wide except the divisor latch, which is one 16-bit view over the
 * DLL and DLM bytes at 0x0c.
 *
 * The block differs from the CH32V00x USART in two ways that matter to a
 * driver:
 *
 *   - the baud rate is a plain 16-bit divisor, not a 12.4 fixed-point value,
 *     so uart_set_baudrate() computes WCH's divisor directly (see the .c);
 *   - there is no global UART enable bit.  The transmitter's pin driver is
 *     gated by IER.TXD_EN, which is what uart_enable() switches on.  The
 *     receiver is always listening.
 *
 * None of these registers is RWA, so no safe-access window is needed.
 */

/* --- Register offsets ---------------------------------------------------- */

/** Modem control, 8-bit. */
#define UART_MCR_OFFSET 0x00
/** Interrupt enable, 8-bit. */
#define UART_IER_OFFSET 0x01
/** FIFO control, 8-bit. */
#define UART_FCR_OFFSET 0x02
/** Line control, 8-bit. */
#define UART_LCR_OFFSET 0x03
/** Interrupt identification, 8-bit, read only. */
#define UART_IIR_OFFSET 0x04
/** Line status, 8-bit, read only. */
#define UART_LSR_OFFSET 0x05
/** Modem status, 8-bit, read only. */
#define UART_MSR_OFFSET 0x06
/** Receiver buffer, 8-bit, read only. */
#define UART_RBR_OFFSET 0x08
/** Transmitter holding, 8-bit, write only. */
#define UART_THR_OFFSET 0x08
/** Receiver FIFO count, 8-bit, read only. */
#define UART_RFC_OFFSET 0x0a
/** Transmitter FIFO count, 8-bit, read only. */
#define UART_TFC_OFFSET 0x0b
/** Divisor latch, LSB, 8-bit. */
#define UART_DLL_OFFSET 0x0c
/** Divisor latch, MSB, 8-bit. */
#define UART_DLM_OFFSET 0x0d
/** Divisor latch, 16-bit view over DLL and DLM. */
#define UART_DL_OFFSET 0x0c
/** Pre-divisor latch, 8-bit, low seven bits, 1..128. */
#define UART_DIV_OFFSET 0x0e
/** Slave address, 8-bit; 0xff disables address matching. */
#define UART_ADR_OFFSET 0x0f

/* --- Register accessors -------------------------------------------------- */

#define UART_MCR(uart) MMIO8((uart) + UART_MCR_OFFSET)
#define UART_IER(uart) MMIO8((uart) + UART_IER_OFFSET)
#define UART_FCR(uart) MMIO8((uart) + UART_FCR_OFFSET)
#define UART_LCR(uart) MMIO8((uart) + UART_LCR_OFFSET)
#define UART_IIR(uart) MMIO8((uart) + UART_IIR_OFFSET)
#define UART_LSR(uart) MMIO8((uart) + UART_LSR_OFFSET)
#define UART_MSR(uart) MMIO8((uart) + UART_MSR_OFFSET)
#define UART_RBR(uart) MMIO8((uart) + UART_RBR_OFFSET)
#define UART_THR(uart) MMIO8((uart) + UART_THR_OFFSET)
#define UART_RFC(uart) MMIO8((uart) + UART_RFC_OFFSET)
#define UART_TFC(uart) MMIO8((uart) + UART_TFC_OFFSET)
#define UART_DLL(uart) MMIO8((uart) + UART_DLL_OFFSET)
#define UART_DLM(uart) MMIO8((uart) + UART_DLM_OFFSET)
#define UART_DL(uart) MMIO16((uart) + UART_DL_OFFSET)
#define UART_DIV(uart) MMIO8((uart) + UART_DIV_OFFSET)
#define UART_ADR(uart) MMIO8((uart) + UART_ADR_OFFSET)

/** FIFO depth, in bytes. */
#define UART_FIFO_SIZE 8
/** Largest RX FIFO trigger level the hardware offers. */
#define UART_RECV_RDY_SZ 7

/* --- MCR bits ------------------------------------------------------------ */

/** @defgroup uart_mcr_bits UART modem control bits
@ingroup uart_defines
@{*/
#define RB_MCR_DTR 0x01	       /**< data terminal ready */
#define RB_MCR_RTS 0x02	       /**< request to send */
#define RB_MCR_OUT1 0x04       /**< general-purpose output 1 */
#define RB_MCR_OUT2 0x08       /**< general-purpose output 2 */
#define RB_MCR_INT_OE 0x08     /**< interrupt output enable */
#define RB_MCR_LOOP 0x10       /**< local loopback */
#define RB_MCR_AU_FLOW_EN 0x20 /**< automatic flow control */
#define RB_MCR_TNOW 0x40       /**< TNOW on the DTR pin */
#define RB_MCR_HALF 0x80       /**< half duplex */
/**@}*/

/* --- IER bits ------------------------------------------------------------ */

/** @defgroup uart_ier_bits UART interrupt enable bits
@ingroup uart_defines
@{*/
#define RB_IER_RECV_RDY 0x01  /**< receiver data available */
#define RB_IER_THR_EMPTY 0x02 /**< transmitter holding empty */
#define RB_IER_LINE_STAT 0x04 /**< receiver line status */
#define RB_IER_MODEM_CHG 0x08 /**< modem status change (UART0 only) */
#define RB_IER_DTR_EN 0x10    /**< DTR/TNOW output pin enable */
#define RB_IER_RTS_EN 0x20    /**< RTS output pin enable */
#define RB_IER_TXD_EN 0x40    /**< TXD output pin enable */
#define RB_IER_RESET 0x80     /**< write-only self-clearing reset */
/**@}*/

/* --- FCR bits and fields ------------------------------------------------- */

/** @defgroup uart_fcr_bits UART FIFO control bits
@ingroup uart_defines
@{*/
#define RB_FCR_FIFO_EN 0x01	/**< FIFO mode enable */
#define RB_FCR_RX_FIFO_CLR 0x02 /**< self-clearing RX FIFO flush */
#define RB_FCR_TX_FIFO_CLR 0x04 /**< self-clearing TX FIFO flush */
#define RB_FCR_FIFO_TRIG 0xc0	/**< bits 7:6, RX FIFO trigger level */
/**@}*/

/* --- LCR bits and fields ------------------------------------------------- */

/** @defgroup uart_lcr_bits UART line control bits
@ingroup uart_defines
@{*/
#define RB_LCR_WORD_SZ 0x03  /**< bits 1:0, word length */
#define RB_LCR_STOP_BIT 0x04 /**< 0 = 1 stop bit, 1 = 2 stop bits */
#define RB_LCR_PAR_EN 0x08   /**< parity enable */
#define RB_LCR_PAR_MOD 0x30  /**< bits 5:4, parity mode */
#define RB_LCR_BREAK_EN 0x40 /**< break control */
/**@}*/

/*
 * Encodings of the RB_LCR_PAR_MOD field, from the CH58x register map:
 * 00 = odd, 01 = even, 10 = mark, 11 = space.  Only meaningful while
 * RB_LCR_PAR_EN is set.
 */
#define RB_LCR_PAR_MOD_ODD 0x00
#define RB_LCR_PAR_MOD_EVEN 0x10
#define RB_LCR_PAR_MOD_MARK 0x20
#define RB_LCR_PAR_MOD_SPACE 0x30

/* --- IIR bits and interrupt identifications ------------------------------ */

/** @defgroup uart_iir_bits UART interrupt identification bits
@ingroup uart_defines
@{*/
#define RB_IIR_NO_INT 0x01   /**< 1 = no interrupt is pending */
#define RB_IIR_INT_MASK 0x0f /**< interrupt identification mask */
#define RB_IIR_FIFO_ID 0xc0  /**< FIFO mode indicator */
/**@}*/

/** @defgroup uart_irq_id UART interrupt identification values
@ingroup uart_defines

The low nibble of UART_IIR(), masked with RB_IIR_INT_MASK, is one of these.
@{*/
#define UART_II_MODEM_CHANGE 0x00 /**< modem status change (UART0) */
#define UART_II_THR_EMPTY 0x02	  /**< transmitter holding empty */
#define UART_II_RX_READY 0x04	  /**< receiver data available */
#define UART_II_RX_TIMEOUT 0x0c	  /**< receiver FIFO timeout */
#define UART_II_LINE_STATUS 0x06  /**< receiver line status */
#define UART_II_SLAVE_ADDR 0x0e	  /**< slave address match */
#define UART_II_NO_INTERRUPT 0x01 /**< no interrupt pending */
/**@}*/

/* --- LSR bits ------------------------------------------------------------ */

/** @defgroup uart_lsr_bits UART line status bits
@ingroup uart_defines
@{*/
#define RB_LSR_DATA_RDY 0x01	/**< receiver FIFO has data */
#define RB_LSR_OVER_ERR 0x02	/**< receiver overrun, read-clear */
#define RB_LSR_PAR_ERR 0x04	/**< parity error, read-clear */
#define RB_LSR_FRAME_ERR 0x08	/**< framing error, read-clear */
#define RB_LSR_BREAK_ERR 0x10	/**< break received, read-clear */
#define RB_LSR_TX_FIFO_EMP 0x20 /**< transmitter FIFO empty */
#define RB_LSR_TX_ALL_EMP 0x40	/**< transmitter completely empty */
#define RB_LSR_ERR_RX_FIFO 0x80 /**< an error sits in the RX FIFO */
/**@}*/

/* --- MSR bits ------------------------------------------------------------ */

/** @defgroup uart_msr_bits UART modem status bits
@ingroup uart_defines
@{*/
#define RB_MSR_CTS_CHG 0x01 /**< CTS changed, read-clear */
#define RB_MSR_DSR_CHG 0x02 /**< DSR changed, read-clear */
#define RB_MSR_CTS 0x10	    /**< current CTS level */
#define RB_MSR_DSR 0x20	    /**< current DSR level */
/**@}*/

/* --- Line-status flags --------------------------------------------------- */

/** @defgroup uart_flags UART line-status flags
@ingroup uart_defines

Passed to uart_get_flag(), which returns the bits of UART_LSR() that are set
in the requested mask.
@{*/
#define UART_FLAG_RX_DATA_READY RB_LSR_DATA_RDY
#define UART_FLAG_RX_OVERRUN RB_LSR_OVER_ERR
#define UART_FLAG_RX_PARITY_ERROR RB_LSR_PAR_ERR
#define UART_FLAG_RX_FRAME_ERROR RB_LSR_FRAME_ERR
#define UART_FLAG_RX_BREAK RB_LSR_BREAK_ERR
#define UART_FLAG_TX_FIFO_EMPTY RB_LSR_TX_FIFO_EMP
#define UART_FLAG_TX_ALL_EMPTY RB_LSR_TX_ALL_EMP
#define UART_FLAG_RX_FIFO_ERROR RB_LSR_ERR_RX_FIFO
/**@}*/

/* --- Word length --------------------------------------------------------- */

/** @defgroup uart_databits UART word lengths
@ingroup uart_defines
@{*/
#define UART_DATA_5BITS 5
#define UART_DATA_6BITS 6
#define UART_DATA_7BITS 7
#define UART_DATA_8BITS 8
/**@}*/

/* --- Stop bits ----------------------------------------------------------- */

/** @defgroup uart_stopbits UART stop-bit counts
@ingroup uart_defines
@{*/
#define UART_STOPBITS_1 1
#define UART_STOPBITS_2 2
/**@}*/

/* --- Parity -------------------------------------------------------------- */

/** UART parity selection. */
typedef enum {
	UART_PARITY_NONE = 0, /**< no parity bit */
	UART_PARITY_ODD,      /**< odd parity */
	UART_PARITY_EVEN,     /**< even parity */
	UART_PARITY_MARK,     /**< mark (always 1) */
	UART_PARITY_SPACE,    /**< space (always 0) */
} uart_parity_t;

/* --- Receiver FIFO trigger level ----------------------------------------- */

/** UART receiver FIFO trigger level. */
typedef enum {
	UART_FIFO_TRIGGER_1 = 0, /**< interrupt at 1 byte */
	UART_FIFO_TRIGGER_2,	 /**< interrupt at 2 bytes */
	UART_FIFO_TRIGGER_4,	 /**< interrupt at 4 bytes */
	UART_FIFO_TRIGGER_7,	 /**< interrupt at 7 bytes */
} uart_fifo_trigger_t;

BEGIN_DECLS

/* --- Line format --------------------------------------------------------- */

/**
 * Set the baud rate from the current system clock.  The divisor is derived
 * with WCH's formula x = 10 * Fsys / 8 / baud, x = (x + 5) / 10, and written
 * to the 16-bit divisor latch.
 */
void uart_set_baudrate(uint32_t uart, uint32_t baud);
/** Set the word length: UART_DATA_5BITS .. UART_DATA_8BITS. */
void uart_set_databits(uint32_t uart, uint8_t bits);
/** Set the number of stop bits: UART_STOPBITS_1 or UART_STOPBITS_2. */
void uart_set_stopbits(uint32_t uart, uint8_t bits);
/** Set the parity mode. */
void uart_set_parity(uint32_t uart, uart_parity_t parity);

/* --- Enable -------------------------------------------------------------- */

/**
 * Enable the transmitter output pin.  The CH58x UART has no global enable
 * bit, so this sets IER.TXD_EN; the receiver is always enabled.
 */
void uart_enable(uint32_t uart);
void uart_disable(uint32_t uart);

/* --- Data ---------------------------------------------------------------- */

/** Queue one byte without waiting for FIFO space. */
void uart_send(uint32_t uart, uint8_t data);
/** Read one byte from the receiver buffer without waiting. */
uint8_t uart_recv(uint32_t uart);

/** Wait for FIFO space and then queue one byte. */
void uart_send_blocking(uint32_t uart, uint8_t data);
/** Wait for a received byte and then read it. */
uint8_t uart_recv_blocking(uint32_t uart);

/** Write `len` bytes, waiting only for FIFO space. */
void uart_write(uint32_t uart, const uint8_t *buf, uint32_t len);
/** Read exactly `len` bytes, waiting for each one. */
void uart_read(uint32_t uart, uint8_t *buf, uint32_t len);

/* --- Interrupts ---------------------------------------------------------- */

void uart_enable_rx_interrupt(uint32_t uart);
void uart_disable_rx_interrupt(uint32_t uart);
void uart_enable_tx_interrupt(uint32_t uart);
void uart_disable_tx_interrupt(uint32_t uart);

/* --- DMA ----------------------------------------------------------------- */

/*
 * The CH582/CH583 UART block has no DMA request gate: only the timers, the
 * SPIs and the ADC can drive the on-chip DMA controller.  These two entry
 * points are kept for API uniformity with the other WCH families and are
 * documented no-ops here rather than inventing a register bit.
 */
void uart_enable_rx_dma(uint32_t uart);
void uart_disable_rx_dma(uint32_t uart);

/* --- FIFO ---------------------------------------------------------------- */

void uart_clear_rx_fifo(uint32_t uart);
void uart_clear_tx_fifo(uint32_t uart);
/** Set the receiver FIFO interrupt trigger level. */
void uart_set_fifo_trigger(uint32_t uart, uart_fifo_trigger_t level);

/* --- Status -------------------------------------------------------------- */

/** Return the requested UART_FLAG_* bits of the line status register. */
uint8_t uart_get_flag(uint32_t uart, uint8_t flag);

END_DECLS

#endif
/** @cond */
#else
#warning "uart_common_v1.h should not be included explicitly, only via uart.h"
#endif
/** @endcond */
/**@}*/
