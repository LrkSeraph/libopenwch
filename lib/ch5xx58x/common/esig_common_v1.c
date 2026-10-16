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

/** @addtogroup esig_file ESIG
 *
 * @ingroup CH5XX58X
 *
 * @brief <b>Electronic signature and factory information for the CH58x</b>
 *
 * The unique-ID bytes are ordinary InfoFlash loads, not ROM ISP commands, so
 * these accessors remain usable in a freestanding application.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch5xx58x/esig.h>
#include <libopenwch/ch5xx58x/flash.h>
#include <libopenwch/qingke/assert.h>

uint8_t esig_get_chip_id(void) {
	return ESIG_CHIP_ID;
}

void esig_get_unique_id(uint8_t *buf) {
	openwch_assert(buf != NULL);

	flash_get_unique_id(buf);
}

void esig_get_mac_address(uint8_t *buf) {
	uint8_t id[8];
	unsigned int i;

	openwch_assert(buf != NULL);

	flash_get_unique_id(id);
	for (i = 0u; i < 6u; i++) {
		buf[i] = id[i];
	}
}

uint32_t esig_get_temp_cal_25c(void) {
	return MMIO32(ESIG_TEMP_CAL_ADDR);
}
/**@}*/
