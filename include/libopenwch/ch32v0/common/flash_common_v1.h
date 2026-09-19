/** @addtogroup flash_defines FLASH Defines

@brief <b>Defined Constants and Types for the CH32V00x FLASH</b>

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA FLASH.H
The order of header inclusion is important: flash.h includes the device
specific memorymap.h header before including this header file. */

/** @cond */
#if defined(LIBOPENWCH_FLASH_H) || defined(LIBOPENWCH_FLASH_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_FLASH_COMMON_V1_H
#define LIBOPENWCH_FLASH_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * The CH32V00x flash controller keeps the classic STM32-style register set,
 * with two erase sizes and a buffered ("fast") page program path:
 *
 *   - a 64-byte page (CTLR PAGE_ER / PAGE_PG) and a 1 KiB block (CTLR PER);
 *   - halfword programming through PG;
 *   - an option-byte block at OB_BASE, unlocked through OBKEYR;
 *   - a write buffer for 64-byte page programs, unlocked through MODEKEYR
 *     and driven by the BUF_LOAD/BUF_RST bits.
 *
 * The flash wait states (ACTLR) are shared with the RCC driver, which
 * reprograms the latency field on every clock change.  The accessor and the
 * latency constants below are therefore defined with the same values as
 * rcc.h's, guarded so that either header can be included first.
 */

/* --- Register accessors -------------------------------------------------- */

/*
 * Flash access control register (FLASH_ACTLR).  Also declared by rcc.h; the
 * definitions are identical, so both headers may be included in any order.
 */
#ifndef FLASH_ACTLR
#define FLASH_ACTLR MMIO32(FLASH_R_BASE + 0x00)
#endif
/* Latency field of FLASH_ACTLR (also in rcc.h). */
#ifndef FLASH_ACTLR_LATENCY_MASK
#define FLASH_ACTLR_LATENCY_MASK 0x3u
#define FLASH_ACTLR_LATENCY_0 0x0u
#define FLASH_ACTLR_LATENCY_1 0x1u
#define FLASH_ACTLR_LATENCY_2 0x2u
#endif

/* Flash key register (FLASH_KEYR) */
#define FLASH_KEYR(flash) MMIO32((flash) + 0x04)
/* Option byte key register (FLASH_OBKEYR) */
#define FLASH_OBKEYR(flash) MMIO32((flash) + 0x08)
/* Status register (FLASH_STATR) */
#define FLASH_STATR(flash) MMIO32((flash) + 0x0c)
/* Control register (FLASH_CTLR) */
#define FLASH_CTLR(flash) MMIO32((flash) + 0x10)
/* Address register (FLASH_ADDR) */
#define FLASH_ADDR(flash) MMIO32((flash) + 0x14)
/* Option byte register (FLASH_OBR) */
#define FLASH_OBR(flash) MMIO32((flash) + 0x1c)
/* Write protection register (FLASH_WPR) */
#define FLASH_WPR(flash) MMIO32((flash) + 0x20)
/* Mode key register, unlocks the fast program path (FLASH_MODEKEYR) */
#define FLASH_MODEKEYR(flash) MMIO32((flash) + 0x24)
/* Boot mode key register (FLASH_BOOT_MODEKEYR) */
#define FLASH_BOOT_MODEKEYR(flash) MMIO32((flash) + 0x28)

/* --- Option byte block (OB_BASE) ----------------------------------------- */

#define FLASH_OB_RDPR MMIO16(OB_BASE + 0x00)
#define FLASH_OB_USER MMIO16(OB_BASE + 0x02)
#define FLASH_OB_DATA0 MMIO16(OB_BASE + 0x04)
#define FLASH_OB_DATA1 MMIO16(OB_BASE + 0x06)
#define FLASH_OB_WRPR0 MMIO16(OB_BASE + 0x08)
#define FLASH_OB_WRPR1 MMIO16(OB_BASE + 0x0a)

/* --- Unlock keys --------------------------------------------------------- */

#define FLASH_KEYR_KEY1 ((uint32_t)0x45670123)
#define FLASH_KEYR_KEY2 ((uint32_t)0xcdef89ab)
#define FLASH_OBKEYR_KEY1 FLASH_KEYR_KEY1
#define FLASH_OBKEYR_KEY2 FLASH_KEYR_KEY2
#define FLASH_MODEKEYR_KEY1 FLASH_KEYR_KEY1
#define FLASH_MODEKEYR_KEY2 FLASH_KEYR_KEY2
#define FLASH_BOOT_MODEKEYR_KEY1 FLASH_KEYR_KEY1
#define FLASH_BOOT_MODEKEYR_KEY2 FLASH_KEYR_KEY2

/* --- Sizes --------------------------------------------------------------- */

/** A CH32V00x fast-program flash page is 64 bytes. */
#define FLASH_PAGE_SIZE 64u
/** The block-erase (CTLR PER) granularity is 1 KiB. */
#define FLASH_BLOCK_SIZE 1024u

