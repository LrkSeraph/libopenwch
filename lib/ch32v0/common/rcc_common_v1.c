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

/** @addtogroup rcc_file RCC
 *
 * @ingroup CH32V0
 *
 * @brief <b>Reset and Clock Control for the CH32V00x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * The CH32V00x has a 24 MHz HSI RC oscillator, an HSE input and a PLL that can
 * only multiply by two, for a maximum system clock of 48 MHz.  There is no
 * HSI/2 path, no PLL M/N dividers and no independent ADC clock domain beyond
 * the ADCPRE prescaler.
 *
 * Reaching 48 MHz from the HSI additionally requires the factory PLL trim
 * value stored at CFG0_PLL_TRIM (0x1FFFF7D4).  WCH's EVT applies it whenever
 * the byte is not 0xFF; we do the same, because a part whose trim byte was
 * never programmed must fall back to the nominal 0x10 setting.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch32v0/rcc.h>
#include <libopenwch/qingke/assert.h>

/* Timeout for oscillator start-up, in loop iterations. */
#define RCC_OSC_TIMEOUT			0x2000u

/* Burned-in factory PLL trim, applied when it has been programmed. */
#define RCC_VENDOR_CFG0_PLL_TRIM	(*(volatile uint8_t *)VENDOR_CFG0_BASE)

/* The nominal HSI trim when no factory value is available. */
#define RCC_HSITRIM_NOMINAL		0x10u

/* --- Oscillator control -------------------------------------------------- */

void rcc_osc_on(uint32_t osc)
{
	openwch_assert(osc != RCC_OSC_NONE);

	if (osc == RCC_OSC_HSI) {
		RCC_CTLR |= RCC_CTLR_HSION;
	} else if (osc == RCC_OSC_HSE) {
		RCC_CTLR |= RCC_CTLR_HSEON;
	} else if (osc == RCC_OSC_PLL) {
		RCC_CTLR |= RCC_CTLR_PLLON;
	} else if (osc == RCC_OSC_LSI) {
		RCC_RSTSCKR |= RCC_RSTSCKR_LSION;
	} else {
		openwch_assert(0);
	}
}

void rcc_osc_off(uint32_t osc)
{
	openwch_assert(osc != RCC_OSC_NONE);

	if (osc == RCC_OSC_HSI) {
		RCC_CTLR &= ~RCC_CTLR_HSION;
	} else if (osc == RCC_OSC_HSE) {
		RCC_CTLR &= ~RCC_CTLR_HSEON;
	} else if (osc == RCC_OSC_PLL) {
		RCC_CTLR &= ~RCC_CTLR_PLLON;
	} else if (osc == RCC_OSC_LSI) {
		RCC_RSTSCKR &= ~RCC_RSTSCKR_LSION;
	} else {
		openwch_assert(0);
	}
}

void rcc_wait_for_osc_ready(uint32_t osc)
{
	uint32_t timeout = RCC_OSC_TIMEOUT;

	openwch_assert(osc != RCC_OSC_NONE);

	while (timeout--) {
		uint32_t ready;

		if (osc == RCC_OSC_HSI) {
			ready = RCC_CTLR & RCC_CTLR_HSIRDY;
		} else if (osc == RCC_OSC_HSE) {
			ready = RCC_CTLR & RCC_CTLR_HSERDY;
		} else if (osc == RCC_OSC_PLL) {
			ready = RCC_CTLR & RCC_CTLR_PLLRDY;
		} else {
			ready = RCC_RSTSCKR & RCC_RSTSCKR_LSIRDY;
		}

		if (ready) {
			return;
		}
	}

	/* The oscillator never came up.  There is nothing sensible left to do:
	 * continuing would silently run the core at the wrong frequency. */
	openwch_assert_not_reached();
}

/* --- System clock selection ---------------------------------------------- */

void rcc_set_sysclk_source(uint32_t source)
{
	uint32_t cfgr0 = RCC_CFGR0;

	cfgr0 &= ~RCC_CFGR0_SW_MASK;
	cfgr0 |= (source << RCC_CFGR0_SW_SHIFT) & RCC_CFGR0_SW_MASK;
	RCC_CFGR0 = cfgr0;
}

uint32_t rcc_get_sysclk_frequency(void)
{
	struct rcc_clock_scale clocks;

	rcc_get_clocks_freq(&clocks);
	return clocks.sysclk;
}

