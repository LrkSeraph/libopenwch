/** @addtogroup pwr_defines PWR Defines

@brief <b>Defined Constants and Types for the CH32V00x PWR</b>

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA PWR.H
The order of header inclusion is important: pwr.h includes the device
specific memorymap.h header before including this header file. */

/** @cond */
#if defined(LIBOPENWCH_PWR_H) || defined(LIBOPENWCH_PWR_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_PWR_COMMON_V1_H
#define LIBOPENWCH_PWR_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * The CH32V00x power-control block holds the power-voltage detector, the
 * standby-mode control bit and the auto-wakeup unit.
 *
 * The auto-wakeup counter runs from the LSI oscillator; the wakeup period is
 *
 *	t = (AWUWR + 1) * 2^AWUPSC / fLSI
 */

/* --- Register accessors -------------------------------------------------- */

/* Power control register (PWR_CTLR) */
#define PWR_CTLR(pwr)			MMIO32((pwr) + 0x00)
/* Power control/status register (PWR_CSR) */
#define PWR_CSR(pwr)			MMIO32((pwr) + 0x04)
/* Auto-wakeup control/status register (PWR_AWUCSR) */
#define PWR_AWUCSR(pwr)			MMIO32((pwr) + 0x08)
/* Auto-wakeup window register (PWR_AWUWR) */
#define PWR_AWUWR(pwr)			MMIO32((pwr) + 0x0c)
/* Auto-wakeup prescaler register (PWR_AWUPSC) */
#define PWR_AWUPSC(pwr)			MMIO32((pwr) + 0x10)

/* --- PWR_CTLR bits ------------------------------------------------------- */

#define PWR_CTLR_PDDS			(1 << 1)	/**< power-down deepsleep */
#define PWR_CTLR_PVDE			(1 << 4)	/**< PVD enable */
#define PWR_CTLR_PLS_MASK		(0x7u << 5)	/**< PVD level field */
#define PWR_CTLR_PLS_0			(1 << 5)
#define PWR_CTLR_PLS_1			(1 << 6)
#define PWR_CTLR_PLS_2			(1 << 7)

/* --- PWR_CSR bits -------------------------------------------------------- */

#define PWR_CSR_PVDO			(1 << 2)	/**< PVD output */

/* --- PWR_AWUCSR bits ----------------------------------------------------- */

#define PWR_AWUCSR_AWUEN		(1 << 1)	/**< auto-wakeup enable */

/* --- PWR_AWUWR / PWR_AWUPSC fields --------------------------------------- */

#define PWR_AWUWR_MASK			0x3fu		/**< window field */
#define PWR_AWUPSC_MASK			0x0fu		/**< prescaler field */

/* --- PVD detection levels ------------------------------------------------ */

/** @defgroup pwr_pvd_level PWR PVD Detection Levels
@ingroup pwr_defines

@{*/
enum pwr_pvd_level {
	PWR_PVD_LEVEL_0 = 0x00,		/**< 2.9 V */
	PWR_PVD_LEVEL_1 = 0x20,		/**< 3.1 V */
	PWR_PVD_LEVEL_2 = 0x40,		/**< 3.3 V */
	PWR_PVD_LEVEL_3 = 0x60,		/**< 3.5 V */
	PWR_PVD_LEVEL_4 = 0x80,		/**< 3.7 V */
	PWR_PVD_LEVEL_5 = 0xa0,		/**< 3.9 V */
	PWR_PVD_LEVEL_6 = 0xc0,		/**< 4.1 V */
	PWR_PVD_LEVEL_7 = 0xe0,		/**< 4.4 V */
};
/**@}*/

/* --- Auto-wakeup prescaler ----------------------------------------------- */

/** @defgroup pwr_awu_prescaler PWR Auto-wakeup Prescaler Dividers
@ingroup pwr_defines

@{*/
enum pwr_awu_prescaler {
	PWR_AWU_DIV1 = 0x0,		/**< LSI / 1 */
	PWR_AWU_DIV2 = 0x2,		/**< LSI / 2 */
	PWR_AWU_DIV4 = 0x3,		/**< LSI / 4 */
	PWR_AWU_DIV8 = 0x4,		/**< LSI / 8 */
	PWR_AWU_DIV16 = 0x5,		/**< LSI / 16 */
	PWR_AWU_DIV32 = 0x6,		/**< LSI / 32 */
	PWR_AWU_DIV64 = 0x7,		/**< LSI / 64 */
	PWR_AWU_DIV128 = 0x8,		/**< LSI / 128 */
	PWR_AWU_DIV256 = 0x9,		/**< LSI / 256 */
	PWR_AWU_DIV512 = 0xa,		/**< LSI / 512 */
	PWR_AWU_DIV1024 = 0xb,		/**< LSI / 1024 */
	PWR_AWU_DIV2048 = 0xc,		/**< LSI / 2048 */
	PWR_AWU_DIV4096 = 0xd,		/**< LSI / 4096 */
	PWR_AWU_DIV10240 = 0xe,		/**< LSI / 10240 */
	PWR_AWU_DIV61440 = 0xf,		/**< LSI / 61440 */
};
/**@}*/

/* --- Flags --------------------------------------------------------------- */

#define PWR_FLAG_PVDO			PWR_CSR_PVDO

BEGIN_DECLS

void pwr_enable_pvd(uint32_t pwr);
void pwr_disable_pvd(uint32_t pwr);
void pwr_set_pvd_level(uint32_t pwr, enum pwr_pvd_level level);
void pwr_enable_auto_wakeup(uint32_t pwr);
void pwr_disable_auto_wakeup(uint32_t pwr);
void pwr_set_awu_prescaler(uint32_t pwr, enum pwr_awu_prescaler prescaler);
void pwr_set_awu_window(uint32_t pwr, uint8_t window);
void pwr_enter_standby_mode(uint32_t pwr);
uint32_t pwr_get_flag(uint32_t pwr, uint32_t flag);

END_DECLS

#endif
/** @cond */
#else
#warning "pwr_common_v1.h should not be included explicitly, only via pwr.h"
#endif
/** @endcond */
/**@}*/
