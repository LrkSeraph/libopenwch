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

/** @addtogroup pwm_file PWM
 *
 * @ingroup CH5XX58X
 *
 * @brief <b>PWM4..PWM11 for the CH58x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * One block, eight channels sharing a single period generator.  A channel is
 * identified by its bit mask (PWM_CH4 .. PWM_CH11), which is simultaneously its
 * index into the contiguous data registers, so enabling a channel, setting its
 * polarity and setting its duty are all one masked byte operation.
 *
 * pwm_enable_channel() and pwm_disable_channel() take a mask because OUT_EN is
 * bit-parallel.  pwm_set_channel() and pwm_set_polarity() take exactly one
 * channel, because a data register holds one value.
 *
 * The polarity convention is WCH's: "active high" means the channel idles low
 * and pulses high, "active low" means it idles high and pulses low.
 *
 * These registers are plain read/write; no safe-access window is involved.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch5xx58x/pwm.h>
#include <libopenwch/qingke/assert.h>

/*
 * Every entry point validates its channel against pwm_channel_index() or, for
 * the two bit-parallel registers, against the channel mask.  The checks are
 * asserts, so a release build compiles them out; the explicit early returns
 * stay in place so that an out-of-range channel can never spill a write into a
 * neighbouring register.
 */

void pwm_set_cycle(uint32_t pwm, uint32_t cycle) {
	openwch_assert((cycle & ~(uint32_t)PWM_CYCLE_MASK) == 0);

	/* The cycle generator is shared, so only its own two fields change. */
	PWM_CONFIG_REG(pwm) = (uint8_t)((PWM_CONFIG_REG(pwm)
					 & (uint8_t)~PWM_CYCLE_MASK)
					| (cycle & PWM_CYCLE_MASK));
}

void pwm_set_clock_divider(uint32_t pwm, uint32_t div) {
	openwch_assert(div <= PWM_DATA_MASK);

	/* The channel clock is (div + 1) * Tsys. */
	PWM_CLOCK_DIV_REG(pwm) = (uint8_t)div;
}

void pwm_set_channel(uint32_t pwm, uint32_t channel, uint32_t duty) {
	int index = pwm_channel_index(channel);

	openwch_assert(index >= 0);
	openwch_assert(duty <= PWM_DATA_MASK);
	if (index < 0) {
		/* Not a channel: never let the write land in a neighbouring
		 * register. */
		return;
	}

	MMIO8(pwm + PWM_DATA_OFFSET((uint32_t)index)) = (uint8_t)duty;
}

void pwm_set_polarity(uint32_t pwm, uint32_t channel, uint32_t polarity) {
	/* Only one channel can be described by one polarity bit at a time. */
	uint32_t mask = (pwm_channel_index(channel) >= 0) ? channel : 0;

	openwch_assert(mask != 0);
	openwch_assert(polarity <= PWM_POLARITY_ACTIVE_LOW);
	if (mask == 0) {
		return;
	}

	if (polarity == PWM_POLARITY_ACTIVE_LOW) {
		PWM_POLAR_REG(pwm) |= (uint8_t)mask;
	} else {
		PWM_POLAR_REG(pwm) &= (uint8_t)~mask;
	}
}

void pwm_enable_channel(uint32_t pwm, uint32_t channel) {
	openwch_assert(channel != 0);
	openwch_assert((channel & ~(uint32_t)PWM_CH_ALL) == 0);

	PWM_OUT_EN_REG(pwm) |= (uint8_t)channel;
}

void pwm_disable_channel(uint32_t pwm, uint32_t channel) {
	openwch_assert(channel != 0);
	openwch_assert((channel & ~(uint32_t)PWM_CH_ALL) == 0);

	PWM_OUT_EN_REG(pwm) &= (uint8_t)~channel;
}

void pwm_enable_alternate(uint32_t pwm, uint32_t channel) {
	/*
	 * "Alternate output" staggers a pair of channels onto the same pin: the
	 * two channels drive their effective level one after the other instead
	 * of at the same time.  Only whole pairs can be selected, so a mask
	 * that is not one of the four group bits is a programming error.
	 */
	openwch_assert(channel == RB_PWM4_5_STAG_EN ||
		       channel == RB_PWM6_7_STAG_EN ||
		       channel == RB_PWM8_9_STAG_EN ||
		       channel == RB_PWM10_11_STAG_EN);

	PWM_CONFIG_REG(pwm) |= (uint8_t)channel;
}
/**@}*/
