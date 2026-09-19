/** @defgroup clk_defines CLK Defines

@brief <b>Clock control for the CH58x</b>

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

/**@{*/

#ifndef LIBOPENWCH_CLK_H
#define LIBOPENWCH_CLK_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/ch5xx58x/memorymap.h>

/*
 * The CH58x clock tree
 * --------------------
 *
 *	XT32M   32 MHz external crystal   <- the ONLY high-speed source
 *	XT32K   32.768 kHz crystal
 *	LSI     internal ~32 kHz RC
 *	PLL     fixed 480 MHz, fed from XT32M
 *
 * The system clock is XT32M, PLL/divider, or one of the 32 kHz sources.
 * There is no HSI on the CH582/CH583 (the CH584/CH585 add one), so the 32 MHz
 * crystal is mandatory and USB -- which needs 48 MHz -- cannot work without
 * it.
 *
 * Clock selection is expressed by a single byte written to R16_CLK_SYS_CFG:
 *
 *	bit 5   select the XT32M divider path
 *	bit 6   select the PLL divider path
 *	bits 4:0  divider, applied to 32 MHz or to 480 MHz
 *	bits 7:6 = 0b11 selects a 32 kHz source instead
 */

/* --- Register accessors -------------------------------------------------- */
#define CLK_SYS_CFG MMIO32(R32_CLK_SYS_CFG)
#define HFCK_PWR_CTRL MMIO8(R8_HFCK_PWR_CTRL)
#define PLL_CONFIG MMIO8(R8_PLL_CONFIG)
#define XT32M_TUNE MMIO8(R8_XT32M_TUNE)
#define OSC32K_CTRL MMIO32(R32_OSC32K_CTRL)
#define FLASH_CFG MMIO8(R8_FLASH_CFG)

/* --- HFCK_PWR_CTRL bits -------------------------------------------------- */
#define CLK_XT32K_PON (1 << 0) /**< 32.768 kHz crystal */
#define CLK_LSI_PON (1 << 1)   /**< internal 32 kHz RC */
#define CLK_XT32M_PON (1 << 2) /**< 32 MHz crystal */
#define CLK_PLL_PON (1 << 4)   /**< 480 MHz PLL */

/* --- CLK_SYS_CFG fields -------------------------------------------------- */
#define CLK_SYS_CFG_MOD_MASK (0xc0u) /**< bits 7:6 */
#define CLK_SYS_CFG_MOD_HSE (0x00u)  /**< 0b00 XT32M path */
#define CLK_SYS_CFG_MOD_PLL (0x40u)  /**< 0b01 PLL path */
#define CLK_SYS_CFG_MOD_32K (0xc0u)  /**< 0b11 32 kHz path */
#define CLK_SYS_CFG_DIV_MASK (0x1fu) /**< bits 4:0 */

/* --- PLL_CONFIG bits ----------------------------------------------------- */
#define PLL_CONFIG_HALT (1 << 5) /**< 1 = PLL halted */
#define PLL_CONFIG_DRV (1 << 7)	 /**< flash clock drive */

/* --- Source selection ---------------------------------------------------- */

