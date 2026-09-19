/** @addtogroup tmr_defines TMR Defines

@brief <b>Defined Constants and Types for the CH58x timers</b>

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA TMR.H */

/** @cond */
#if defined(LIBOPENWCH_TMR_H) || defined(LIBOPENWCH_TMR_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_TMR_COMMON_V1_H
#define LIBOPENWCH_TMR_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * The CH58x has four identical timer blocks, TMR0..TMR3, at 0x400-aligned
 * addresses.  Each one is a 26-bit up counter driving one pin that is used
 * either as a PWM output or as a capture input:
 *
 *	TMR0  PA9  / PB23 (remapped)
 *	TMR1  PA10 / PB10 (remapped)
 *	TMR2  PA11 / PB11 (remapped)
 *	TMR3  PA2  / PB22 (remapped)
 *
 * The pin function is not selected in the timer at all: it is the
 * R16_PIN_ALTERNATE remap bit (GPIO_REMAP_TMRn in gpio.h) plus the pin's own
 * direction.  See gpio_pin_remap().
 *
 * Mode control is a single byte, R8_TMRn_CTRL_MOD, whose two halves also act
 * as the FIFO wait-level and end-of-cycle settings.
 *
 * These are peripheral registers, NOT RWA: no safe-access window is needed.
 * The TMR0..TMR3 clock gates (R8_SLP_CLK_OFF0) reset to "clock running", so
 * there is nothing to switch on before use.
 *
 * TMR0/TMR3 vs TMR1/TMR2
 * ----------------------
 * TMR1 and TMR2 add a DMA controller: an extra CTRL_DMA byte at offset 0x01
 * and the DMA_NOW / DMA_BEG / DMA_END address registers at 0x14 / 0x18 /
 * 0x1C.  TMR0 and TMR3 have no such registers and no DMA interrupt flag.  The
 * shared accessors below are defined for every instance because they encode
 * the same offsets, but tmr_enable_dma() asserts that the instance it was
 * given actually has the hardware.
 */

/* --- Register offsets ---------------------------------------------------- */

/** Mode control, 8-bit. */
#define TMR_CTRL_MOD			0x00
/** DMA control, 8-bit; TMR1/TMR2 only. */
#define TMR_CTRL_DMA			0x01
/** Interrupt enable, 8-bit. */
#define TMR_INTER_EN			0x02
/** Status word, 32-bit. */
#define TMR_STATUS			0x04
/** Interrupt flag, 8-bit, write 1 to clear. */
#define TMR_INT_FLAG			0x06
/** Capture FIFO occupancy, 8-bit, read only. */
#define TMR_FIFO_COUNT			0x07
/** Current count, 32-bit. */
#define TMR_COUNT			0x08
/** End-of-cycle value, 32-bit, only the low 26 bits are used. */
#define TMR_CNT_END			0x0c
/** Capture / PWM data FIFO, 32-bit, only the low 26 bits are used. */
#define TMR_FIFO			0x10
/** DMA current address, 32-bit; TMR1/TMR2 only. */
#define TMR_DMA_NOW			0x14
/** DMA start address, 32-bit; TMR1/TMR2 only. */
#define TMR_DMA_BEG			0x18
/** DMA end address, 32-bit; TMR1/TMR2 only. */
#define TMR_DMA_END			0x1c

/** Capture FIFO depth, in entries. */
#define TMR_FIFO_SIZE			8

/*
 * The counter width.  WCH's drivers use 67108864 (2^26) as "the longest
 * period", but CNT_END is only implemented for the low 26 bits, so the
 * largest value a full wrap can come back from is 2^26 - 1.
 */
#define TMR_COUNT_MAX			0x3ffffffu

/* --- Register accessors -------------------------------------------------- */

