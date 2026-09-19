/** @addtogroup pwm_defines PWM Defines

@brief <b>Defined Constants and Types for the CH58x PWMX block</b>

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA PWM.H */

/** @cond */
#if defined(LIBOPENWCH_PWM_H) || defined(LIBOPENWCH_PWM_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_PWM_COMMON_V1_H
#define LIBOPENWCH_PWM_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * PWMX is one 8-channel PWM block controlling PWM4..PWM11.  It shares nothing
 * with TMR0..TMR3 except the pin remap matrix: PWM4/PWM5/PWM7/PWM8/PWM9 have
 * an alternate pin group selected by GPIO_REMAP_PWMX, PWM6/PWM10/PWM11 do not.
 *
 * Unlike the timers, PWMX has no per-channel period.  All eight channels share
 * one cycle generator, whose length is selected by RB_PWM_CYC_MOD (8/7/6/5 bits
 * of data) together with RB_PWM_CYCLE_SEL (256/128/64/32 or the corresponding
 * minus-one cycle).  Each channel only holds its own pulse width, in
 * R8_PWMn_DATA, and optionally its own polarity.
 *
 * The eight data registers are contiguous from offset 0x04, which is what
 * makes pwm_set_channel() a single indexed byte store.
 *
 * These registers are plain read/write, NOT RWA.  The PWMX clock gate
 * (R8_SLP_CLK_OFF1 bit 2) resets to "clock running".
 */

/* --- Register offsets ---------------------------------------------------- */

/** Output enable, 8-bit, one bit per channel. */
#define PWM_OUT_EN 0x00
/** Output polarity, 8-bit, one bit per channel. */
#define PWM_POLAR 0x01
/** Configuration: data width, cycle selection, stagger, 8-bit. */
#define PWM_CONFIG 0x02
/** Clock divisor, 8-bit: the channel clock is (div + 1) * Tsys. */
#define PWM_CLOCK_DIV 0x03
/** Data for PWM4..PWM7, 8-bit each, increasing with the channel number. */
#define PWM4_DATA_HOLD 0x04
#define PWM5_DATA_HOLD 0x05
#define PWM6_DATA_HOLD 0x06
#define PWM7_DATA_HOLD 0x07
/** Data for PWM8..PWM11, 8-bit each, increasing with the channel number. */
#define PWM8_DATA_HOLD 0x08
#define PWM9_DATA_HOLD 0x09
#define PWM10_DATA_HOLD 0x0a
#define PWM11_DATA_HOLD 0x0b
/** Interrupt control, 8-bit. */
#define PWM_INT_CTRL 0x0c

/* --- Register accessors -------------------------------------------------- */

#define PWM_OUT_EN_REG(pwm) MMIO8((pwm) + PWM_OUT_EN)
#define PWM_POLAR_REG(pwm) MMIO8((pwm) + PWM_POLAR)
#define PWM_CONFIG_REG(pwm) MMIO8((pwm) + PWM_CONFIG)
#define PWM_CLOCK_DIV_REG(pwm) MMIO8((pwm) + PWM_CLOCK_DIV)
#define PWM_INT_CTRL_REG(pwm) MMIO8((pwm) + PWM_INT_CTRL)

/*
 * The data registers are contiguous, in channel order, from PWM4_DATA_HOLD:
 * PWM_CH4 is at offset 4, PWM_CH5 at 5, ... PWM_CH11 at 11.  A data register is
 * therefore (base + PWM4_DATA_HOLD + channel index), and the index is produced
 * by pwm_channel_index() below.
 */

/* --- Channel identifiers ------------------------------------------------- */

/** @defgroup pwm_channel_id PWM Channel Identifiers

The value is the channel's bit in OUT_EN and POLAR, which is also its 0-based
index within the data-register block, so the same constant addresses both.
@{*/
#define PWM_CH4 (1u << 0)
#define PWM_CH5 (1u << 1)
#define PWM_CH6 (1u << 2)
#define PWM_CH7 (1u << 3)
#define PWM_CH8 (1u << 4)
#define PWM_CH9 (1u << 5)
#define PWM_CH10 (1u << 6)
#define PWM_CH11 (1u << 7)
/**@}*/

/* WCH's own spelling, kept as an alias. */
#define CH_PWM4 PWM_CH4
#define CH_PWM5 PWM_CH5
#define CH_PWM6 PWM_CH6
#define CH_PWM7 PWM_CH7
#define CH_PWM8 PWM_CH8
#define CH_PWM9 PWM_CH9
#define CH_PWM10 PWM_CH10
#define CH_PWM11 PWM_CH11

#define PWM_CH_ALL 0xffu
/** Width of a data register, in bits, at the widest cycle setting. */
#define PWM_DATA_MASK 0xffu

/** Data register offset for a channel given as its 0-based index. */
#define PWM_DATA_OFFSET(index) (PWM4_DATA_HOLD + (index))