/** System clock sources, as written to R16_CLK_SYS_CFG. */
typedef enum {
	CLK_SOURCE_LSI = 0x00, /**< ~32 kHz internal RC */
	CLK_SOURCE_LSE = 0x01, /**< 32.768 kHz crystal */

	CLK_SOURCE_HSE_16MHZ = 0x22,  /**< XT32M / 2 */
	CLK_SOURCE_HSE_8MHZ = 0x24,   /**< XT32M / 4 */
	CLK_SOURCE_HSE_6_4MHZ = 0x25, /**< XT32M / 5 */
	CLK_SOURCE_HSE_4MHZ = 0x28,   /**< XT32M / 8 */
	CLK_SOURCE_HSE_2MHZ = 0x30,   /**< XT32M / 16 */
	CLK_SOURCE_HSE_1MHZ = 0x20,   /**< XT32M / 32 */

	CLK_SOURCE_PLL_80MHZ = 0x46,   /**< 480 / 6 */
	CLK_SOURCE_PLL_60MHZ = 0x48,   /**< 480 / 8 */
	CLK_SOURCE_PLL_48MHZ = 0x4a,   /**< 480 / 10  (USB) */
	CLK_SOURCE_PLL_40MHZ = 0x4c,   /**< 480 / 12 */
	CLK_SOURCE_PLL_36_9MHZ = 0x4d, /**< 480 / 13 */
	CLK_SOURCE_PLL_32MHZ = 0x4f,   /**< 480 / 15 */
	CLK_SOURCE_PLL_30MHZ = 0x50,   /**< 480 / 16 */
	CLK_SOURCE_PLL_24MHZ = 0x54,   /**< 480 / 20 */
	CLK_SOURCE_PLL_20MHZ = 0x58,   /**< 480 / 24 */
	CLK_SOURCE_PLL_15MHZ = 0x40,   /**< 480 / 32 */
} clk_source_t;

/** Nominal XT32M frequency, in Hz.  The crystal on every WCH board is 32 MHz. */
#define CLK_XT32M_FREQUENCY 32000000u
/** The PLL is a fixed 480 MHz multiplier of XT32M. */
#define CLK_PLL_FREQUENCY 480000000u
/** LSI frequency, in Hz. */
#define CLK_LSI_FREQUENCY 32000u
/** LSE frequency, in Hz. */
#define CLK_LSE_FREQUENCY 32768u

/* --- Crystal tuning ------------------------------------------------------ */

/** Drive current for XT32M / XT32K. */
typedef enum {
	CLK_HSE_CURRENT_25MA = 0,
	CLK_HSE_CURRENT_50MA,
	CLK_HSE_CURRENT_75MA,
	CLK_HSE_CURRENT_100MA,
} clk_current_t;

/** Load capacitance for XT32M / XT32K. */
typedef enum {
	CLK_HSE_CAP_0PF = 0,
	CLK_HSE_CAP_6PF,
	CLK_HSE_CAP_10PF,
	CLK_HSE_CAP_12PF,
	CLK_HSE_CAP_14PF,
	CLK_HSE_CAP_16PF,
	CLK_HSE_CAP_18PF,
	CLK_HSE_CAP_20PF,
	CLK_HSE_CAP_22PF,
	CLK_HSE_CAP_24PF,
	CLK_HSE_CAP_26PF,
	CLK_HSE_CAP_28PF,
	CLK_HSE_CAP_30PF,
	CLK_HSE_CAP_32PF,
} clk_cap_t;

BEGIN_DECLS

/* --- Clock module power -------------------------------------------------- */

/** Power up the 32 MHz crystal and wait for it to start. */
void clk_hse_enable(void);
void clk_hse_disable(void);
/** Power up the PLL and wait for it to lock. */
void clk_pll_enable(void);
void clk_pll_disable(void);
void clk_lse_enable(void);
void clk_lsi_enable(void);
void clk_lse_disable(void);
void clk_lsi_disable(void);

/** Configure the 32 MHz crystal drive current / load capacitance. */
void clk_hse_set_current(clk_current_t current);
void clk_hse_set_capacitance(clk_cap_t cap);
void clk_lse_set_current(clk_current_t current);
void clk_lse_set_capacitance(clk_cap_t cap);

/** Select which 32 kHz source drives the low-speed domain. */
void clk_32k_select(uint32_t source);

/* --- System clock -------------------------------------------------------- */

/**
 * Switch the system clock to `source`.  Powers the crystal and the PLL first
 * if they are not already running, so this is safe to call straight from
 * reset.
 */
void clk_set_sys_clock(clk_source_t source);

/** Current system clock frequency in Hz, measured from R16_CLK_SYS_CFG. */
uint32_t clk_get_sys_clock(void);

/** Convenience: 48 MHz from the PLL, the configuration USB requires. */
void clk_set_sys_clock_48mhz(void);

END_DECLS

#endif
/**@}*/
