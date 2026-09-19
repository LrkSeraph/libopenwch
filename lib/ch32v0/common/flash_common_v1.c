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
 * @ingroup CH32V0
 *
 * @brief <b>Internal Flash Memory Controller for the CH32V00x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * The controller follows the classic STM32 sequence, with the WCH additions:
 *
 *   - the key register is written twice with FLASH_KEYR_KEY1/KEY2 to unlock;
 *   - the option-byte block is unlocked separately through FLASH_OBKEYR;
 *   - a 64-byte page program uses the write buffer: unlock with
 *     flash_unlock_fast(), clear the buffer with flash_buf_reset(), load the
 *     sixteen words with flash_buf_load(), then start the program with
 *     flash_program_page_fast().
 *
 * Every operation polls STATR.BSY through flash_wait_for_last_operation() and
 * reports STATR.WRPRTERR, so a protected or failed write is visible to the
 * caller instead of being silently dropped.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch32v0/flash.h>
#include <libopenwch/qingke/assert.h>

enum flash_status flash_wait_for_last_operation(uint32_t flash) {
	while (FLASH_STATR(flash) & FLASH_STATR_BSY) {
		;
	}

	if (FLASH_STATR(flash) & FLASH_STATR_WRPRTERR) {
		return FLASH_STATUS_WRPRTERR;
	}

	return FLASH_STATUS_COMPLETE;
}

uint32_t flash_get_status_flags(uint32_t flash) {
	return FLASH_STATR(flash);
}

void flash_clear_status_flags(uint32_t flash) {
	/* EOP and WRPRTERR are rc_w1; write a one to clear. */
	FLASH_STATR(flash) = FLASH_STATR_EOP | FLASH_STATR_WRPRTERR;
}

void flash_unlock(uint32_t flash) {
	FLASH_KEYR(flash) = FLASH_KEYR_KEY1;
	FLASH_KEYR(flash) = FLASH_KEYR_KEY2;
}

void flash_lock(uint32_t flash) {
	FLASH_CTLR(flash) |= FLASH_CTLR_LOCK;
}

void flash_unlock_option_bytes(uint32_t flash) {
	FLASH_OBKEYR(flash) = FLASH_OBKEYR_KEY1;
	FLASH_OBKEYR(flash) = FLASH_OBKEYR_KEY2;
}

enum flash_status flash_erase_page(uint32_t flash, uint32_t address) {
	enum flash_status status;

	openwch_assert((address & (FLASH_PAGE_SIZE - 1u)) == 0);

	status = flash_wait_for_last_operation(flash);
	if (status != FLASH_STATUS_COMPLETE) {
		return status;
	}

	FLASH_CTLR(flash) |= FLASH_CTLR_PAGE_ER;
	FLASH_ADDR(flash) = address;
	FLASH_CTLR(flash) |= FLASH_CTLR_STRT;

	status = flash_wait_for_last_operation(flash);

	FLASH_CTLR(flash) &= ~FLASH_CTLR_PAGE_ER;

	return status;
}

enum flash_status flash_erase_all_pages(uint32_t flash) {
	enum flash_status status;

	status = flash_wait_for_last_operation(flash);
	if (status != FLASH_STATUS_COMPLETE) {
		return status;
	}

	FLASH_CTLR(flash) |= FLASH_CTLR_MER;
	FLASH_CTLR(flash) |= FLASH_CTLR_STRT;

	status = flash_wait_for_last_operation(flash);

	FLASH_CTLR(flash) &= ~FLASH_CTLR_MER;

	return status;
}

enum flash_status
flash_program_halfword(uint32_t flash, uint32_t address, uint16_t data) {
	enum flash_status status;

	openwch_assert((address & 0x1u) == 0);

	status = flash_wait_for_last_operation(flash);
	if (status != FLASH_STATUS_COMPLETE) {
		return status;
	}

	FLASH_CTLR(flash) |= FLASH_CTLR_PG;
	MMIO16(address) = data;

	status = flash_wait_for_last_operation(flash);

	FLASH_CTLR(flash) &= ~FLASH_CTLR_PG;

	return status;
}

enum flash_status
flash_program_word(uint32_t flash, uint32_t address, uint32_t data) {
	enum flash_status status;

	openwch_assert((address & 0x1u) == 0);

	status = flash_wait_for_last_operation(flash);
	if (status != FLASH_STATUS_COMPLETE) {
		return status;
	}

	FLASH_CTLR(flash) |= FLASH_CTLR_PG;

	MMIO16(address) = (uint16_t)data;
	status = flash_wait_for_last_operation(flash);

	if (status == FLASH_STATUS_COMPLETE) {
		MMIO16(address + 2u) = (uint16_t)(data >> 16);
		status = flash_wait_for_last_operation(flash);
	}

	FLASH_CTLR(flash) &= ~FLASH_CTLR_PG;

	return status;
}

