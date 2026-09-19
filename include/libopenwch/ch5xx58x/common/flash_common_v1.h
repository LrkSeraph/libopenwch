/** @addtogroup flash_defines FLASH Defines

@brief <b>Defined Constants and Types for the CH58x flash and factory info</b>

@ingroup CH5XX58X_defines

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA FLASH.H */

/** @cond */
#if defined(LIBOPENWCH_FLASH_H) || defined(LIBOPENWCH_FLASH_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_FLASH_COMMON_V1_H
#define LIBOPENWCH_FLASH_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * The CH58x flash controller is not a normal peripheral.
 *
 * There is no self-contained program/erase register block the way the
 * CH32V00x and the STM32 parts have.  The chip exposes:
 *
 *   - the flash array itself, memory-mapped for reads at CODE_BASE
 *     (0x00000000).  Code and constant data can simply be loaded from it, so
 *     reading never needs a driver;
 *   - a small ROM control window at ROM_CTRL_BASE (SYS_BASE + 0x800) holding
 *     R8_FLASH_DATA / R8_FLASH_CTRL / R8_FLASH_CFG.  Of these only
 *     R8_FLASH_CFG is documented: it selects the flash access timing and is
 *     RWA, i.e. writes need a safe-access window;
 *   - factory information at ROM_CFG_BASE (0x0007F000) -- the 6-byte MAC /
 *     unique ID at ROM_CFG_MAC_ADDR (0x0007F018) and the 25 C temperature
 *     calibration word at ROM_CFG_TMP_25C (0x0007F014).  These are
 *     memory-mapped and readable directly;
 *   - a 32 KiB data-flash (EEPROM) region at DATA_FLASH_ADDR (0x00070000).
 *
 * ERASE AND PROGRAM ARE NOT IMPLEMENTED HERE, DELIBERATELY.
 *
 * On this family erase and program are performed by the ROM-resident ISP
 * routines, reached through FLASH_EEPROM_CMD() in libISP583.a, which WCH ships
 * as a binary blob.  libopenwch does not link that blob (it is not free
 * software), so it cannot and does not implement erase or program.  The
 * functions at the bottom of this header are honest stubs that always return
 * FLASH_STATUS_UNSUPPORTED.
 *
 * To actually write flash you have three options:
 *
 *   1. link WCH's libISP583.a and call FLASH_EEPROM_CMD() yourself, from RAM
 *      or from .highcode (the ISP entry point cannot run from the flash it is
 *      erasing);
 *   2. use an external programmer such as minichlink over the two-wire debug
 *      interface;
 *   3. provide your own ISP implementation ported to this library.
 *
 * The read/identity/latency helpers below are real: they only need plain
 * memory loads and one RWA register write.
 *
 * NOTE ON BASE ADDRESSES: unlike the other drivers in this library the flash
 * functions do not take a peripheral base address.  The CH58x flash is a
 * singleton whose registers are split between the SYS window (R8_FLASH_CFG,
 * R8_CHIP_ID) and the memory-mapped array and factory-information region, so
 * there is no single base to pass.  Every function takes the address it acts
 * on instead.
 */

/* --- Register accessors -------------------------------------------------- */

/*
 * Flash access configuration (R8_FLASH_CFG, 0x40001807), RWA.  clk.h owns the
 * same accessor because it reprograms the timing on every clock change; the
 * definition is identical on purpose so the two headers can be included in
 * any order.
 */
#ifndef FLASH_CFG
#define FLASH_CFG MMIO8(R8_FLASH_CFG)
#endif
/* Flash data buffer (R8_FLASH_DATA, 0x40001804) */
#ifndef FLASH_DATA
#define FLASH_DATA MMIO8(ROM_CTRL_BASE + 0x04)
#endif
/* Flash access control (R8_FLASH_CTRL, 0x40001806) */
#ifndef FLASH_CTRL
#define FLASH_CTRL MMIO8(ROM_CTRL_BASE + 0x06)
#endif

/*
 * Chip ID (R8_CHIP_ID, read-only, 0x40001041).
 *
 * The offset is spelled out here rather than taken from memorymap.h: that
 * header's R8_CHIP_ID macro points at SYS_BASE + 0x46, which the CH583 SFR
 * header defines as R8_RST_WDOG_CTRL.  The real R8_CHIP_ID is one byte above
 * R8_SAFE_ACCESS_SIG, at 0x40001041.
 */
