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

/** @addtogroup exti_file EXTI
 *
 * @ingroup CH32V0
 *
 * @brief <b>External Interrupt/Event Controller for the CH32V00x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * Each EXTI line has independent interrupt and event masks plus a pair of
 * edge-select bits.  The pending register is the usual write-1-to-clear
 * arrangement: writing a one to a set bit in EXTI_INTFR clears it.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch32v0/exti.h>
#include <libopenwch/qingke/assert.h>

void exti_set_trigger(uint32_t exti, uint32_t line,
		      enum exti_trigger_type trigger)
{
	openwch_assert((line & ~EXTI_LINE_ALL) == 0);
	openwch_assert(line != 0);

	switch (trigger) {
	case EXTI_TRIGGER_RISING:
		EXTI_RTENR(exti) |= line;
		EXTI_FTENR(exti) &= ~line;
		break;
	case EXTI_TRIGGER_FALLING:
		EXTI_RTENR(exti) &= ~line;
		EXTI_FTENR(exti) |= line;
		break;
	case EXTI_TRIGGER_BOTH:
		EXTI_RTENR(exti) |= line;
		EXTI_FTENR(exti) |= line;
		break;
	default:
		openwch_assert_not_reached();
		break;
	}
}

void exti_enable_request(uint32_t exti, uint32_t line)
{
	openwch_assert((line & ~EXTI_LINE_ALL) == 0);

	EXTI_INTENR(exti) |= line;
}

void exti_disable_request(uint32_t exti, uint32_t line)
{
	openwch_assert((line & ~EXTI_LINE_ALL) == 0);

	EXTI_INTENR(exti) &= ~line;
}

void exti_reset_request(uint32_t exti, uint32_t line)
{
	openwch_assert((line & ~EXTI_LINE_ALL) == 0);

	/* INTFR is rc_w1: write a one to the bit to clear it. */
	EXTI_INTFR(exti) = line;
}

uint32_t exti_get_flag_status(uint32_t exti, uint32_t line)
{
	openwch_assert((line & ~EXTI_LINE_ALL) == 0);

	return EXTI_INTFR(exti) & line;
}

void exti_trigger_software(uint32_t exti, uint32_t line)
{
	openwch_assert((line & ~EXTI_LINE_ALL) == 0);

	EXTI_SWIEVR(exti) |= line;
}

void exti_enable_event(uint32_t exti, uint32_t line)
{
	openwch_assert((line & ~EXTI_LINE_ALL) == 0);

	EXTI_EVENR(exti) |= line;
}

void exti_disable_event(uint32_t exti, uint32_t line)
{
	openwch_assert((line & ~EXTI_LINE_ALL) == 0);

	EXTI_EVENR(exti) &= ~line;
}
/**@}*/
