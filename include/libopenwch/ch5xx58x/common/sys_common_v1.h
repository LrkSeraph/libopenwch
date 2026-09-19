/** @addtogroup sys_defines SYS Defines

@brief <b>Defined Constants and Types for the CH58x system control block</b>

@ingroup CH5XX58X_defines

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA SYS.H */

/** @cond */
#if defined(LIBOPENWCH_SYS_H) || defined(LIBOPENWCH_SYS_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_SYS_COMMON_V1_H
#define LIBOPENWCH_SYS_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * The SYS block is the chip-wide control block at SYS_BASE: chip identity,
 * reset cause and the value kept across a reset.  Unlike the peripheral
 * drivers the SYS block is a single instance with no base-address parameter.
 *
 * Register offsets
 * ----------------
 * These byte registers sit next to the safe-access signature at
 * SYS_BASE + 0x40:
 *
 *	0x40  R8_SAFE_ACCESS_SIG
 *	0x41  R8_CHIP_ID
 *	0x42  R8_SAFE_ACCESS_ID
 *	0x43  R8_WDOG_COUNT
 *	0x44  R8_RESET_STATUS / R8_GLOB_ROM_CFG
 *	0x45  R8_GLOB_CFG_INFO
 *	0x46  R8_RST_WDOG_CTRL
 *	0x47  R8_GLOB_RESET_KEEP
 *
 * NOTE: the R8_CHIP_ID / R8_GLOB_RESET_KEEP / R8_RST_WDOG_CTRL offsets in
 * <libopenwch/ch5xx58x/memorymap.h> do not agree with the WCH CH583 SFR header
 * (they put the chip id at 0x46); the correct offsets are the ones above, and
 * the accessors below spell them out so that this driver never reads the
 * watchdog control register when it means to read the chip id.
 *
 * R8_RST_WDOG_CTRL is RWA, but sys_reset() uses the PFIC software reset
 * instead, so no safe-access window is needed here.  R8_GLOB_RESET_KEEP is a
 * plain read/write byte.
 */

/* --- Register accessors -------------------------------------------------- */

#define SYS_SAFE_ACCESS_SIG		MMIO8(SYS_BASE + 0x40)
#define SYS_CHIP_ID			MMIO8(SYS_BASE + 0x41)
#define SYS_SAFE_ACCESS_ID		MMIO8(SYS_BASE + 0x42)
#define SYS_WDOG_COUNT			MMIO8(SYS_BASE + 0x43)
#define SYS_RESET_STATUS		MMIO8(SYS_BASE + 0x44)
#define SYS_GLOB_CFG_INFO		MMIO8(SYS_BASE + 0x45)
#define SYS_RST_WDOG_CTRL		MMIO8(SYS_BASE + 0x46)
#define SYS_GLOB_RESET_KEEP		MMIO8(SYS_BASE + 0x47)

/* --- R8_RESET_STATUS bits ------------------------------------------------ */

/** @defgroup sys_reset_bits SYS reset status bits
@ingroup sys_defines
@{*/
#define RB_RESET_FLAG			0x07	/**< recent reset cause */
#define RST_FLAG_SW			0x00	/**< software reset */
#define RST_FLAG_RPOR			0x01	/**< real power-on reset */
#define RST_FLAG_WTR			0x02	/**< watchdog timeout reset */
#define RST_FLAG_MR			0x03	/**< external manual reset */
#define RST_FLAG_GPWSM			0x05	/**< wake from shutdown */
/**@}*/

/* --- R8_RST_WDOG_CTRL bits ----------------------------------------------- */

/** @defgroup sys_wdog_bits SYS reset / watchdog control bits
@ingroup sys_defines
@{*/
#define RB_SOFTWARE_RESET		0x01	/**< self-clearing global software reset */
#define RB_WDOG_RST_EN			0x02	/**< reset on watchdog overflow */
#define RB_WDOG_INT_EN			0x04	/**< watchdog overflow interrupt enable */
#define RB_WDOG_INT_FLAG		0x10	/**< watchdog overflow flag, write 1 to clear */
/**@}*/

/* --- R8_GLOB_CFG_INFO bits ----------------------------------------------- */

/** @defgroup sys_cfg_info_bits SYS global configuration information bits

Read-only status latched at reset.
@{*/
#define RB_CFG_ROM_READ			0x01	/**< flash readable by an external programmer */
#define RB_CFG_RESET_EN			0x04	/**< manual reset input enabled */
#define RB_CFG_BOOT_EN			0x08	/**< boot loader enabled */
#define RB_CFG_DEBUG_EN			0x10	/**< debug interface enabled */
#define RB_BOOT_LOADER			0x20	/**< running the boot loader */
/**@}*/

/* --- Recent reset cause -------------------------------------------------- */

/** Reason for the most recent reset, from R8_RESET_STATUS.RB_RESET_FLAG. */
typedef enum {
	SYS_RESET_SW = RST_FLAG_SW,		/**< software reset */
	SYS_RESET_RPOR = RST_FLAG_RPOR,		/**< real power-on reset */
	SYS_RESET_WDOG = RST_FLAG_WTR,		/**< watchdog timeout */
	SYS_RESET_MANUAL = RST_FLAG_MR,		/**< external manual reset */
	SYS_RESET_LOW_POWER_0 = 0x04,		/**< wake from low power, cause 0 */
	SYS_RESET_SHUTDOWN_WAKE = RST_FLAG_GPWSM,	/**< wake from shutdown */
	SYS_RESET_LOW_POWER_1 = 0x06,		/**< wake from low power, cause 1 */
	SYS_RESET_LOW_POWER_2 = 0x07,		/**< wake from low power, cause 2 */
} sys_reset_status_t;

BEGIN_DECLS

/** Reset the chip through the PFIC software-reset bit; never returns. */
void sys_reset(void);

/** Current SysTick counter value (low 32 bits). */
uint32_t sys_get_systick_count(void);

/** Chip identification byte, always an ID_CH58* value. */
uint8_t sys_get_chip_id(void);

/** Cause of the most recent reset. */
sys_reset_status_t sys_get_reset_status(void);

/** Value of the byte kept across a global reset. */
uint8_t sys_reset_keep(void);
/** Store a byte that survives a global reset. */
void sys_set_reset_keep(uint8_t value);

/**
 * Disable every PFIC interrupt and return the previous enable state in
 * *saved, for sys_recover_irq().
 */
void sys_disable_all_irq(uint32_t *saved);
/** Restore the PFIC interrupt enables captured by sys_disable_all_irq(). */
void sys_recover_irq(uint32_t saved);

END_DECLS

#endif
/** @cond */
#else
#warning "sys_common_v1.h should not be included explicitly, only via sys.h"
#endif
/** @endcond */
/**@}*/