#define TMR_CTRL_MOD_REG(tmr)		MMIO8((tmr) + TMR_CTRL_MOD)
#define TMR_CTRL_DMA_REG(tmr)		MMIO8((tmr) + TMR_CTRL_DMA)
#define TMR_INTER_EN_REG(tmr)		MMIO8((tmr) + TMR_INTER_EN)
#define TMR_STATUS_REG(tmr)		MMIO32((tmr) + TMR_STATUS)
#define TMR_INT_FLAG_REG(tmr)		MMIO8((tmr) + TMR_INT_FLAG)
#define TMR_FIFO_COUNT_REG(tmr)		MMIO8((tmr) + TMR_FIFO_COUNT)
#define TMR_COUNT_REG(tmr)		MMIO32((tmr) + TMR_COUNT)
#define TMR_CNT_END_REG(tmr)		MMIO32((tmr) + TMR_CNT_END)
#define TMR_FIFO_REG(tmr)		MMIO32((tmr) + TMR_FIFO)
#define TMR_DMA_NOW_REG(tmr)		MMIO32((tmr) + TMR_DMA_NOW)
#define TMR_DMA_BEG_REG(tmr)		MMIO32((tmr) + TMR_DMA_BEG)
#define TMR_DMA_END_REG(tmr)		MMIO32((tmr) + TMR_DMA_END)

/* --- CTRL_MOD bits ------------------------------------------------------- */

/** @defgroup tmr_mode_bits TMR CTRL_MOD bits

The low nibble defines the direction; bits 5:4 and 7:6 are re-used depending
on RB_TMR_MODE_IN.
@{*/
#define RB_TMR_MODE_IN			0x01	/**< 0 = timer/PWM, 1 = capture/count */
#define RB_TMR_ALL_CLEAR		0x02	/**< force-clear the FIFO and the count */
#define RB_TMR_COUNT_EN			0x04	/**< counting enable */
#define RB_TMR_OUT_EN			0x08	/**< output (PWM) enable */
#define RB_TMR_OUT_POLAR		0x10	/**< 0 = active high, 1 = active low */
#define RB_TMR_CAP_COUNT		0x10	/**< with MODE_IN: 1 = edge count */
#define RB_TMR_PWM_REPEAT		0xc0	/**< bits 7:6, PWM repeat count */
#define RB_TMR_CAP_EDGE			0xc0	/**< bits 7:6, capture edge mode */
/**@}*/

/* --- CTRL_DMA bits (TMR1/TMR2 only) -------------------------------------- */

/** @defgroup tmr_dma_bits TMR CTRL_DMA bits
@ingroup tmr_defines
@{*/
#define RB_TMR_DMA_ENABLE		0x01	/**< DMA enable */
#define RB_TMR_DMA_LOOP			0x04	/**< restart at DMA_BEG when DMA_END is hit */
/**@}*/

/* --- Interrupt and flag bits --------------------------------------------- */

/** @defgroup tmr_irq_bits TMR interrupt and flag bits

The same bit positions are used in TMR_INTER_EN and TMR_INT_FLAG.  The last
two are meaningless on TMR0 and TMR3, which have no DMA controller.
@{*/
#define RB_TMR_IE_CYC_END		0x01	/**< capture timeout or PWM cycle end */
#define RB_TMR_IE_DATA_ACT		0x02	/**< capture data ready or PWM trigger */
#define RB_TMR_IE_FIFO_HF		0x04	/**< capture FIFO >= 4, PWM FIFO <= 3 */
#define RB_TMR_IE_DMA_END		0x08	/**< DMA complete; TMR1/TMR2 only */
#define RB_TMR_IE_FIFO_OV		0x10	/**< capture FIFO full / PWM FIFO empty */

#define RB_TMR_IF_CYC_END		0x01
#define RB_TMR_IF_DATA_ACT		0x02
#define RB_TMR_IF_FIFO_HF		0x04
#define RB_TMR_IF_DMA_END		0x08	/**< TMR1/TMR2 only */
#define RB_TMR_IF_FIFO_OV		0x10
/**@}*/

/** Every bit the flag register is guaranteed to have on all four instances. */
#define TMR_IF_ALL			0x1f
/** The bits TMR0 and TMR3 do not have. */
#define TMR_IF_DMA_ONLY			0x08
/** Every interrupt enable the four instances share. */
#define TMR_IE_ALL			0x1f

/* --- Timer modes --------------------------------------------------------- */

