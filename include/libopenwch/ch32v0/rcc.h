/** @defgroup rcc_defines RCC Defines

@brief <b>Reset and Clock Control for the CH32V00x</b>

@ingroup CH32V0_defines

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

#ifndef LIBOPENWCH_RCC_H
#define LIBOPENWCH_RCC_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/ch32v0/memorymap.h>

/* --- Register accessors -------------------------------------------------- */

/* Clock control register */
#define RCC_CTLR MMIO32(RCC_BASE + 0x00)
/* Clock configuration register */
#define RCC_CFGR0 MMIO32(RCC_BASE + 0x04)
/* Clock interrupt register */
#define RCC_INTR MMIO32(RCC_BASE + 0x08)
/* APB2 peripheral reset register */
#define RCC_APB2PRSTR MMIO32(RCC_BASE + 0x0c)
/* APB1 peripheral reset register */
#define RCC_APB1PRSTR MMIO32(RCC_BASE + 0x10)
/* AHB peripheral clock enable register */
#define RCC_AHBPCENR MMIO32(RCC_BASE + 0x14)
/* APB2 peripheral clock enable register */
#define RCC_APB2PCENR MMIO32(RCC_BASE + 0x18)
/* APB1 peripheral clock enable register */
#define RCC_APB1PCENR MMIO32(RCC_BASE + 0x1c)
/* Control/status register */
#define RCC_RSTSCKR MMIO32(RCC_BASE + 0x24)

/* --- Flash access control register --------------------------------------- */
/*
 * The flash wait-state field lives in the flash controller block, but it is
 * programmed as part of every clock change, so libopencm3-style code expects
 * it near the RCC definitions.
 */
#define FLASH_ACTLR MMIO32(FLASH_R_BASE + 0x00)
#define FLASH_ACTLR_LATENCY_MASK 0x3u
#define FLASH_ACTLR_LATENCY_0 0x0u
#define FLASH_ACTLR_LATENCY_1 0x1u
#define FLASH_ACTLR_LATENCY_2 0x2u

/* --- RCC_CTLR bits ------------------------------------------------------- */

#define RCC_CTLR_HSION (1 << 0)
#define RCC_CTLR_HSIRDY (1 << 1)
#define RCC_CTLR_HSITRIM_SHIFT 3
#define RCC_CTLR_HSITRIM_MASK (0x1fu << 3)
#define RCC_CTLR_HSICAL_SHIFT 8
#define RCC_CTLR_HSICAL_MASK (0xffu << 8)
#define RCC_CTLR_HSEON (1 << 16)
#define RCC_CTLR_HSERDY (1 << 17)
#define RCC_CTLR_HSEBYP (1 << 18)
#define RCC_CTLR_CSSON (1 << 19)
#define RCC_CTLR_PLLON (1 << 24)
#define RCC_CTLR_PLLRDY (1 << 25)

/* --- RCC_CFGR0 bits ------------------------------------------------------ */

#define RCC_CFGR0_SW_SHIFT 0
#define RCC_CFGR0_SW_MASK (0x3u << 0)
#define RCC_CFGR0_SW_HSI 0x0
#define RCC_CFGR0_SW_HSE 0x1
#define RCC_CFGR0_SW_PLL 0x2

#define RCC_CFGR0_SWS_SHIFT 2
#define RCC_CFGR0_SWS_MASK (0x3u << 2)

#define RCC_CFGR0_HPRE_SHIFT 4
#define RCC_CFGR0_HPRE_MASK (0xfu << 4)
#define RCC_CFGR0_HPRE_DIV1 0x0
#define RCC_CFGR0_HPRE_DIV2 0x8
#define RCC_CFGR0_HPRE_DIV4 0x9
#define RCC_CFGR0_HPRE_DIV8 0xa
#define RCC_CFGR0_HPRE_DIV16 0xb
#define RCC_CFGR0_HPRE_DIV64 0xc
#define RCC_CFGR0_HPRE_DIV128 0xd
#define RCC_CFGR0_HPRE_DIV256 0xe
#define RCC_CFGR0_HPRE_DIV512 0xf

