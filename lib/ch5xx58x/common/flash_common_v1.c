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

/** @addtogroup flash_file FLASH
 *
 * @ingroup CH5XX58X
 *
 * @brief <b>Internal flash and factory information for the CH58x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * This file implements everything that does not need WCH's closed-source ROM
 * ISP library: the factory MAC / unique ID, the chip ID, plain array reads and
 * the RWA flash-timing byte.  Erase and program are deliberate stubs returning
 * FLASH_STATUS_UNSUPPORTED; see common/flash_common_v1.h for what would be
 * required to fill that gap.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch5xx58x/flash.h>
#include <libopenwch/ch5xx58x/rwa.h>
#include <libopenwch/qingke/assert.h>

/* --- Factory information ------------------------------------------------- */

void flash_get_unique_id(uint8_t *buf)
{
	uint32_t lo;
	uint32_t hi;

	openwch_assert(buf != NULL);

	/*
	 * ROM_CFG_MAC_ADDR is plain memory-mapped factory data, so this is a
	 * load, not an ISP command.  The MAC is six bytes; the last two bytes
	 * of the eight-byte ID are reserved and are cleared to match WCH's
	 * GET_UNIQUE_ID().
	 */
	lo = MMIO32(FLASH_UNIQUE_ID_ADDR);
	hi = MMIO32(FLASH_UNIQUE_ID_ADDR + 4u);

	buf[0] = (uint8_t)lo;
	buf[1] = (uint8_t)(lo >> 8);
	buf[2] = (uint8_t)(lo >> 16);
	buf[3] = (uint8_t)(lo >> 24);
	buf[4] = (uint8_t)hi;
	buf[5] = (uint8_t)(hi >> 8);
	buf[6] = 0;
	buf[7] = 0;
}

uint8_t flash_get_chip_id(void)
{
	return FLASH_CHIP_ID;
}

/* --- Reading ------------------------------------------------------------- */

void flash_read(uint32_t address, void *buf, uint32_t len)
{
	uint8_t *dst = (uint8_t *)buf;
	uint32_t i;

	openwch_assert(buf != NULL);

	for (i = 0; i < len; i++) {
		dst[i] = MMIO8(address + i);
	}
}

void flash_rom_read(uint32_t address, void *buf, uint32_t len)
{
	uint32_t *dst = (uint32_t *)buf;
	const volatile uint32_t *src =
		(const volatile uint32_t *)address;
	uint32_t words = len >> 2;
	uint32_t i;

	openwch_assert(buf != NULL);
	openwch_assert((address & 0x3u) == 0);
	openwch_assert((len & 0x3u) == 0);

	/* WCH's FLASH_ROM_READ() is exactly this word loop. */
	for (i = 0; i < words; i++) {
		dst[i] = src[i];
	}
}

/* --- Timing -------------------------------------------------------------- */

void flash_set_latency(flash_latency_t latency)
{
	openwch_assert((latency == FLASH_LATENCY_PLL_FAST) ||
		       (latency == FLASH_LATENCY_PLL) ||
		       (latency == FLASH_LATENCY_HSE) ||
		       (latency == FLASH_LATENCY_HSE_SLOW));

	RWA_WRITE8(FLASH_CFG, (uint8_t)latency);
}

/* --- Erase and program: not available in libopenwch ---------------------- */

enum flash_status flash_erase_page(uint32_t address)
{
	/*
	 * R8_FLASH_CFG selects the timing but there is no documented
	 * erase/program register: on this family the operation is performed by
	 * FLASH_EEPROM_CMD() in WCH's libISP583.a.  Rather than write a
	 * register sequence that cannot work, report the gap.
	 */
	(void)address;

	return FLASH_STATUS_UNSUPPORTED;
}

enum flash_status flash_program(uint32_t address, const void *buf,
				uint32_t len)
{
	(void)address;
	(void)buf;
	(void)len;

	return FLASH_STATUS_UNSUPPORTED;
}
/**@}*/