/** @defgroup tmr_modes TMR Modes

Passed to tmr_set_mode().  RB_TMR_ALL_CLEAR is deliberately not offered: it is
a self-clearing strobe, and tmr_set_mode() already pulses it so that whatever
is being configured starts from a clean FIFO and count.
@{*/
#define TMR_MODE_TIMER			0x00	/**< count Tsys, interrupt at CNT_END */
#define TMR_MODE_PWM			0x00	/**< produce a PWM waveform */
#define TMR_MODE_CAPTURE		0x01	/**< capture edges, RB_TMR_MODE_IN */
#define TMR_MODE_COUNT			0x11	/**< count edges, MODE_IN | CAP_COUNT */
/**@}*/

/* --- Capture sub-modes --------------------------------------------------- */

/** @defgroup tmr_capture_modes TMR Capture Edge Modes

A capture unit latches the count on an edge, and the same two bits select which
edge pairs delimit a capture window.
@{*/
#define TMR_CAPTURE_DISABLE		0x00	/**< no capture and no edge count */
#define TMR_CAPTURE_EDGE		0x40	/**< any edge to any edge */
#define TMR_CAPTURE_FALLING		0x80	/**< falling edge to falling edge */
#define TMR_CAPTURE_RISING		0xc0	/**< rising edge to rising edge */
/**@}*/

/* --- PWM waveform polarity ----------------------------------------------- */

/** @defgroup tmr_polarity TMR PWM Polarity
@{*/
#define TMR_POLARITY_ACTIVE_HIGH	0x00	/**< idle low, active high */
#define TMR_POLARITY_ACTIVE_LOW		0x10	/**< idle high, active low */
/**@}*/

/* --- PWM repeat count ---------------------------------------------------- */

/** @defgroup tmr_pwm_repeat TMR PWM Repeat Count

How many effective-level periods are emitted per trigger.
@{*/
#define TMR_PWM_REPEAT_1		0x00
#define TMR_PWM_REPEAT_4		0x40
#define TMR_PWM_REPEAT_8		0x80
#define TMR_PWM_REPEAT_16		0xc0
/**@}*/

/* --- DMA modes ----------------------------------------------------------- */

/** @defgroup tmr_dma_modes TMR DMA Modes
@{*/
#define TMR_DMA_DISABLE			0x00	/**< stop and disable the DMA engine */
#define TMR_DMA_SINGLE			0x01	/**< one pass from DMA_BEG to DMA_END */
#define TMR_DMA_LOOP			0x05	/**< restart at DMA_BEG, DMA_LOOP */
/**@}*/

/** True when `tmr` is a TMR1/TMR2-style instance with a DMA controller. */
#define TMR_HAS_DMA(tmr) \
	(((tmr) == TMR1_BASE) || ((tmr) == TMR2_BASE))

BEGIN_DECLS

/* --- Mode and run control ------------------------------------------------ */

void tmr_set_mode(uint32_t tmr, uint32_t mode);
void tmr_enable(uint32_t tmr);
void tmr_disable(uint32_t tmr);

/* --- Period and count ---------------------------------------------------- */

void tmr_set_period(uint32_t tmr, uint32_t cycles);
void tmr_set_count(uint32_t tmr, uint32_t cycles);
uint32_t tmr_get_count(uint32_t tmr);

/* --- Capture / edge count ------------------------------------------------ */

void tmr_set_capture_mode(uint32_t tmr, uint32_t mode);
uint32_t tmr_get_capture(uint32_t tmr);

/* --- PWM output ---------------------------------------------------------- */

void tmr_set_pwm_polarity(uint32_t tmr, uint32_t polarity);
void tmr_enable_pwm(uint32_t tmr);
void tmr_disable_pwm(uint32_t tmr);
void tmr_set_pwm_repeat(uint32_t tmr, uint32_t repeat);

/* --- Interrupts ---------------------------------------------------------- */

void tmr_enable_irq(uint32_t tmr, uint32_t irq);
void tmr_disable_irq(uint32_t tmr, uint32_t irq);
uint32_t tmr_get_flag(uint32_t tmr, uint32_t flag);
void tmr_clear_flag(uint32_t tmr, uint32_t flag);

/* --- DMA; TMR1 and TMR2 only --------------------------------------------- */

void tmr_enable_dma(uint32_t tmr, uint32_t mode, uint32_t start, uint32_t end);

END_DECLS

#endif
/** @cond */
#else
#warning "tmr_common_v1.h should not be included explicitly, only via tmr.h"
#endif
/** @endcond */
/**@}*/