#define RCC_CFGR0_PPRE1_SHIFT 8
#define RCC_CFGR0_PPRE1_MASK (0x7u << 8)
#define RCC_CFGR0_PPRE2_SHIFT 11
#define RCC_CFGR0_PPRE2_MASK (0x7u << 11)
#define RCC_CFGR0_PPRE_DIV1 0x0
#define RCC_CFGR0_PPRE_DIV2 0x4
#define RCC_CFGR0_PPRE_DIV4 0x5
#define RCC_CFGR0_PPRE_DIV8 0x6
#define RCC_CFGR0_PPRE_DIV16 0x7

#define RCC_CFGR0_ADCPRE_SHIFT 14
#define RCC_CFGR0_ADCPRE_MASK (0x3u << 14)
#define RCC_CFGR0_ADCPRE_DIV2 0x0
#define RCC_CFGR0_ADCPRE_DIV4 0x1
#define RCC_CFGR0_ADCPRE_DIV6 0x2
#define RCC_CFGR0_ADCPRE_DIV8 0x3

#define RCC_CFGR0_PLLSRC (1 << 16)
#define RCC_CFGR0_PLLXTPRE (1 << 17)
#define RCC_CFGR0_USBPRE (1 << 22)
#define RCC_CFGR0_MCO_SHIFT 24
#define RCC_CFGR0_MCO_MASK (0x7u << 24)

/* --- RCC_INTR bits ------------------------------------------------------- */

#define RCC_INTR_LSIRDY (1 << 0)
#define RCC_INTR_HSIRDY (1 << 2)
#define RCC_INTR_HSERDY (1 << 3)
#define RCC_INTR_PLLRDY (1 << 5)
#define RCC_INTR_CSS (1 << 7)
#define RCC_INTR_CLEAR_MASK 0x009f0000u

/* --- Peripheral clock enable bits ---------------------------------------- */

/*
 * Peripheral clock identifiers.
 *
 * The three enable registers are separate, so a peripheral identifier has to
 * say which bus it belongs to as well as which bit to set.  WCH uses bit
 * positions up to 28 (PWR, for example), so the bus selector lives in the top
 * two bits [31:30] and the bit position in bits [29:0].  That lets
 * rcc_periph_clock_enable() pick the register from the value itself while a
 * plain `RCC_APB2_GPIOA`-style mask stays available to callers that already
 * know the bus.
 */

/** @defgroup rcc_periph_bus Bus Selectors
@ingroup rcc_defines

@{*/
#define RCC_PERIPH_BUS_SHIFT 30u
#define RCC_PERIPH_BUS_AHB (0x0u << RCC_PERIPH_BUS_SHIFT)
#define RCC_PERIPH_BUS_APB1 (0x1u << RCC_PERIPH_BUS_SHIFT)
#define RCC_PERIPH_BUS_APB2 (0x2u << RCC_PERIPH_BUS_SHIFT)
#define RCC_PERIPH_BUS_MASK (0x3u << RCC_PERIPH_BUS_SHIFT)
#define RCC_PERIPH_BIT_MASK (0x3fffffffu)
/**@}*/

/** @defgroup rcc_ahb_periph AHB Peripheral Identifiers
@ingroup rcc_defines

@{*/
#define RCC_AHB_DMA1 (1 << 0)
#define RCC_AHB_SRAM (1 << 2)

#define RCC_DMA1 (RCC_PERIPH_BUS_AHB | RCC_AHB_DMA1)
#define RCC_SRAM (RCC_PERIPH_BUS_AHB | RCC_AHB_SRAM)
/**@}*/

/** @defgroup rcc_apb2_periph APB2 Peripheral Identifiers
@ingroup rcc_defines

@{*/
#define RCC_APB2_AFIO (1 << 0)
#define RCC_APB2_GPIOA (1 << 2)
#define RCC_APB2_GPIOC (1 << 4)
#define RCC_APB2_GPIOD (1 << 5)
#define RCC_APB2_ADC1 (1 << 9)
#define RCC_APB2_TIM1 (1 << 11)
#define RCC_APB2_SPI1 (1 << 12)
#define RCC_APB2_USART1 (1 << 14)

