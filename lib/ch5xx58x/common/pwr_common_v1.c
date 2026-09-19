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

/** @addtogroup pwr_file PWR
 *
 * @ingroup CH5XX58X
 *
 * @brief <b>Power control for the CH58x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * Every register written here is RWA, so each individual store goes through
 * its own safe-access window via the RWA_* macros.  The vendor driver writes
 * pairs of registers inside one window; that is not legal here because the
 * window is only about sixteen system clocks wide, so each register gets its
 * own RWA_* call.
 *
 * The four low-power entry points are tagged OPENWCH_HIGH_CODE.  That is not
 * an optimisation: they park the flash controller and then execute WFI, so
 * the instruction stream that follows the wake-up event -- and, for shutdown,
 * the final reset sequence -- must already be in RAM.  The linker script
 * places .highcode in the main SRAM and copies it there at startup.
 *
 * Two vendor steps are deliberately re-expressed rather than called:
 * FLASH_ROM_SW_RESET() and GetMACAddress() are entries into the mask-ROM ISP
 * routine (FLASH_EEPROM_CMD) that libopenwch does not own.  The flash is
 * parked by writing R8_FLASH_CTRL directly, and the post-wake-up readiness
 * poll reads the ROM config word at ROM_CFG_MAC_ADDR from the memory map.
 * The remainder of the sequences follows the vendor driver step for step.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch5xx58x/pwr.h>
#include <libopenwch/ch5xx58x/clk.h>
#include <libopenwch/ch5xx58x/rwa.h>
#include <libopenwch/qingke/assert.h>
#include <libopenwch/qingke/pfic.h>
#include <libopenwch/qingke/systick.h>

/* --- Field encodings used only by the low-power paths -------------------- */

/* R8_XT32M_TUNE.RB_XT32M_I_BIAS: 0b11 = 150% of the nominal bias current. */
#define PWR_XT32M_I_BIAS_MASK		0x03u
#define PWR_XT32M_I_BIAS_150		0x03u

/* R8_XT32K_TUNE.RB_XT32K_I_TUNE: 0b01 = standard current. */
#define PWR_XT32K_I_TUNE_MASK		0x03u
#define PWR_XT32K_I_TUNE_STANDARD	0x01u

/* R8_PLL_CONFIG.RB_PLL_HALT: 1 = PLL halted for low power. */
#define PWR_PLL_HALT			(1u << 5)

/* R8_FLASH_CTRL value that parks the flash controller, as used by the EVT. */
#define PWR_FLASH_PWR_DOWN		0x04u

/* R16_RTC_CNT_32K above this count means more than about 500 ms of RTC
 * history, at which point the LSE can drop back to its standard drive. */
#define PWR_RTC_32K_500MS		0x3fffu

/** The 32 kHz countdown register value that disables the internal RC trim. */
#define PWR_INT32K_TUNE_OFF		0xffffu

/*
 * Wait for an event.  always_inline is required, not a hint: the call sites
 * are in .highcode and must not jump into flash while the flash is parked.
 */
__attribute__((always_inline))
static inline void pwr_wfi(void) {
	__asm__ volatile ("wfi");
}

/* Masks for the two R8_SLP_* registers the wake-up code touches. */
#define PWR_SLP_WAKE_DLY_MASK		0x03u

/* --- DC/DC converter ----------------------------------------------------- */

void pwr_enable_dcdc(void) {
	RWA_SET_BITS(PWR_AUX_POWER_ADJ, PWR_DCDC_CHARGE);
	RWA_SET_BITS(PWR_POWER_PLAN, PWR_DCDC_PRE);

	/* Give the auxiliary charge pump time to come up before enabling. */
	qingke_delay_us(10);

	RWA_SET_BITS(PWR_POWER_PLAN, PWR_DCDC_EN);
}

void pwr_disable_dcdc(void) {
	RWA_CLEAR_BITS(PWR_POWER_PLAN, PWR_DCDC_EN | PWR_DCDC_PRE);
	RWA_CLEAR_BITS(PWR_AUX_POWER_ADJ, PWR_DCDC_CHARGE);
}

/* --- Clock units --------------------------------------------------------- */

void pwr_set_unit(bool enable, uint32_t unit) {
	uint8_t hfck = PWR_HFCK_PWR_CTRL;
	uint8_t ck32k = PWR_CK32K_CONFIG;
	uint32_t hfck_unit = unit & (PWR_UNIT_HSE | PWR_UNIT_PLL);
	uint32_t ck32k_unit = unit & (PWR_UNIT_LSE | PWR_UNIT_LSI);

	openwch_assert((unit & ~PWR_UNIT_MASK) == 0);

	if (enable) {
		hfck |= (uint8_t)hfck_unit;
		ck32k |= (uint8_t)ck32k_unit;
	} else {
		hfck &= (uint8_t)~hfck_unit;
		ck32k &= (uint8_t)~ck32k_unit;
	}

	/*
	 * The HSE/PLL power bits and the LSE/LSI power bits live in two
	 * different registers, so they need two independent windows.
	 */
	RWA_WRITE8(PWR_HFCK_PWR_CTRL, hfck);
	RWA_WRITE8(PWR_CK32K_CONFIG, ck32k);
}