/**
 * 0-based data-register index of a single channel.
 *
 * Returns -1 for anything that is not exactly one of PWM_CH4..PWM_CH11, so
 * that PWM_DATA_REG() cannot index outside the data block.  This is a switch
 * rather than __builtin_ctz() on purpose: the builtin would make the library
 * depend on the libgcc __ctzsi2 helper.
 */
static inline int pwm_channel_index(uint32_t channel) {
	switch (channel) {
	case PWM_CH4:
		return 0;
	case PWM_CH5:
		return 1;
	case PWM_CH6:
		return 2;
	case PWM_CH7:
		return 3;
	case PWM_CH8:
		return 4;
	case PWM_CH9:
		return 5;
	case PWM_CH10:
		return 6;
	case PWM_CH11:
		return 7;
	default:
		return -1;
	}
}

/** Data register for one channel, given as a single-channel identifier. */
#define PWM_DATA_REG(pwm, ch)                                                  \
	MMIO8((pwm) + PWM_DATA_OFFSET(pwm_channel_index(ch)))

/* --- CONFIG fields ------------------------------------------------------- */

/** @defgroup pwm_config_bits PWM CONFIG bits
@{*/
#define RB_PWM_CYCLE_SEL 0x01	 /**< 0 = 2^n cycles, 1 = 2^n - 1 */
#define RB_PWM_STAG_ST 0x02	 /**< read only: stagger cycle status */
#define RB_PWM_CYC_MOD 0x0c	 /**< bits 3:2, data width */
#define RB_PWM4_5_STAG_EN 0x10	 /**< PWM4/PWM5 alternate output */
#define RB_PWM6_7_STAG_EN 0x20	 /**< PWM6/PWM7 alternate output */
#define RB_PWM8_9_STAG_EN 0x40	 /**< PWM8/PWM9 alternate output */
#define RB_PWM10_11_STAG_EN 0x80 /**< PWM10/PWM11 alternate output */
/**@}*/

/* --- INT_CTRL fields ----------------------------------------------------- */

/** @defgroup pwm_int_bits PWM INT_CTRL bits
@{*/
#define RB_PWM_IE_CYC 0x01  /**< enable the cycle-end interrupt */
#define RB_PWM_CYC_PRE 0x02 /**< interrupt point: 1 = earlier */
#define RB_PWM_IF_CYC 0x80  /**< cycle-end flag, write 1 to clear */
/**@}*/

/* --- Cycle settings ------------------------------------------------------ */

/** @defgroup pwm_cycles PWM Cycle Settings

Passed to pwm_set_cycle().  The value is exactly what goes into the
RB_PWM_CYC_MOD / RB_PWM_CYCLE_SEL field of PWM_CONFIG.  Note that the cycle
generator is shared by all eight channels.
@{*/
#define PWM_CYCLE_256 0x00 /**< 8-bit data, 256 clocks */
#define PWM_CYCLE_255 0x01 /**< 8-bit data, 255 clocks */
#define PWM_CYCLE_128 0x04 /**< 7-bit data, 128 clocks */
#define PWM_CYCLE_127 0x05 /**< 7-bit data, 127 clocks */
#define PWM_CYCLE_64 0x08  /**< 6-bit data, 64 clocks */
#define PWM_CYCLE_63 0x09  /**< 6-bit data, 63 clocks */
#define PWM_CYCLE_32 0x0c  /**< 5-bit data, 32 clocks */
#define PWM_CYCLE_31 0x0d  /**< 5-bit data, 31 clocks */
/** The bits of PWM_CONFIG that pwm_set_cycle() owns. */
#define PWM_CYCLE_MASK (RB_PWM_CYC_MOD | RB_PWM_CYCLE_SEL)
/**@}*/

/* --- Output polarity ----------------------------------------------------- */

/** @defgroup pwm_polarity PWM Output Polarity
@{*/
#define PWM_POLARITY_ACTIVE_HIGH 0 /**< idle low, active high */
#define PWM_POLARITY_ACTIVE_LOW 1  /**< idle high, active low */
/**@}*/

BEGIN_DECLS

void pwm_set_cycle(uint32_t pwm, uint32_t cycle);
void pwm_set_clock_divider(uint32_t pwm, uint32_t div);
void pwm_set_channel(uint32_t pwm, uint32_t channel, uint32_t duty);
void pwm_set_polarity(uint32_t pwm, uint32_t channel, uint32_t polarity);
void pwm_enable_channel(uint32_t pwm, uint32_t channel);
void pwm_disable_channel(uint32_t pwm, uint32_t channel);
void pwm_enable_alternate(uint32_t pwm, uint32_t channel);

END_DECLS

#endif
/** @cond */
#else
#warning "pwm_common_v1.h should not be included explicitly, only via pwm.h"
#endif
/** @endcond */
/**@}*/