#define RCC_AFIO (RCC_PERIPH_BUS_APB2 | RCC_APB2_AFIO)
#define RCC_GPIOA (RCC_PERIPH_BUS_APB2 | RCC_APB2_GPIOA)
#define RCC_GPIOC (RCC_PERIPH_BUS_APB2 | RCC_APB2_GPIOC)
#define RCC_GPIOD (RCC_PERIPH_BUS_APB2 | RCC_APB2_GPIOD)
#define RCC_ADC1 (RCC_PERIPH_BUS_APB2 | RCC_APB2_ADC1)
#define RCC_TIM1 (RCC_PERIPH_BUS_APB2 | RCC_APB2_TIM1)
#define RCC_SPI1 (RCC_PERIPH_BUS_APB2 | RCC_APB2_SPI1)
#define RCC_USART1 (RCC_PERIPH_BUS_APB2 | RCC_APB2_USART1)
/**@}*/

/** @defgroup rcc_apb1_periph APB1 Peripheral Identifiers
@ingroup rcc_defines

@{*/
#define RCC_APB1_TIM2 (1 << 0)
#define RCC_APB1_WWDG (1 << 11)
#define RCC_APB1_I2C1 (1 << 21)
#define RCC_APB1_PWR (1 << 28)

#define RCC_TIM2 (RCC_PERIPH_BUS_APB1 | RCC_APB1_TIM2)
#define RCC_WWDG (RCC_PERIPH_BUS_APB1 | RCC_APB1_WWDG)
#define RCC_I2C1 (RCC_PERIPH_BUS_APB1 | RCC_APB1_I2C1)
#define RCC_PWR (RCC_PERIPH_BUS_APB1 | RCC_APB1_PWR)
/**@}*/

/* --- Clock constants ----------------------------------------------------- */

/** Internal high-speed RC oscillator frequency, in Hz. */
#define RCC_HSI_FREQUENCY 24000000u
/** External crystal frequency, in Hz.  Board dependent; 24 MHz is the value
 * the WCH evaluation boards use and the one the EVT defaults to. */
#ifndef RCC_HSE_FREQUENCY
#define RCC_HSE_FREQUENCY 24000000u
#endif
/** Internal low-speed RC oscillator frequency, in Hz. */
#define RCC_LSI_FREQUENCY 40000u
/** The CH32V00x PLL only multiplies by two. */
#define RCC_PLL_MUL 2u

/* --- Clock monitor / reset flags ----------------------------------------- */

#define RCC_RSTSCKR_LSION (1 << 0)
#define RCC_RSTSCKR_LSIRDY (1 << 1)
#define RCC_RSTSCKR_RMVF (1 << 24)
#define RCC_RSTSCKR_PINRSTF (1 << 26)
#define RCC_RSTSCKR_PORRSTF (1 << 27)
#define RCC_RSTSCKR_SFTRSTF (1 << 28)
#define RCC_RSTSCKR_IWDGRSTF (1 << 29)
#define RCC_RSTSCKR_WWDGRSTF (1 << 30)
#define RCC_RSTSCKR_LPWRRSTF (1 << 31)

/* --- Types --------------------------------------------------------------- */

/** Clock source and frequency at which the system clock can run. */
enum rcc_sysclk {
	RCC_SYSCLK_HSI_24MHZ,
	RCC_SYSCLK_HSE_DIRECT,
	RCC_SYSCLK_PLL_HSI_48MHZ,
	RCC_SYSCLK_PLL_HSE_48MHZ,
};

/** Index into @ref rcc_hsi_configs. */
enum rcc_clock_hsi {
	RCC_CLOCK_HSI_24MHZ = 0,
	RCC_CLOCK_PLL_HSI_48MHZ,
	RCC_CLOCK_HSI_END,
};

/** Index into @ref rcc_hse_configs. */
enum rcc_clock_hse {
	RCC_CLOCK_HSE_DIRECT = 0,
	RCC_CLOCK_PLL_HSE_48MHZ,
	RCC_CLOCK_HSE_END,
};

/**
 * One selectable system clock configuration.
 *
 * The first group of fields is the request -- what rcc_clock_setup_pll()
 * programs.  The second group is the outcome, recorded in the table so that
 * the setup call can publish the resulting frequencies without measuring
 * anything.  Both live in one structure because that is the shape libopencm3
 * gives its clock tables, and it is what lets a caller name a whole clock tree
 * in a single line:
 *
 *	rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_PLL_HSI_48MHZ]);
 *
 * The prescaler fields carry the register's own codes, not the divider.
 */