/** Compute the frequency of an HCLK/APBx output from the HPRE/PPRE encoding. */
static uint32_t rcc_apply_prescaler(uint32_t freq, uint32_t reg, uint32_t shift)
{
	uint32_t code = (reg >> shift) & 0x7u;

	if (code < 0x4u) {
		return freq;
	}

	return freq >> (code - 0x3u);
}

void rcc_get_clocks_freq(struct rcc_clock_scale *clocks)
{
	uint32_t cfgr0 = RCC_CFGR0;
	uint32_t sysclk;
	uint32_t hpre;
	uint32_t pllsrc;

	switch ((cfgr0 & RCC_CFGR0_SWS_MASK) >> RCC_CFGR0_SWS_SHIFT) {
	case RCC_CFGR0_SW_HSI:
		sysclk = RCC_HSI_FREQUENCY;
		break;
	case RCC_CFGR0_SW_HSE:
		sysclk = RCC_HSE_FREQUENCY;
		break;
	case RCC_CFGR0_SW_PLL:
		pllsrc = cfgr0 & RCC_CFGR0_PLLSRC;
		if (pllsrc) {
			sysclk = RCC_HSE_FREQUENCY * RCC_PLL_MUL;
		} else {
			sysclk = RCC_HSI_FREQUENCY * RCC_PLL_MUL;
		}
		break;
	default:
		sysclk = RCC_HSI_FREQUENCY;
		break;
	}

	clocks->sysclk = sysclk;

	/* HPRE: 0x0..0x7 divide by 1, 0x8..0xf by 2..512. */
	hpre = (cfgr0 & RCC_CFGR0_HPRE_MASK) >> RCC_CFGR0_HPRE_SHIFT;
	if (hpre < 0x8u) {
		clocks->hclk = sysclk;
	} else {
		clocks->hclk = sysclk >> (hpre - 0x7u);
	}

	clocks->pclk1 = rcc_apply_prescaler(clocks->hclk, cfgr0, RCC_CFGR0_PPRE1_SHIFT);
	clocks->pclk2 = rcc_apply_prescaler(clocks->hclk, cfgr0, RCC_CFGR0_PPRE2_SHIFT);

	switch ((cfgr0 & RCC_CFGR0_ADCPRE_MASK) >> RCC_CFGR0_ADCPRE_SHIFT) {
	case RCC_CFGR0_ADCPRE_DIV2:
		clocks->adcclk = clocks->pclk2 / 2;
		break;
	case RCC_CFGR0_ADCPRE_DIV4:
		clocks->adcclk = clocks->pclk2 / 4;
		break;
	case RCC_CFGR0_ADCPRE_DIV6:
		clocks->adcclk = clocks->pclk2 / 6;
		break;
	default:
		clocks->adcclk = clocks->pclk2 / 8;
		break;
	}
}

/* --- Prescalers ---------------------------------------------------------- */

void rcc_ahb_set_prescaler(uint32_t ppre)
{
	RCC_CFGR0 = (RCC_CFGR0 & ~RCC_CFGR0_HPRE_MASK)
		| ((ppre << RCC_CFGR0_HPRE_SHIFT) & RCC_CFGR0_HPRE_MASK);
}

void rcc_apb1_set_prescaler(uint32_t ppre)
{
	RCC_CFGR0 = (RCC_CFGR0 & ~RCC_CFGR0_PPRE1_MASK)
		| ((ppre << RCC_CFGR0_PPRE1_SHIFT) & RCC_CFGR0_PPRE1_MASK);
}

void rcc_apb2_set_prescaler(uint32_t ppre)
{
	RCC_CFGR0 = (RCC_CFGR0 & ~RCC_CFGR0_PPRE2_MASK)
		| ((ppre << RCC_CFGR0_PPRE2_SHIFT) & RCC_CFGR0_PPRE2_MASK);
}

void rcc_adc_set_prescaler(uint32_t ppre)
{
	RCC_CFGR0 = (RCC_CFGR0 & ~RCC_CFGR0_ADCPRE_MASK)
		| ((ppre << RCC_CFGR0_ADCPRE_SHIFT) & RCC_CFGR0_ADCPRE_MASK);
}