void flash_set_latency(uint32_t flash, uint32_t latency) {
	/*
	 * The latency field lives in the flash access-control register.  There
	 * is only one flash controller, so the base argument is accepted for
	 * API symmetry and checked rather than used to index anything.
	 */
	openwch_assert(flash == FLASH_R_BASE);
	openwch_assert((latency & ~FLASH_ACTLR_LATENCY_MASK) == 0);
	(void)flash;

	FLASH_ACTLR = (FLASH_ACTLR & ~FLASH_ACTLR_LATENCY_MASK) |
		      (latency & FLASH_ACTLR_LATENCY_MASK);
}

enum flash_status
flash_program_option_bytes(uint32_t flash, uint32_t address, uint16_t data) {
	enum flash_status status;

	openwch_assert((address & 0x1u) == 0);

	status = flash_wait_for_last_operation(flash);
	if (status != FLASH_STATUS_COMPLETE) {
		return status;
	}

	flash_unlock_option_bytes(flash);

	FLASH_CTLR(flash) |= FLASH_CTLR_OPTPG;
	MMIO16(address) = data;

	status = flash_wait_for_last_operation(flash);

	FLASH_CTLR(flash) &= ~FLASH_CTLR_OPTPG;

	return status;
}

uint32_t flash_get_option_bytes(uint32_t flash) {
	return FLASH_OBR(flash);
}

enum flash_status flash_enable_write_protection(uint32_t flash,
						uint32_t pages) {
	enum flash_status status;

	status = flash_wait_for_last_operation(flash);
	if (status != FLASH_STATUS_COMPLETE) {
		return status;
	}

	/*
	 * The option bytes are complemented: a zero bit in WRPRx protects the
	 * matching page, so invert the caller's "pages to protect" mask.
	 */
	pages = ~pages;

	flash_unlock_option_bytes(flash);
	FLASH_CTLR(flash) |= FLASH_CTLR_OPTPG;

	if ((pages & 0xffu) != 0xffu) {
		FLASH_OB_WRPR0 = (uint16_t)(pages & 0xffu);
		status = flash_wait_for_last_operation(flash);
	}

	if ((status == FLASH_STATUS_COMPLETE) &&
	    (((pages >> 8) & 0xffu) != 0xffu)) {
		FLASH_OB_WRPR1 = (uint16_t)((pages >> 8) & 0xffu);
		status = flash_wait_for_last_operation(flash);
	}

	FLASH_CTLR(flash) &= ~FLASH_CTLR_OPTPG;

	return status;
}

/* --- Fast (buffered) page program ---------------------------------------- */

void flash_unlock_fast(uint32_t flash) {
	flash_unlock(flash);

	FLASH_MODEKEYR(flash) = FLASH_MODEKEYR_KEY1;
	FLASH_MODEKEYR(flash) = FLASH_MODEKEYR_KEY2;
}

void flash_lock_fast(uint32_t flash) {
	FLASH_CTLR(flash) |= FLASH_CTLR_FLOCK;
}

void flash_buf_reset(uint32_t flash) {
	FLASH_CTLR(flash) |= FLASH_CTLR_PAGE_PG;
	FLASH_CTLR(flash) |= FLASH_CTLR_BUF_RST;

	while (FLASH_STATR(flash) & FLASH_STATR_BSY) {
		;
	}

	FLASH_CTLR(flash) &= ~FLASH_CTLR_PAGE_PG;
}

void flash_buf_load(uint32_t flash, uint32_t address, uint32_t data) {
	openwch_assert((address & 0x3u) == 0);

	FLASH_CTLR(flash) |= FLASH_CTLR_PAGE_PG;
	MMIO32(address) = data;
	FLASH_CTLR(flash) |= FLASH_CTLR_BUF_LOAD;

	while (FLASH_STATR(flash) & FLASH_STATR_BSY) {
		;
	}

	FLASH_CTLR(flash) &= ~FLASH_CTLR_PAGE_PG;
}

void flash_erase_page_fast(uint32_t flash, uint32_t address) {
	openwch_assert((address & (FLASH_PAGE_SIZE - 1u)) == 0);

	FLASH_CTLR(flash) |= FLASH_CTLR_PAGE_ER;
	FLASH_ADDR(flash) = address;
	FLASH_CTLR(flash) |= FLASH_CTLR_STRT;

	while (FLASH_STATR(flash) & FLASH_STATR_BSY) {
		;
	}

	FLASH_CTLR(flash) &= ~FLASH_CTLR_PAGE_ER;
}

void flash_program_page_fast(uint32_t flash, uint32_t address) {
	openwch_assert((address & (FLASH_PAGE_SIZE - 1u)) == 0);

	FLASH_CTLR(flash) |= FLASH_CTLR_PAGE_PG;
	FLASH_ADDR(flash) = address;
	FLASH_CTLR(flash) |= FLASH_CTLR_STRT;

	while (FLASH_STATR(flash) & FLASH_STATR_BSY) {
		;
	}

	FLASH_CTLR(flash) &= ~FLASH_CTLR_PAGE_PG;
}
/**@}*/