struct rcc_clock_scale {
	enum rcc_sysclk source;	 /**< which tree to bring up */
	uint32_t pll_source;	 /**< non-zero: PLL from HSE, zero: from HSI */
	uint32_t flash_latency;	 /**< FLASH_ACTLR latency code */
	uint32_t ahb_prescaler;	 /**< RCC_CFGR0_HPRE_* code */
	uint32_t apb1_prescaler; /**< RCC_CFGR0_PPRE_DIV* code */
	uint32_t apb2_prescaler; /**< RCC_CFGR0_PPRE_DIV* code */
	uint32_t adc_prescaler;	 /**< RCC_CFGR0_ADCPRE_DIV* code */

	uint32_t sysclk_frequency; /**< resulting SYSCLK, in Hz */
	uint32_t ahb_frequency;	   /**< resulting HCLK, in Hz */
	uint32_t apb1_frequency;   /**< resulting PCLK1, in Hz */
	uint32_t apb2_frequency;   /**< resulting PCLK2, in Hz */
	uint32_t adc_frequency;	   /**< resulting ADCCLK, in Hz */
};

BEGIN_DECLS

/* --- Clock setup --------------------------------------------------------- */

/** Configurations derived from the internal RC oscillator. */
extern const struct rcc_clock_scale rcc_hsi_configs[RCC_CLOCK_HSI_END];
/** Configurations derived from the external crystal. */
extern const struct rcc_clock_scale rcc_hse_configs[RCC_CLOCK_HSE_END];

/**
 * Bring the system clock up to @p clock.
 *
 * Everything the tree needs comes from the descriptor: the oscillator is
 * started and waited for, the prescalers and the flash latency are set, and
 * the system clock is switched.  One call, no temporary, no follow-up
 * measurement -- the rcc_*_frequency variables below already describe the new
 * tree when this returns.
 */
void rcc_clock_setup_pll(const struct rcc_clock_scale *clock);

/** The named configurations, for callers that prefer a function name. */
void rcc_clock_setup_hsi_24mhz(void);
void rcc_clock_setup_hsi_48mhz(void);
void rcc_clock_setup_hse_48mhz(void);
void rcc_clock_setup_pll_hse_48mhz(void);

/** Select a configuration by @ref rcc_sysclk value. */
void rcc_clock_setup_sysclk(enum rcc_sysclk source);

/*
 * The frequencies of the running tree, in Hz.
 *
 * Published as variables the way libopencm3 publishes them, so that a driver
 * or an application reads the number it needs instead of asking for a
 * measurement and holding the answer in a temporary of its own.
 */
extern uint32_t rcc_sysclk_frequency; /**< SYSCLK */
extern uint32_t rcc_ahb_frequency;    /**< HCLK */
extern uint32_t rcc_apb1_frequency;   /**< PCLK1 */
extern uint32_t rcc_apb2_frequency;   /**< PCLK2 */
extern uint32_t rcc_adc_frequency;    /**< ADCCLK */

/** Re-measure the tree from the registers and refresh the variables above. */
void rcc_measure_clocks(void);

void rcc_osc_on(uint32_t osc);
void rcc_osc_off(uint32_t osc);
void rcc_wait_for_osc_ready(uint32_t osc);

void rcc_periph_clock_enable(uint32_t periph);
void rcc_periph_clock_disable(uint32_t periph);
void rcc_periph_reset_pulse(uint32_t periph);
void rcc_periph_reset_hold(uint32_t periph);
void rcc_periph_reset_release(uint32_t periph);

void rcc_set_sysclk_source(uint32_t source);
void rcc_set_pll_source(uint32_t source);
void rcc_ahb_set_prescaler(uint32_t ppre);
void rcc_apb1_set_prescaler(uint32_t ppre);
void rcc_apb2_set_prescaler(uint32_t ppre);
void rcc_adc_set_prescaler(uint32_t ppre);

uint32_t rcc_get_sysclk_frequency(void);

void rcc_clock_security_system_enable(void);
void rcc_clock_security_system_disable(void);

void rcc_clear_reset_flags(void);
uint32_t rcc_get_reset_flags(void);

END_DECLS

/* Clock source identifiers used by rcc_osc_on()/rcc_osc_off(). */
#define RCC_OSC_NONE (0x00u)
#define RCC_OSC_HSI (0x01u)
#define RCC_OSC_HSE (0x02u)
#define RCC_OSC_PLL (0x03u)
#define RCC_OSC_LSI (0x04u)

#endif
/**@}*/