void rcc_set_pll_source(uint32_t source)
{
	if (source) {
		RCC_CFGR0 |= RCC_CFGR0_PLLSRC;
	} else {
		RCC_CFGR0 &= ~RCC_CFGR0_PLLSRC;
	}
}

/* --- Clock setup presets ------------------------------------------------- */

/*
 * Bring the part from reset to 48 MHz on the HSI-derived PLL.
 *
 * The sequence follows WCH's own SetSysClockTo_48MHz_HSI: reset the clock
 * configuration, apply the factory PLL trim, set one flash wait state (the
 * flash cannot keep up with 48 MHz at zero wait states), select the HSI as
 * the PLL source, start the PLL and switch the system clock to it.
 */
void rcc_clock_setup_hsi_48mhz(void)
{
	uint32_t trim;

	/* Ensure the HSI is running and start from a known configuration. */
	RCC_CTLR |= RCC_CTLR_HSION;
	RCC_CFGR0 &= 0xf8ff0000u;
	RCC_CTLR &= 0xfef6ffffu;
	RCC_CTLR &= 0xfffbffffu;
	RCC_CFGR0 &= 0xfffeffffu;
	RCC_INTR = RCC_INTR_CLEAR_MASK;

	/* Factory PLL trim, when programmed. */
	trim = RCC_VENDOR_CFG0_PLL_TRIM;
	if (trim != 0xffu) {
		RCC_CTLR = (RCC_CTLR & ~RCC_CTLR_HSITRIM_MASK)
			| ((trim & 0x1fu) << RCC_CTLR_HSITRIM_SHIFT);
	} else {
		RCC_CTLR = (RCC_CTLR & ~RCC_CTLR_HSITRIM_MASK)
			| (RCC_HSITRIM_NOMINAL << RCC_CTLR_HSITRIM_SHIFT);
	}

	/* Flash latency 1 is required above 24 MHz. */
	FLASH_ACTLR = (FLASH_ACTLR & ~0x3u) | 0x1u;

	/* HCLK undivided, PLL fed from the HSI. */
	RCC_CFGR0 |= RCC_CFGR0_HPRE_DIV1;
	RCC_CFGR0 = (RCC_CFGR0 & ~RCC_CFGR0_PLLSRC);

	rcc_osc_on(RCC_OSC_PLL);
	rcc_wait_for_osc_ready(RCC_OSC_PLL);

	rcc_set_sysclk_source(RCC_CFGR0_SW_PLL);
	while (((RCC_CFGR0 & RCC_CFGR0_SWS_MASK) >> RCC_CFGR0_SWS_SHIFT)
			!= RCC_CFGR0_SW_PLL) {
		;
	}
}

/*
 * 48 MHz from the HSE crystal.  The HSE is required for USB and is the only
 * way to get an accurate system clock on this family.
 */
void rcc_clock_setup_hse_48mhz(void)
{
	rcc_osc_on(RCC_OSC_HSE);
	rcc_wait_for_osc_ready(RCC_OSC_HSE);

	RCC_CFGR0 = (RCC_CFGR0 & ~RCC_CFGR0_PLLXTPRE);
	FLASH_ACTLR = (FLASH_ACTLR & ~0x3u) | 0x1u;
	RCC_CFGR0 |= RCC_CFGR0_HPRE_DIV1;
	RCC_CFGR0 |= RCC_CFGR0_PLLSRC;

	rcc_osc_on(RCC_OSC_PLL);
	rcc_wait_for_osc_ready(RCC_OSC_PLL);

	rcc_set_sysclk_source(RCC_CFGR0_SW_PLL);
	while (((RCC_CFGR0 & RCC_CFGR0_SWS_MASK) >> RCC_CFGR0_SWS_SHIFT)
			!= RCC_CFGR0_SW_PLL) {
		;
	}
}

/* Alias kept so that callers can be explicit about the PLL being involved. */
void rcc_clock_setup_pll_hse_48mhz(void)
{
	rcc_clock_setup_hse_48mhz();
}