/* --- Peripheral clocks --------------------------------------------------- */

/*
 * R32_SLEEP_CONTROL is a set of clock-OFF gates: a set bit disables the
 * peripheral clock.  The API names below are from the caller's point of view,
 * so the enable path clears the bit and the disable path sets it.
 */
void pwr_periph_clock_enable(uint32_t periph) {
	openwch_assert((periph & ~PWR_CLK_ALL) == 0);

	RWA_CLEAR_BITS(PWR_SLEEP_CONTROL, periph);
}

void pwr_periph_clock_disable(uint32_t periph) {
	openwch_assert((periph & ~PWR_CLK_ALL) == 0);

	RWA_SET_BITS(PWR_SLEEP_CONTROL, periph);
}

/* --- Sleep wake-up sources ----------------------------------------------- */

void pwr_set_wakeup(bool enable, uint32_t periph, pwr_wakeup_delay_t mode) {
	openwch_assert((periph & ~PWR_WAKE_MASK) == 0);
	openwch_assert(mode <= PWR_WAKEUP_DELAY_LONG);

	if (!enable) {
		RWA_CLEAR_BITS(PWR_SLP_WAKE_CTRL, periph);
		return;
	}

	/*
	 * RB_WAKE_DLY_MOD is encoded the other way round from the reference
	 * enum: the "short delay" selection is 0b01 and "long delay" is
	 * 0b00.  Mirror the vendor mapping here so the public enum keeps the
	 * vendor's ordering.
	 */
	RWA_SET_BITS(PWR_SLP_WAKE_CTRL, PWR_WAKE_EV_MODE | periph);
	RWA_MODIFY(PWR_SLP_POWER_CTRL, PWR_SLP_WAKE_DLY_MASK,
		   (mode == PWR_WAKEUP_DELAY_LONG) ? 0x00u : 0x01u);
}

/* --- Supply voltage monitor ---------------------------------------------- */

void pwr_enable_voltage_monitor(pwr_voltage_monitor_t level) {
	uint8_t cfg = (uint8_t)level & PWR_BAT_LOW_VTH_MASK;
	uint8_t ctrl;

	openwch_assert(level <= PWR_VOLTAGE_MONITOR_2V5_LP);

	if ((uint8_t)level & 0x80u) {
		/* Low-power monitor; bit 2 selects the higher threshold. */
		ctrl = PWR_BAT_MON_EN | ((((uint8_t)level) >> 2) & 0x01u);
	} else {
		/* High-accuracy detector. */
		ctrl = PWR_BAT_DET_EN;
	}

	RWA_WRITE8(PWR_BAT_DET_CTRL, ctrl);
	RWA_WRITE8(PWR_BAT_DET_CFG, cfg);

	qingke_delay_us(1);

	RWA_SET_BITS(PWR_BAT_DET_CTRL, PWR_BAT_LOWER_IE | PWR_BAT_LOW_IE);
}

void pwr_disable_voltage_monitor(void) {
	RWA_WRITE8(PWR_BAT_DET_CTRL, 0);
}

/* --- Flags --------------------------------------------------------------- */

bool pwr_get_flag(uint32_t flag) {
	openwch_assert((flag & ~PWR_FLAG_MASK) == 0);
	openwch_assert(flag != 0);

	return (PWR_BAT_STATUS & (uint8_t)flag) != 0;
}

/* --- Low-power entry points ---------------------------------------------- */

OPENWCH_HIGH_CODE
void pwr_enter_idle(void) {
	/* Park the flash; the sleep controller brings it back on wake-up. */
	PWR_FLASH_CTRL = PWR_FLASH_PWR_DOWN;

	/* Clear SLEEPDEEP: this is a plain idle, not a deep sleep. */
	PFIC->sctlr &= ~PFIC_SCTLR_SLEEPDEEP;

	pwr_wfi();
	qingke_nop();
	qingke_nop();
}

OPENWCH_HIGH_CODE
void pwr_enter_halt(void) {
	uint8_t x32k_tune = PWR_XT32K_TUNE;
	uint8_t x32m_tune = PWR_XT32M_TUNE;

	PWR_FLASH_CTRL = PWR_FLASH_PWR_DOWN;

	/* Boost the 32 MHz bias current so the crystal restarts quickly. */
	x32m_tune = (uint8_t)((x32m_tune & ~PWR_XT32M_I_BIAS_MASK) |
			      PWR_XT32M_I_BIAS_150);
	if (PWR_RTC_CNT_32K > PWR_RTC_32K_500MS) {
		/* More than 500 ms of history: standard LSE drive is enough. */
		x32k_tune = (uint8_t)((x32k_tune & ~PWR_XT32K_I_TUNE_MASK) |
				      PWR_XT32K_I_TUNE_STANDARD);
	}

	/* The voltage monitor is not usable in halt. */
	RWA_WRITE8(PWR_BAT_DET_CTRL, 0);
	RWA_WRITE8(PWR_XT32K_TUNE, x32k_tune);
	RWA_WRITE8(PWR_XT32M_TUNE, x32m_tune);
	RWA_SET_BITS(PWR_PLL_CONFIG, PWR_PLL_HALT);

	PFIC->sctlr |= PFIC_SCTLR_SLEEPDEEP;

	pwr_wfi();
	qingke_nop();
	qingke_nop();

	RWA_CLEAR_BITS(PWR_PLL_CONFIG, PWR_PLL_HALT);
}

