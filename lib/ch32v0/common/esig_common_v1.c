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
 * @ingroup CH32V0
 *
 * @brief <b>Electronic signature for the CH32V00x</b>
 *
 * The ESIG block is factory-programmed read-only data, so every accessor is a
 * plain memory load.  CH32V003RM section 15.2 gives the addresses used below.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch32v0/esig.h>
#include <libopenwch/qingke/assert.h>

uint16_t esig_get_flash_size_kb(void) {
	return (uint16_t)(ESIG_FLACAP & 0xffffu);
}

uint32_t esig_get_unique_id_word(uint8_t index) {
	switch (index) {
	case 0u:
		return ESIG_UNIID1;
	case 1u:
		return ESIG_UNIID2;
	case 2u:
		return ESIG_UNIID3;
	default:
		openwch_assert_not_reached();
		return 0u;
	}
}

void esig_get_unique_id(uint8_t *buf) {
	uint32_t word = 0u;
	unsigned int i;

	openwch_assert(buf != NULL);

	for (i = 0; i < 12u; i++) {
		if ((i & 0x3u) == 0u) {
			word = esig_get_unique_id_word((uint8_t)(i >> 2));
		}
		buf[i] = (uint8_t)(word >> ((i & 0x3u) * 8u));
	}
}
/**@}*/
