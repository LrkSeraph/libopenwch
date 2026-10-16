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

/** @addtogroup exten_file EXTEN
 *
 * @ingroup CH32V0
 *
 * @brief <b>Extended configuration unit for the CH32V00x</b>
 *
 * CH32V003RM section 17.2.1 defines the register fields implemented here.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch32v0/exten.h>
#include <libopenwch/qingke/assert.h>

void exten_enable_lockup_reset(uint32_t exten) {
	EXTEN_CTR(exten) |= EXTEN_CTR_LOCKUP_EN;
}

void exten_disable_lockup_reset(uint32_t exten) {
	EXTEN_CTR(exten) &= ~(uint32_t)EXTEN_CTR_LOCKUP_EN;
}

uint32_t exten_get_lockup_reset_flag(uint32_t exten) {
	return EXTEN_CTR(exten) & EXTEN_CTR_LOCKUP_RSTF;
}

void exten_clear_lockup_reset_flag(uint32_t exten) {
	EXTEN_CTR(exten) |= EXTEN_CTR_LOCKUP_RSTF;
}

void exten_set_ldo_mode(uint32_t exten, exten_ldo_mode_t mode) {
	uint32_t reg;

	openwch_assert(mode <= EXTEN_LDO_MODE_BOOST);

	reg = EXTEN_CTR(exten) & ~(uint32_t)EXTEN_CTR_LDO_TRIM;
	if (mode == EXTEN_LDO_MODE_BOOST) {
		reg |= EXTEN_CTR_LDO_TRIM;
	}
	EXTEN_CTR(exten) = reg;
}

exten_ldo_mode_t exten_get_ldo_mode(uint32_t exten) {
	if ((EXTEN_CTR(exten) & EXTEN_CTR_LDO_TRIM) != 0u) {
		return EXTEN_LDO_MODE_BOOST;
	}
	return EXTEN_LDO_MODE_NORMAL;
}
/**@}*/