OPENWCH_HIGH_CODE
void pwr_enter_sleep(uint8_t rm) {
	uint32_t mac_before;
	uint32_t mac_now;
	uint16_t plan;
	uint8_t x32m_tune;

	openwch_assert((rm & ~PWR_RETAIN_MASK) == 0);

	/*
	 * The flash is powered down for the duration.  Polling the ROM config
	 * word is how the vendor detects that it is readable again after the
	 * wake-up delay; a protected or absent MAC reads as the same value
	 * before and after, so the loop still terminates.
	 */
	mac_before = MMIO32(ROM_CFG_MAC_ADDR);

	x32m_tune = (uint8_t)((PWR_XT32M_TUNE & ~PWR_XT32M_I_BIAS_MASK) |
			      PWR_XT32M_I_BIAS_150);

	RWA_WRITE8(PWR_BAT_DET_CTRL, 0);
	RWA_WRITE8(PWR_XT32M_TUNE, x32m_tune);

	PFIC->sctlr |= PFIC_SCTLR_SLEEPDEEP;

	/* Keep the DC/DC setting, force the plan bits and the retention mask. */
	plan = (uint16_t)(PWR_POWER_PLAN & (PWR_DCDC_EN | PWR_DCDC_PRE));
	plan = (uint16_t)(plan | PWR_PLAN_EN | PWR_PLAN_MUST_0010 |
			  PWR_RETAIN_CORE | rm);
	qingke_nop();

	RWA_SET_BITS(PWR_SLP_POWER_CTRL, PWR_SLP_RAM_RET_LV);
	RWA_SET_BITS(PWR_PLL_CONFIG, PWR_PLL_HALT);
	RWA_WRITE(PWR_POWER_PLAN, plan);

	do {
		pwr_wfi();
		qingke_nop();
		qingke_nop();
		qingke_delay_us(300);
		mac_now = MMIO32(ROM_CFG_MAC_ADDR);
	} while (mac_now != mac_before);

	RWA_CLEAR_BITS(PWR_POWER_PLAN, PWR_PLAN_EN);
	RWA_CLEAR_BITS(PWR_PLL_CONFIG, PWR_PLL_HALT);
	qingke_delay_us(20);
}

OPENWCH_HIGH_CODE
void pwr_enter_shutdown(uint8_t rm) {
	uint8_t x32k_tune = PWR_XT32K_TUNE;
	uint8_t x32m_tune = PWR_XT32M_TUNE;

	openwch_assert((rm & ~PWR_RETAIN_MASK) == 0);

	x32m_tune = (uint8_t)((x32m_tune & ~PWR_XT32M_I_BIAS_MASK) |
			      PWR_XT32M_I_BIAS_150);
	if (PWR_RTC_CNT_32K > PWR_RTC_32K_500MS) {
		x32k_tune = (uint8_t)((x32k_tune & ~PWR_XT32K_I_TUNE_MASK) |
				      PWR_XT32K_I_TUNE_STANDARD);
	}

	RWA_WRITE8(PWR_BAT_DET_CTRL, 0);
	RWA_WRITE8(PWR_XT32K_TUNE, x32k_tune);
	RWA_WRITE8(PWR_XT32M_TUNE, x32m_tune);

	/*
	 * Drop to 6.4 MHz before removing power.  This runs from flash, which
	 * is still powered at this point; only the WFI below parks it.
	 */
	clk_set_sys_clock(CLK_SOURCE_HSE_6_4MHZ);

	PFIC->sctlr |= PFIC_SCTLR_SLEEPDEEP;

	RWA_SET_BITS(PWR_SLP_POWER_CTRL, PWR_SLP_RAM_RET_LV);
	RWA_WRITE(PWR_POWER_PLAN,
		  (uint16_t)(PWR_PLAN_EN | PWR_PLAN_MUST_0010 | rm));

	pwr_wfi();
	qingke_nop();
	qingke_nop();

	/*
	 * The core comes back from shutdown through a full reset, which also
	 * clears the internal 32 kHz trim, so restore it and request the
	 * software reset explicitly.  The reset takes effect before this
	 * function can return.
	 */
	RWA_WRITE(PWR_INT32K_TUNE, PWR_INT32K_TUNE_OFF);
	RWA_SET_BITS(PWR_RST_WDOG_CTRL, PWR_SOFTWARE_RESET);
}
/**@}*/
