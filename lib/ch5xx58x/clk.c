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

/** @addtogroup clk_file CLK
 *
 * @ingroup CH5XX58X
 *
 * @brief <b>Clock control for the CH58x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * Every register touched here is RWA, so each individual write is wrapped in
 * its own safe-access window through the RWA_* macros.
 *
 * Crystal start-up: WCH's own code waits 1200 iterations of two nops for
 * XT32M and 2000 for the PLL.  Those counts are what the vendor ships, so
 * they are kept; they are deliberately generous.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch5xx58x/clk.h>
#include <libopenwch/ch5xx58x/rwa.h>
#include <libopenwch/qingke/assert.h>

/* Start-up delays, in iterations of two nops, as used by WCH's EVT. */
#define CLK_XT32M_STARTUP_LOOPS 1200u
#define CLK_PLL_LOCK_LOOPS 2000u

/** Burn a few cycles so the peripheral can act on the previous write. */
static void clk_short_delay(uint32_t loops) {
	while (loops--) {
		qingke_nop();
		qingke_nop();
	}
}

/* --- Clock module power -------------------------------------------------- */

void clk_hse_enable(void) {
	if (HFCK_PWR_CTRL & CLK_XT32M_PON) {
		return;
	}

	RWA_SET_BITS(HFCK_PWR_CTRL, CLK_XT32M_PON);
	clk_short_delay(CLK_XT32M_STARTUP_LOOPS);
}

void clk_hse_disable(void) {
	RWA_CLEAR_BITS(HFCK_PWR_CTRL, CLK_XT32M_PON);
}

void clk_pll_enable(void) {
	if (HFCK_PWR_CTRL & CLK_PLL_PON) {
		return;
	}

	RWA_SET_BITS(HFCK_PWR_CTRL, CLK_PLL_PON);
	clk_short_delay(CLK_PLL_LOCK_LOOPS);
}

void clk_pll_disable(void) {
	RWA_CLEAR_BITS(HFCK_PWR_CTRL, CLK_PLL_PON);
}

void clk_lse_enable(void) {
	RWA_SET_BITS(HFCK_PWR_CTRL, CLK_XT32K_PON);
}

void clk_lsi_enable(void) {
	RWA_SET_BITS(HFCK_PWR_CTRL, CLK_LSI_PON);
}

void clk_lse_disable(void) {
	RWA_CLEAR_BITS(HFCK_PWR_CTRL, CLK_XT32K_PON);
}

void clk_lsi_disable(void) {
	RWA_CLEAR_BITS(HFCK_PWR_CTRL, CLK_LSI_PON);
}

void clk_hse_set_current(clk_current_t current) {
	uint8_t reg = XT32M_TUNE;

	openwch_assert(current <= CLK_HSE_CURRENT_100MA);

	reg = (uint8_t)((reg & ~0x03u) | ((uint8_t)current & 0x03u));
	RWA_WRITE8(XT32M_TUNE, reg);
}

void clk_hse_set_capacitance(clk_cap_t cap) {
	uint8_t reg = XT32M_TUNE;

	openwch_assert(cap <= CLK_HSE_CAP_32PF);

	reg = (uint8_t)((reg & ~0x3cu) | (((uint8_t)cap << 2) & 0x3cu));
	RWA_WRITE8(XT32M_TUNE, reg);
}

void clk_lse_set_current(clk_current_t current) {
	uint8_t reg = MMIO8(SYS_BASE + 0x2e);

	openwch_assert(current <= CLK_HSE_CURRENT_100MA);

	reg = (uint8_t)((reg & ~0x03u) | ((uint8_t)current & 0x03u));
	RWA_WRITE8(MMIO8(SYS_BASE + 0x2e), reg);
}

void clk_lse_set_capacitance(clk_cap_t cap) {
	uint8_t reg = MMIO8(SYS_BASE + 0x2e);

	openwch_assert(cap <= CLK_HSE_CAP_32PF);

	reg = (uint8_t)((reg & ~0x3cu) | (((uint8_t)cap << 2) & 0x3cu));
	RWA_WRITE8(MMIO8(SYS_BASE + 0x2e), reg);
}