void rcc_clock_setup_sysclk(enum rcc_sysclk source)
{
	switch (source) {
	case RCC_SYSCLK_HSI_24MHZ:
		rcc_osc_on(RCC_OSC_HSI);
		rcc_wait_for_osc_ready(RCC_OSC_HSI);
		rcc_set_sysclk_source(RCC_CFGR0_SW_HSI);
		break;
	case RCC_SYSCLK_HSE_DIRECT:
		rcc_osc_on(RCC_OSC_HSE);
		rcc_wait_for_osc_ready(RCC_OSC_HSE);
		rcc_set_sysclk_source(RCC_CFGR0_SW_HSE);
		break;
	case RCC_SYSCLK_PLL_HSI_48MHZ:
		rcc_clock_setup_hsi_48mhz();
		break;
	case RCC_SYSCLK_PLL_HSE_48MHZ:
		rcc_clock_setup_hse_48mhz();
		break;
	default:
		openwch_assert_not_reached();
		break;
	}
}

/* --- Peripheral clocks --------------------------------------------------- */

void rcc_periph_clock_enable(uint32_t periph)
{
	/*
	 * The three enable registers have the same shape, so the peripheral
	 * identifier carries its bus in the high bits.  This keeps the API to
	 * one function, and the RCC_* constants above stay plain bit masks.
	 */
	switch (periph & RCC_PERIPH_BUS_MASK) {
	case RCC_PERIPH_BUS_APB1:
		RCC_APB1PCENR |= periph & RCC_PERIPH_BIT_MASK;
		break;
	case RCC_PERIPH_BUS_APB2:
		RCC_APB2PCENR |= periph & RCC_PERIPH_BIT_MASK;
		break;
	case RCC_PERIPH_BUS_AHB:
		RCC_AHBPCENR |= periph & RCC_PERIPH_BIT_MASK;
		break;
	default:
		openwch_assert_not_reached();
		break;
	}
}

void rcc_periph_clock_disable(uint32_t periph)
{
	switch (periph & RCC_PERIPH_BUS_MASK) {
	case RCC_PERIPH_BUS_APB1:
		RCC_APB1PCENR &= ~(periph & RCC_PERIPH_BIT_MASK);
		break;
	case RCC_PERIPH_BUS_APB2:
		RCC_APB2PCENR &= ~(periph & RCC_PERIPH_BIT_MASK);
		break;
	case RCC_PERIPH_BUS_AHB:
		RCC_AHBPCENR &= ~(periph & RCC_PERIPH_BIT_MASK);
		break;
	default:
		openwch_assert_not_reached();
		break;
	}
}

void rcc_periph_reset_pulse(uint32_t periph)
{
	rcc_periph_reset_hold(periph);
	rcc_periph_reset_release(periph);
}

void rcc_periph_reset_hold(uint32_t periph)
{
	switch (periph & RCC_PERIPH_BUS_MASK) {
	case RCC_PERIPH_BUS_APB1:
		RCC_APB1PRSTR |= periph & RCC_PERIPH_BIT_MASK;
		break;
	case RCC_PERIPH_BUS_APB2:
		RCC_APB2PRSTR |= periph & RCC_PERIPH_BIT_MASK;
		break;
	default:
		/* The AHB peripherals on this family have no reset line. */
		openwch_assert_not_reached();
		break;
	}
}

void rcc_periph_reset_release(uint32_t periph)
{
	switch (periph & RCC_PERIPH_BUS_MASK) {
	case RCC_PERIPH_BUS_APB1:
		RCC_APB1PRSTR &= ~(periph & RCC_PERIPH_BIT_MASK);
		break;
	case RCC_PERIPH_BUS_APB2:
		RCC_APB2PRSTR &= ~(periph & RCC_PERIPH_BIT_MASK);
		break;
	default:
		openwch_assert_not_reached();
		break;
	}
}

/* --- Clock security system ----------------------------------------------- */

void rcc_clock_security_system_enable(void)
{
	RCC_CTLR |= RCC_CTLR_CSSON;
}

void rcc_clock_security_system_disable(void)
{
	RCC_CTLR &= ~RCC_CTLR_CSSON;
}

/* --- Reset flags --------------------------------------------------------- */

void rcc_clear_reset_flags(void)
{
	RCC_RSTSCKR |= RCC_RSTSCKR_RMVF;
}

uint32_t rcc_get_reset_flags(void)
{
	return RCC_RSTSCKR & (RCC_RSTSCKR_PINRSTF | RCC_RSTSCKR_PORRSTF
			| RCC_RSTSCKR_SFTRSTF | RCC_RSTSCKR_IWDGRSTF
			| RCC_RSTSCKR_WWDGRSTF | RCC_RSTSCKR_LPWRRSTF);
}
/**@}*/