/* --- FLASH_STATR bits ---------------------------------------------------- */

#define FLASH_STATR_BSY (1 << 0)      /**< busy */
#define FLASH_STATR_WRPRTERR (1 << 4) /**< write protection error */
#define FLASH_STATR_EOP (1 << 5)      /**< end of operation */
#define FLASH_STATR_MODE (1 << 14)    /**< fast mode status */
#define FLASH_STATR_LOCK (1 << 15)    /**< locked status */

/* --- FLASH_CTLR bits ----------------------------------------------------- */

#define FLASH_CTLR_PG (1 << 0)	      /**< halfword program */
#define FLASH_CTLR_PER (1 << 1)	      /**< 1 KiB block erase */
#define FLASH_CTLR_MER (1 << 2)	      /**< mass erase */
#define FLASH_CTLR_OPTPG (1 << 4)     /**< option byte program */
#define FLASH_CTLR_OPTER (1 << 5)     /**< option byte erase */
#define FLASH_CTLR_STRT (1 << 6)      /**< start operation */
#define FLASH_CTLR_LOCK (1 << 7)      /**< lock the controller */
#define FLASH_CTLR_OPTWRE (1 << 9)    /**< option byte write enable */
#define FLASH_CTLR_ERRIE (1 << 10)    /**< error interrupt enable */
#define FLASH_CTLR_EOPIE (1 << 12)    /**< end-of-operation IE */
#define FLASH_CTLR_FLOCK (1 << 15)    /**< fast-mode lock */
#define FLASH_CTLR_PAGE_PG (1 << 16)  /**< 64-byte page program */
#define FLASH_CTLR_PAGE_ER (1 << 17)  /**< 64-byte page erase */
#define FLASH_CTLR_BUF_LOAD (1 << 18) /**< load write buffer */
#define FLASH_CTLR_BUF_RST (1 << 19)  /**< reset write buffer */

/* --- FLASH_OBR bits ------------------------------------------------------ */

#define FLASH_OBR_OPTERR (1 << 0)      /**< option byte error */
#define FLASH_OBR_RDPRT (1 << 1)       /**< read protection */
#define FLASH_OBR_WDG_SW (1 << 2)      /**< watchdog software mode */
#define FLASH_OBR_nRST_STOP (1 << 3)   /**< reset on stop */
#define FLASH_OBR_nRST_STDBY (1 << 4)  /**< reset on standby */
#define FLASH_OBR_RST_MODE (0x3u << 5) /**< reset pin mode */
#define FLASH_OBR_STATR_MODE (1 << 7)
#define FLASH_OBR_USER (0xffu << 2) /**< user option field */
#define FLASH_OBR_FIX_11 (0x3u << 8)

/* --- Status -------------------------------------------------------------- */

/** @defgroup flash_status FLASH Operation Status
@ingroup flash_defines

@{*/
enum flash_status {
	FLASH_STATUS_COMPLETE = 0, /**< operation finished, no error */
	FLASH_STATUS_BUSY,	   /**< controller is busy */
	FLASH_STATUS_WRPRTERR,	   /**< write protection error */
};
/**@}*/

BEGIN_DECLS

/* --- Lock and unlock ----------------------------------------------------- */

void flash_unlock(uint32_t flash);
void flash_lock(uint32_t flash);
void flash_unlock_option_bytes(uint32_t flash);

/* --- Status -------------------------------------------------------------- */

enum flash_status flash_wait_for_last_operation(uint32_t flash);
uint32_t flash_get_status_flags(uint32_t flash);
void flash_clear_status_flags(uint32_t flash);

/* --- Erase and program --------------------------------------------------- */

enum flash_status flash_erase_page(uint32_t flash, uint32_t address);
enum flash_status flash_erase_all_pages(uint32_t flash);
enum flash_status
flash_program_word(uint32_t flash, uint32_t address, uint32_t data);
enum flash_status
flash_program_halfword(uint32_t flash, uint32_t address, uint16_t data);

/* --- Latency and option bytes -------------------------------------------- */

void flash_set_latency(uint32_t flash, uint32_t latency);
enum flash_status
flash_program_option_bytes(uint32_t flash, uint32_t address, uint16_t data);
uint32_t flash_get_option_bytes(uint32_t flash);
enum flash_status flash_enable_write_protection(uint32_t flash, uint32_t pages);

/* --- Fast (buffered) page program ---------------------------------------- */

void flash_unlock_fast(uint32_t flash);
void flash_lock_fast(uint32_t flash);
void flash_buf_reset(uint32_t flash);
void flash_buf_load(uint32_t flash, uint32_t address, uint32_t data);
void flash_erase_page_fast(uint32_t flash, uint32_t address);
void flash_program_page_fast(uint32_t flash, uint32_t address);

END_DECLS

#endif
/** @cond */
#else
#warning "flash_common_v1.h should not be included explicitly, only via flash.h"
#endif
/** @endcond */
/**@}*/
