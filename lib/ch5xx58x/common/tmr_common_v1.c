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

/** @addtogroup tmr_file TMR
 *
 * @ingroup CH5XX58X
 *
 * @brief <b>Timer, PWM and capture units for the CH58x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * TMR0..TMR3 are four independent 26-bit blocks with a shared register map.
 * Each has one pin that is routed by the GPIO remap matrix, not by anything in
 * the timer, and each can be used in three ways:
 *
 *	timer      -- count Tsys, interrupt every CNT_END + 1 cycles
 *	PWM        -- emit a waveform with the period in CNT_END and the
 *		      effective pulse width in FIFO
 *	capture    -- latch the count on edges of the pin, or count the edges
 *
 * tmr_set_mode() selects between them, and pulses RB_TMR_ALL_CLEAR so that a
 * reconfigured timer never starts with a stale FIFO or count.  The mode,
 * polarity and repeat fields share CTRL_MOD, so a setter only touches its own
 * bits and leaves the rest of the configuration alone.
 *
 * The registers are plain read/write; no safe-access window is involved.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch5xx58x/tmr.h>
#include <libopenwch/qingke/assert.h>

/*
 * Mode, polarity, repeat count and capture edge all live in the one CTRL_MOD
 * byte, so tmr_set_capture_mode(), tmr_set_pwm_polarity() and
 * tmr_set_pwm_repeat() are read-modify-writes that each own their bits and
 * leave the rest alone.  That is safe here because the manual gives every bit
 * of the register a defined value in both directions -- in particular
 * RB_TMR_ALL_CLEAR is RW with 0 = "not clear", so a read-modify-write never
 * silently re-triggers the clear strobe.
 *
 * The TMR1/TMR2-only entry points check TMR_HAS_DMA() before touching CTRL_DMA
 * or a DMA address register.  That check is an assert, like every other
 * argument check in this driver, so NDEBUG compiles it out; a release build
 * then lets the caller use DMA only on an instance that has it.
 */

void tmr_set_mode(uint32_t tmr, uint32_t mode) {
	openwch_assert(mode == TMR_MODE_TIMER || mode == TMR_MODE_PWM ||
		       mode == TMR_MODE_CAPTURE || mode == TMR_MODE_COUNT);

	/*
	 * RB_TMR_ALL_CLEAR is a self-clearing strobe, so it has to be written
	 * on its own; the requested mode follows immediately.
	 */
	TMR_CTRL_MOD_REG(tmr) = RB_TMR_ALL_CLEAR;
	TMR_CTRL_MOD_REG(tmr) = (uint8_t)mode;
}

void tmr_enable(uint32_t tmr) {
	TMR_CTRL_MOD_REG(tmr) |= RB_TMR_COUNT_EN;
}

void tmr_disable(uint32_t tmr) {
	/* Stop the output as well, so that no partial waveform is left on the
	 * pin once the count is halted. */
	TMR_CTRL_MOD_REG(tmr) &= (uint8_t)~(RB_TMR_COUNT_EN | RB_TMR_OUT_EN);
}

void tmr_set_period(uint32_t tmr, uint32_t cycles) {
	openwch_assert(cycles <= TMR_COUNT_MAX);

	TMR_CNT_END_REG(tmr) = cycles & TMR_COUNT_MAX;
}

void tmr_set_count(uint32_t tmr, uint32_t cycles) {
	openwch_assert(cycles <= TMR_COUNT_MAX);

	TMR_COUNT_REG(tmr) = cycles & TMR_COUNT_MAX;
}

uint32_t tmr_get_count(uint32_t tmr) {
	return TMR_COUNT_REG(tmr) & TMR_COUNT_MAX;
}

void tmr_set_capture_mode(uint32_t tmr, uint32_t mode) {
	openwch_assert(
	    mode == TMR_CAPTURE_DISABLE || mode == TMR_CAPTURE_EDGE ||
	    mode == TMR_CAPTURE_FALLING || mode == TMR_CAPTURE_RISING);

	/* Only bits 7:6 belong to the capture unit; MODE_IN / CAP_COUNT are the
	 * caller's choice of tmr_set_mode(). */
	TMR_CTRL_MOD_REG(tmr) =
	    (uint8_t)((TMR_CTRL_MOD_REG(tmr) & (uint8_t)~RB_TMR_CAP_EDGE) |
		      (mode & RB_TMR_CAP_EDGE));
}

