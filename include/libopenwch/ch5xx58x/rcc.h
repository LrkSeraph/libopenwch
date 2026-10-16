/** @defgroup rcc_defines RCC Defines

@brief <b>Reset and clock control for the CH58x</b>

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

/**@{*/

#ifndef LIBOPENWCH_CH5XX58X_RCC_H
#define LIBOPENWCH_CH5XX58X_RCC_H

#include <libopenwch/ch5xx58x/clk.h>

/*
 * Unified clock-configuration view
 * ---------------------------------
 *
 * The low-level clk_*() interface above names one register value at a time.
 * This header adds the same table-driven style used by the CH32V0 family:
 * a clock configuration is a row in rcc_sysclk_configs[], and one call names
 * the whole tree:
 *
 *rcc_clock_setup_pll(&rcc_sysclk_configs[RCC_SYSCLK_PLL_60MHZ]);
 *
 * The CH58x has fewer independent fields than the CH32V0 RCC -- the source
 * byte selects the complete tree -- so the descriptor stores only the source
 * code and the resulting SYSCLK frequency.  clk_*() remains available for
 * callers that want to work directly with the register values.
 */

/** One selectable CH58x system-clock tree. */
struct rcc_clock_scale {
	clk_source_t source;	   /**< byte written to R16_CLK_SYS_CFG */
	uint32_t sysclk_frequency; /**< resulting SYSCLK, in Hz */
};

/** Index into @ref rcc_sysclk_configs. */
enum rcc_sysclk {
	RCC_SYSCLK_LSI = 0,
	RCC_SYSCLK_LSE,
	RCC_SYSCLK_HSE_16MHZ,
	RCC_SYSCLK_HSE_8MHZ,
	RCC_SYSCLK_HSE_6_4MHZ,
	RCC_SYSCLK_HSE_4MHZ,
	RCC_SYSCLK_HSE_2MHZ,
	RCC_SYSCLK_HSE_1MHZ,
	RCC_SYSCLK_PLL_80MHZ,
	RCC_SYSCLK_PLL_60MHZ,
	RCC_SYSCLK_PLL_48MHZ,
	RCC_SYSCLK_PLL_40MHZ,
	RCC_SYSCLK_PLL_36_9MHZ,
	RCC_SYSCLK_PLL_32MHZ,
	RCC_SYSCLK_PLL_30MHZ,
	RCC_SYSCLK_PLL_24MHZ,
	RCC_SYSCLK_PLL_20MHZ,
	RCC_SYSCLK_PLL_15MHZ,
	RCC_SYSCLK_END,
};

/** All selectable trees, in @ref rcc_sysclk order. */
extern const struct rcc_clock_scale rcc_sysclk_configs[RCC_SYSCLK_END];

BEGIN_DECLS

/**
 * Bring the system clock up to @p clock.
 *
 * This is the common table-driven entry point.  The descriptor names the
 * source byte and the resulting SYSCLK; the function powers the crystal/PLL
 * as needed, applies the flash timing, switches the tree, and publishes
 * rcc_sysclk_frequency.
 */
void rcc_clock_setup(const struct rcc_clock_scale *clock);

/**
 * Alias kept for source compatibility with the CH32V0 family spelling.
 * On CH58x it is not restricted to PLL sources; any table entry is valid.
 */
void rcc_clock_setup_pll(const struct rcc_clock_scale *clock);

/** Select one entry of @ref rcc_sysclk_configs by enum. */
void rcc_clock_setup_sysclk(enum rcc_sysclk source);

/** Common named configurations. */
void rcc_clock_setup_pll_60mhz(void);
void rcc_clock_setup_pll_48mhz(void);

END_DECLS

#endif
/**@}*/
