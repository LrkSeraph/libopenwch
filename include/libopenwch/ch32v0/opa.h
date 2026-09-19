/** @defgroup opa_defines OPA Defines

@brief <b>Operational Amplifier for the CH32V00x</b>

@ingroup CH32V0_defines

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

/**@{*/

#ifndef LIBOPENWCH_OPA_H
#define LIBOPENWCH_OPA_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/ch32v0/memorymap.h>

/*
 * The CH32V00x has one operational amplifier.  It has no register block of its
 * own: the enable and input-selection bits live in the EXTEN "enhanced
 * function" register at 0x40023800, which is also where the LDO trim and the
 * lockup-reset bits are.
 */
#define EXTEN				EXTEN_BASE
#define EXTEN_CTR(exten)		MMIO32((exten) + 0x00)

/* EXTEN_CTR bits */
#define EXTEN_CTR_LOCKUP_EN		(1 << 6)	/**< lockup reset enable */
#define EXTEN_CTR_LOCKUP_RSTF		(1 << 7)	/**< lockup reset flag */
#define EXTEN_CTR_LDO_TRIM		(1 << 10)	/**< LDO trim */
#define EXTEN_CTR_OPA_EN		(1 << 16)	/**< op-amp enable */
#define EXTEN_CTR_OPA_NSEL		(1 << 17)	/**< negative input select */
#define EXTEN_CTR_OPA_PSEL		(1 << 18)	/**< positive input select */

/* Only the OPA control bits are covered by opa_reset(). */
#define EXTEN_CTR_OPA_MASK \
	(EXTEN_CTR_OPA_EN | EXTEN_CTR_OPA_NSEL | EXTEN_CTR_OPA_PSEL)

/** Which pin drives the op-amp's positive input. */
typedef enum {
	OPA_PSEL_CHP0 = 0,	/**< positive input on the default pin */
	OPA_PSEL_CHP1 = 1,	/**< positive input on the alternate pin */
} opa_psel_t;

/** Which pin drives the op-amp's negative input. */
typedef enum {
	OPA_NSEL_CHN0 = 0,	/**< negative input on the default pin */
	OPA_NSEL_CHN1 = 1,	/**< negative input on the alternate pin */
} opa_nsel_t;

#include <libopenwch/ch32v0/common/opa_common_v1.h>

#endif
/**@}*/