uint32_t tmr_get_capture(uint32_t tmr) {
	/* Reading the FIFO pops the oldest captured value. */
	return TMR_FIFO_REG(tmr) & TMR_COUNT_MAX;
}

void tmr_set_pwm_polarity(uint32_t tmr, uint32_t polarity) {
	openwch_assert(polarity == TMR_POLARITY_ACTIVE_HIGH ||
		       polarity == TMR_POLARITY_ACTIVE_LOW);

	if (polarity == TMR_POLARITY_ACTIVE_LOW) {
		TMR_CTRL_MOD_REG(tmr) |= RB_TMR_OUT_POLAR;
	} else {
		TMR_CTRL_MOD_REG(tmr) &= (uint8_t)~RB_TMR_OUT_POLAR;
	}
}

void tmr_enable_pwm(uint32_t tmr) {
	TMR_CTRL_MOD_REG(tmr) |= RB_TMR_OUT_EN;
}

void tmr_disable_pwm(uint32_t tmr) {
	TMR_CTRL_MOD_REG(tmr) &= (uint8_t)~RB_TMR_OUT_EN;
}

void tmr_set_pwm_repeat(uint32_t tmr, uint32_t repeat) {
	openwch_assert(
	    repeat == TMR_PWM_REPEAT_1 || repeat == TMR_PWM_REPEAT_4 ||
	    repeat == TMR_PWM_REPEAT_8 || repeat == TMR_PWM_REPEAT_16);

	TMR_CTRL_MOD_REG(tmr) =
	    (uint8_t)((TMR_CTRL_MOD_REG(tmr) & (uint8_t)~RB_TMR_PWM_REPEAT) |
		      (repeat & RB_TMR_PWM_REPEAT));
}

void tmr_enable_irq(uint32_t tmr, uint32_t irq) {
	openwch_assert((irq & ~(uint32_t)TMR_IE_ALL) == 0);
	openwch_assert((irq & TMR_IF_DMA_ONLY) == 0 || TMR_HAS_DMA(tmr));

	TMR_INTER_EN_REG(tmr) |= (uint8_t)irq;
}

void tmr_disable_irq(uint32_t tmr, uint32_t irq) {
	openwch_assert((irq & ~(uint32_t)TMR_IE_ALL) == 0);
	openwch_assert((irq & TMR_IF_DMA_ONLY) == 0 || TMR_HAS_DMA(tmr));

	TMR_INTER_EN_REG(tmr) &= (uint8_t)~irq;
}

uint32_t tmr_get_flag(uint32_t tmr, uint32_t flag) {
	openwch_assert((flag & ~(uint32_t)TMR_IF_ALL) == 0);
	openwch_assert((flag & TMR_IF_DMA_ONLY) == 0 || TMR_HAS_DMA(tmr));

	return TMR_INT_FLAG_REG(tmr) & flag;
}

void tmr_clear_flag(uint32_t tmr, uint32_t flag) {
	openwch_assert((flag & ~(uint32_t)TMR_IF_ALL) == 0);
	openwch_assert((flag & TMR_IF_DMA_ONLY) == 0 || TMR_HAS_DMA(tmr));

	/* R8_TMRn_INT_FLAG is write-1-to-clear. */
	TMR_INT_FLAG_REG(tmr) = (uint8_t)flag;
}

void tmr_enable_dma(uint32_t tmr, uint32_t mode, uint32_t start, uint32_t end) {
	openwch_assert(TMR_HAS_DMA(tmr));
	openwch_assert(mode == TMR_DMA_DISABLE || mode == TMR_DMA_SINGLE ||
		       mode == TMR_DMA_LOOP);

	if (mode == TMR_DMA_DISABLE) {
		TMR_CTRL_DMA_REG(tmr) = 0;
		return;
	}

	/*
	 * DMA_BEG and DMA_END are 16-bit halves of their 32-bit registers, so
	 * only a 16-bit address space can be described.  The FIFO is at
	 * 0x40000000 + tmr + 0x10, which is out of reach; the WCH EVT driver
	 * has the same restriction and is used the same way, with the encode
	 * address inside the low 64 KiB.
	 */
	openwch_assert(start <= 0xffffu);
	openwch_assert(end <= 0xffffu);

	MMIO16((tmr) + TMR_DMA_BEG) = (uint16_t)start;
	MMIO16((tmr) + TMR_DMA_END) = (uint16_t)end;
	TMR_CTRL_DMA_REG(tmr) = (uint8_t)mode;
}
/**@}*/