#define FLASH_CHIP_ID_ADDR (SYS_BASE + 0x41)
#define FLASH_CHIP_ID MMIO8(FLASH_CHIP_ID_ADDR)

/* Factory information, memory-mapped for reads. */
#define FLASH_UNIQUE_ID_ADDR ROM_CFG_MAC_ADDR
#define FLASH_TEMP_CAL_ADDR ROM_CFG_TMP_25C

/* --- Sizes --------------------------------------------------------------- */

/** Data-flash (EEPROM) block size, in bytes. */
#define FLASH_EEPROM_BLOCK_SIZE 256u
/** Flash-ROM erase block size, in bytes. */
#define FLASH_ROM_BLOCK_SIZE 4096u
/** Flash-ROM program granularity, in bytes. */
#define FLASH_ROM_MIN_WRITE_SIZE 4u

/* --- Flash timing -------------------------------------------------------- */

/** @defgroup flash_latency FLASH Access Timing
@ingroup flash_defines

R8_FLASH_CFG is an opaque timing/divider byte: the CH583 SFR header documents
no bit fields for it, only the literal values WCH's own clock code writes.
These are those literals.

@{*/
typedef enum {
	FLASH_LATENCY_PLL_FAST = 0x02, /**< PLL clock path, 80 MHz */
	FLASH_LATENCY_PLL = 0x52,      /**< PLL clock path, below 80 MHz */
	FLASH_LATENCY_HSE = 0x51, /**< XT32M divider path, 6.4 MHz and up */
	FLASH_LATENCY_HSE_SLOW =
	    0x57, /**< XT32M divider path, 4 MHz and down */
} flash_latency_t;
/**@}*/

/* --- Status -------------------------------------------------------------- */

/** @defgroup flash_status FLASH Operation Status
@ingroup flash_defines

@{*/
enum flash_status {
	FLASH_STATUS_COMPLETE = 0, /**< operation finished, no error */
	FLASH_STATUS_UNSUPPORTED,  /**< not implemented without libISP583.a */
	FLASH_STATUS_BAD_ARGUMENT, /**< address or length rejected */
	FLASH_STATUS_VERIFY_ERROR, /**< write read back differently */
};
/**@}*/

BEGIN_DECLS

/* --- Factory information ------------------------------------------------- */

/**
 * Read the 6-byte factory MAC / unique ID into \a buf.
 *
 * \a buf must hold at least 8 bytes: bytes 0..5 are the MAC and bytes 6..7 are
 * written as zero, matching WCH's GET_UNIQUE_ID().
 */
void flash_get_unique_id(uint8_t *buf);

/** Read the one-byte chip ID register (R8_CHIP_ID). */
uint8_t flash_get_chip_id(void);

/* --- Reading ------------------------------------------------------------- */

/** Copy \a len bytes from the memory-mapped flash to \a buf, byte by byte. */
void flash_read(uint32_t address, void *buf, uint32_t len);

/**
 * Word-oriented read, matching WCH's FLASH_ROM_READ().
 *
 * Both \a address and \a len must be multiples of 4; the copy is \a len / 4
 * 32-bit loads.  Unlike flash_read() this has no unaligned fallback.
 */
void flash_rom_read(uint32_t address, void *buf, uint32_t len);

/* --- Timing -------------------------------------------------------------- */

/**
 * Write the flash access timing byte (R8_FLASH_CFG) through a safe-access
 * window.  clk_set_sys_clock() already does this on every clock change; call
 * this directly only when changing the timing yourself.
 */
void flash_set_latency(flash_latency_t latency);

/* --- Erase and program: not available in libopenwch ---------------------- */

/**
 * Erase the 4 KiB flash-ROM block containing \a address.
 *
 * NOT IMPLEMENTED: always returns FLASH_STATUS_UNSUPPORTED.  Erase needs the
 * ROM ISP entry point in libISP583.a, which libopenwch does not link.  See the
 * note at the top of this header.
 */
enum flash_status flash_erase_page(uint32_t address);

/**
 * Program \a len bytes from \a buf at \a address.
 *
 * NOT IMPLEMENTED: always returns FLASH_STATUS_UNSUPPORTED.  Program needs the
 * ROM ISP entry point in libISP583.a, which libopenwch does not link.  See the
 * note at the top of this header.
 */
enum flash_status
flash_program(uint32_t address, const void *buf, uint32_t len);

END_DECLS

#endif
/** @cond */
#else
#warning "flash_common_v1.h should not be included explicitly, only via flash.h"
#endif
/** @endcond */
/**@}*/
