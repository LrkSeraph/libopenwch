/** @addtogroup exti_defines EXTI Defines

@brief <b>Defined Constants and Types for the CH32V00x EXTI</b>

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA EXTI.H
The order of header inclusion is important: exti.h includes the device
specific memorymap.h header before including this header file. */

/** @cond */
#if defined(LIBOPENWCH_EXTI_H) || defined(LIBOPENWCH_EXTI_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_EXTI_COMMON_V1_H
#define LIBOPENWCH_EXTI_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * The CH32V00x EXTI block owns the per-line trigger, mask and pending
 * registers.  Selecting *which* port drives a line 0..7 is not done here: that
 * lives in the AFIO block and is exposed by the GPIO driver as
 * gpio_exti_select_source().
 *
 * The lines are:
 *	0..7	GPIO (port selected through AFIO_EXTICR)
 *	8	PVD output
 *	9	auto-wakeup event
 *
 * Every mask register has one bit per line, so a "line" argument is really a
 * bit mask: pass EXTI0, EXTI3 or EXTI0 | EXTI3 for several lines at once.
 */

/* --- Register accessors -------------------------------------------------- */

/* Interrupt mask register (EXTI_INTENR) */
#define EXTI_INTENR(exti) MMIO32((exti) + 0x00)
/* Event mask register (EXTI_EVENR) */
#define EXTI_EVENR(exti) MMIO32((exti) + 0x04)
/* Rising trigger selection register (EXTI_RTENR) */
#define EXTI_RTENR(exti) MMIO32((exti) + 0x08)
/* Falling trigger selection register (EXTI_FTENR) */
#define EXTI_FTENR(exti) MMIO32((exti) + 0x0c)
/* Software interrupt event register (EXTI_SWIEVR) */
#define EXTI_SWIEVR(exti) MMIO32((exti) + 0x10)
/* Pending register (EXTI_INTFR) */
#define EXTI_INTFR(exti) MMIO32((exti) + 0x14)

/* --- Line identifiers ---------------------------------------------------- */

/** @defgroup exti_lines EXTI Line Bit Masks
@ingroup exti_defines

@{*/
#define EXTI0 (1 << 0)	      /**< line 0 (GPIO) */
#define EXTI1 (1 << 1)	      /**< line 1 (GPIO) */
#define EXTI2 (1 << 2)	      /**< line 2 (GPIO) */
#define EXTI3 (1 << 3)	      /**< line 3 (GPIO) */
#define EXTI4 (1 << 4)	      /**< line 4 (GPIO) */
#define EXTI5 (1 << 5)	      /**< line 5 (GPIO) */
#define EXTI6 (1 << 6)	      /**< line 6 (GPIO) */
#define EXTI7 (1 << 7)	      /**< line 7 (GPIO) */
#define EXTI8 (1 << 8)	      /**< line 8 (PVD output) */
#define EXTI9 (1 << 9)	      /**< line 9 (auto-wakeup) */
#define EXTI_LINE_ALL 0x03ffu /**< lines 0..9 */
/**@}*/

/* --- Interrupt/event edge selection -------------------------------------- */

/** @defgroup exti_trigger EXTI Trigger Types
@ingroup exti_defines

@{*/
enum exti_trigger_type {
	EXTI_TRIGGER_RISING = 0, /**< rising edge only */
	EXTI_TRIGGER_FALLING,	 /**< falling edge only */
	EXTI_TRIGGER_BOTH,	 /**< both edges */
};
/**@}*/

BEGIN_DECLS

void exti_set_trigger(uint32_t exti,
		      uint32_t line,
		      enum exti_trigger_type trigger);
void exti_enable_request(uint32_t exti, uint32_t line);
void exti_disable_request(uint32_t exti, uint32_t line);
void exti_reset_request(uint32_t exti, uint32_t line);
uint32_t exti_get_flag_status(uint32_t exti, uint32_t line);
void exti_trigger_software(uint32_t exti, uint32_t line);
void exti_enable_event(uint32_t exti, uint32_t line);
void exti_disable_event(uint32_t exti, uint32_t line);

END_DECLS

#endif
/** @cond */
#else
#warning "exti_common_v1.h should not be included explicitly, only via exti.h"
#endif
/** @endcond */
/**@}*/