void clk_32k_select(uint32_t source) {
	/*
	 * R8_CK32K_CONFIG selects between the 32.768 kHz crystal and the
	 * internal RC.  Its documented values are 0 (LSI) and 0x80 (LSE).
	 */
	RWA_WRITE8(MMIO8(SYS_BASE + 0x2f),
		   (source == CLK_SOURCE_LSE) ? 0x80u : 0x00u);
}

/* --- System clock -------------------------------------------------------- */

void clk_set_sys_clock(clk_source_t source) {
	uint8_t cfg = (uint8_t)source;

	/*
	 * Take the PLL out of its halted low-power state before anything
	 * else; the divider write below is meaningless while it is halted.
	 */
	RWA_CLEAR_BITS(PLL_CONFIG, PLL_CONFIG_HALT);

	if (cfg & CLK_SYS_CFG_MOD_PLL) {
		/*
		 * PLL path.  Powering XT32M is the caller's job in WCH's EVT,
		 * which is a common source of a silently dead PLL; do it here
		 * so that one call is always enough.
		 */
		clk_hse_enable();
		clk_pll_enable();

		RWA_MODIFY(MMIO16(R16_CLK_SYS_CFG), 0x00ffu,
			   (uint32_t)(cfg & (CLK_SYS_CFG_MOD_MASK |
					     CLK_SYS_CFG_DIV_MASK)));
		clk_short_delay(4);

		/*
		 * Flash wait states.  80 MHz needs the fast setting; everything
		 * slower uses the standard one.
		 */
		if (cfg == CLK_SOURCE_PLL_80MHZ) {
			RWA_WRITE8(FLASH_CFG, 0x02u);
		} else {
			RWA_WRITE8(FLASH_CFG, 0x52u);
		}

		RWA_SET_BITS(PLL_CONFIG, PLL_CONFIG_DRV);
	} else if (cfg & 0x20u) {
		/* XT32M divider path. */
		clk_hse_enable();

		RWA_MODIFY(MMIO16(R16_CLK_SYS_CFG), 0x00ffu,
			   (uint32_t)(cfg & (CLK_SYS_CFG_MOD_MASK |
					     CLK_SYS_CFG_DIV_MASK)));
		clk_short_delay(4);

		RWA_WRITE8(FLASH_CFG, 0x51u);
	} else {
		/* 32 kHz source: bits 7:6 = 0b11. */
		RWA_MODIFY(MMIO16(R16_CLK_SYS_CFG), CLK_SYS_CFG_MOD_MASK,
			   CLK_SYS_CFG_MOD_32K);
	}
}

uint32_t clk_get_sys_clock(void) {
	uint32_t cfg = MMIO16(R16_CLK_SYS_CFG) & 0xffu;
	uint32_t div = cfg & CLK_SYS_CFG_DIV_MASK;

	switch (cfg & CLK_SYS_CFG_MOD_MASK) {
	case CLK_SYS_CFG_MOD_HSE:
		/* XT32M divided.  A divider of 0 is not meaningful, but guard
		 * against it rather than dividing by zero. */
		return div ? (CLK_XT32M_FREQUENCY / div) : CLK_XT32M_FREQUENCY;
	case CLK_SYS_CFG_MOD_PLL:
		return div ? (CLK_PLL_FREQUENCY / div) : CLK_PLL_FREQUENCY;
	default:
		/* 32 kHz domain.  R8_CK32K_CONFIG decides which source. */
		return (MMIO8(SYS_BASE + 0x2f) & 0x80u) ? CLK_LSE_FREQUENCY
							: CLK_LSI_FREQUENCY;
	}
}

void clk_set_sys_clock_48mhz(void) {
	clk_set_sys_clock(CLK_SOURCE_PLL_48MHZ);
}
/**@}*/
