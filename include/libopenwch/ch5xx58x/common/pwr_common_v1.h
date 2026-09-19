/** @addtogroup pwr_defines PWR Defines

@brief <b>Defined Constants and Types for the CH58x power control</b>

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA PWR.H */

/** @cond */
#if defined(LIBOPENWCH_PWR_H) || defined(LIBOPENWCH_PWR_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_PWR_COMMON_V1_H
#define LIBOPENWCH_PWR_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * Power management on the CH58x
 * ----------------------------
 *
 * The whole power block lives inside the SYS window at SYS_BASE; it is a
 * single instance, so unlike the timer/UART/SPI drivers none of these
 * functions takes a base address.  There is exactly one SYS_BASE.
 *
 * Almost every register here is RWA: a write is silently dropped unless the
 * 0x57/0xA8 safe-access sequence has been issued first.  libopenwch wraps
 * each individual store in its own window through the RWA_* macros, so the
 * implementation of this driver never writes these registers directly.
 *
 * Clock gating
 * ------------
 * The R32_SLEEP_CONTROL bits are CLOCK-OFF flags, not clock-enable flags:
 * setting a bit turns that peripheral's clock OFF and clearing it turns the
 * clock back ON.  The API therefore inverts the sense of the register:
 *
 *	pwr_periph_clock_enable(periph)   CLEARS the bit  (clock on)
 *	pwr_periph_clock_disable(periph)  SETS the bit    (clock off)
 *
 * That inversion is deliberate and matches the reference manual, where every
 * field is named RB_SLP_CLK_<PERIPH> and documented as "close <periph> clock".
 */

/* --- Register accessors -------------------------------------------------- */

/*
 * R32_SLEEP_CONTROL has four byte views: the clock-off gates for the low
 * peripherals live in byte 0, the high peripherals in byte 1, the wake-up
 * sources in byte 2 and the power-down controls in byte 3.
 */
#define PWR_SLEEP_CONTROL	MMIO32(SYS_BASE + 0x0c)
#define PWR_SLP_CLK_OFF0	MMIO8(SYS_BASE + 0x0c)
#define PWR_SLP_CLK_OFF1	MMIO8(SYS_BASE + 0x0d)
#define PWR_SLP_WAKE_CTRL	MMIO8(SYS_BASE + 0x0e)
#define PWR_SLP_POWER_CTRL	MMIO8(SYS_BASE + 0x0f)

/* High-frequency clock module power (R8_HFCK_PWR_CTRL). */
#define PWR_HFCK_PWR_CTRL	MMIO8(SYS_BASE + 0x0a)

/* Power plan applied when the core executes WFI. */
#define PWR_POWER_MANAG		MMIO32(SYS_BASE + 0x20)
#define PWR_POWER_PLAN		MMIO16(SYS_BASE + 0x20)
#define PWR_AUX_POWER_ADJ	MMIO8(SYS_BASE + 0x22)

/* Battery / supply voltage detector. */
#define PWR_BAT_DET_CTRL	MMIO8(SYS_BASE + 0x24)
#define PWR_BAT_DET_CFG		MMIO8(SYS_BASE + 0x25)
#define PWR_BAT_STATUS		MMIO8(SYS_BASE + 0x26)

/* 32 kHz oscillator power and source selection. */
#define PWR_CK32K_CONFIG	MMIO8(SYS_BASE + 0x2f)

/* Flash controller access control, used to park the flash before sleeping. */
#define PWR_FLASH_CTRL		MMIO8(ROM_CTRL_BASE + 0x06)

/* Oscillator tuning and reset control, touched only by the low-power paths. */
#define PWR_XT32M_TUNE		MMIO8(SYS_BASE + 0x4e)
#define PWR_XT32K_TUNE		MMIO8(SYS_BASE + 0x2e)
#define PWR_INT32K_TUNE		MMIO16(SYS_BASE + 0x2c)
#define PWR_RTC_CNT_32K		MMIO16(SYS_BASE + 0x38)
#define PWR_PLL_CONFIG		MMIO8(SYS_BASE + 0x4b)
#define PWR_RST_WDOG_CTRL	MMIO8(SYS_BASE + 0x46)

/* --- Peripheral clock gates (R32_SLEEP_CONTROL) -------------------------- */

/** @defgroup pwr_clk PWR Peripheral Clock Gates
@ingroup pwr_defines

Each bit is a clock-OFF flag in R32_SLEEP_CONTROL (see the note above).
@{
*/
#define PWR_CLK_TMR0		(1u << 0)	/**< TMR0 */
#define PWR_CLK_TMR1		(1u << 1)	/**< TMR1 */
#define PWR_CLK_TMR2		(1u << 2)	/**< TMR2 */
#define PWR_CLK_TMR3		(1u << 3)	/**< TMR3 */
#define PWR_CLK_UART0		(1u << 4)	/**< UART0 */
#define PWR_CLK_UART1		(1u << 5)	/**< UART1 */
#define PWR_CLK_UART2		(1u << 6)	/**< UART2 */
#define PWR_CLK_UART3		(1u << 7)	/**< UART3 */
#define PWR_CLK_SPI0		(1u << 8)	/**< SPI0 */
#define PWR_CLK_SPI1		(1u << 9)	/**< SPI1 */
#define PWR_CLK_PWMX		(1u << 10)	/**< PWMX */
#define PWR_CLK_I2C		(1u << 11)	/**< I2C */
#define PWR_CLK_USB		(1u << 12)	/**< USB */
#define PWR_CLK_USB2		(1u << 13)	/**< USB2 (CH582/CH583 only) */
#define PWR_CLK_BLE		(1u << 15)	/**< BLE */
#define PWR_CLK_RAMX		(1u << 28)	/**< main SRAM */
#define PWR_CLK_RAM2K		(1u << 29)	/**< retention 2 KB SRAM */
/** Every gate the family defines, low half plus the two SRAM bits. */
#define PWR_CLK_ALL		(PWR_CLK_RAMX | PWR_CLK_RAM2K | 0xffffu)
/**@}*/

/* --- Controllable power units -------------------------------------------- */

/** @defgroup pwr_unit PWR Power Units
@ingroup pwr_defines

LSE/LSI are the power bits of R8_CK32K_CONFIG, HSE/PLL those of
R8_HFCK_PWR_CTRL.  They are deliberately kept in one mask space because the
reference API takes them as a single `unit` argument.
@{
*/
#define PWR_UNIT_LSE		(1u << 0)	/**< XT32K crystal */
#define PWR_UNIT_LSI		(1u << 1)	/**< internal 32 kHz RC */
#define PWR_UNIT_HSE		(1u << 2)	/**< XT32M crystal */
#define PWR_UNIT_PLL		(1u << 4)	/**< 480 MHz PLL */
#define PWR_UNIT_MASK		(PWR_UNIT_LSE | PWR_UNIT_LSI | \
				 PWR_UNIT_HSE | PWR_UNIT_PLL)
/**@}*/

/* --- Sleep wake-up sources (R8_SLP_WAKE_CTRL) ---------------------------- */

/** @defgroup pwr_wake PWR Wake-Up Sources
@ingroup pwr_defines
@{
*/
#define PWR_WAKE_USB		(1u << 0)	/**< USB resume */
#define PWR_WAKE_USB2		(1u << 1)	/**< USB2 resume */
#define PWR_WAKE_RTC		(1u << 3)	/**< RTC alarm */
#define PWR_WAKE_GPIO		(1u << 4)	/**< GPIO edge */
#define PWR_WAKE_BAT		(1u << 5)	/**< battery voltage drop */
#define PWR_WAKE_MASK		(PWR_WAKE_USB | PWR_WAKE_USB2 | PWR_WAKE_RTC | \
				 PWR_WAKE_GPIO | PWR_WAKE_BAT)
/** Event-wakeup mode: 1 = keep the event for a long time, 0 = short pulse. */
#define PWR_WAKE_EV_MODE	(1u << 6)
/**@}*/

/** Wake-up delay, as encoded by RB_WAKE_DLY_MOD in R8_SLP_POWER_CTRL. */
typedef enum {
	/** Short delay: RB_WAKE_DLY_MOD = 0b01, 520 cycles + TSUHSE. */
	PWR_WAKEUP_DELAY_SHORT = 0,
	/** Long delay: RB_WAKE_DLY_MOD = 0b00, 3590 cycles + TSUHSE. */
	PWR_WAKEUP_DELAY_LONG = 1,
} pwr_wakeup_delay_t;

/* --- Supply voltage monitor levels --------------------------------------- */

/**
 * Thresholds accepted by pwr_enable_voltage_monitor().  The 0x80 bit selects
 * the low-power comparator (about 1 uA) instead of the high-accuracy one
 * (about 210 uA); the low nibble selects the threshold.
 */
typedef enum {
	PWR_VOLTAGE_MONITOR_1V9		= 0x00,	/**< 1.7 - 1.9 V, high accuracy */
	PWR_VOLTAGE_MONITOR_2V1		= 0x01,	/**< 1.9 - 2.1 V, high accuracy */
	PWR_VOLTAGE_MONITOR_2V3		= 0x02,	/**< 2.1 - 2.3 V, high accuracy */
	PWR_VOLTAGE_MONITOR_2V5		= 0x03,	/**< 2.3 - 2.5 V, high accuracy */

	PWR_VOLTAGE_MONITOR_1V8_LP	= 0x80,	/**< 1.8 V, low power */
	PWR_VOLTAGE_MONITOR_1V9_LP	= 0x81,	/**< 1.9 V, low power */
	PWR_VOLTAGE_MONITOR_2V0_LP	= 0x82,	/**< 2.0 V, low power */
	PWR_VOLTAGE_MONITOR_2V1_LP	= 0x83,	/**< 2.1 V, low power */
	PWR_VOLTAGE_MONITOR_2V2_LP	= 0x84,	/**< 2.2 V, low power */
	PWR_VOLTAGE_MONITOR_2V3_LP	= 0x85,	/**< 2.3 V, low power */
	PWR_VOLTAGE_MONITOR_2V4_LP	= 0x86,	/**< 2.4 V, low power */
	PWR_VOLTAGE_MONITOR_2V5_LP	= 0x87,	/**< 2.5 V, low power */
} pwr_voltage_monitor_t;

/* --- Battery detector bits ----------------------------------------------- */

/** @defgroup pwr_bat PWR Battery Detector Bits
@ingroup pwr_defines
@{
*/
#define PWR_BAT_DET_EN		(1u << 0)	/**< enable detector */
#define PWR_BAT_LOW_VTHX	(1u << 0)	/**< threshold select, monitor mode */
#define PWR_BAT_MON_EN		(1u << 1)	/**< enable monitor during sleep */
#define PWR_BAT_LOWER_IE	(1u << 2)	/**< lower-voltage interrupt */
#define PWR_BAT_LOW_IE		(1u << 3)	/**< low-voltage interrupt */
/** Threshold field of R8_BAT_DET_CFG. */
#define PWR_BAT_LOW_VTH_MASK	0x03u
/**@}*/

/* --- Retention mask for the low-power entry points ----------------------- */

/** @defgroup pwr_retain PWR Retention Units
@ingroup pwr_defines

Bits OR-ed into R16_POWER_PLAN to keep a supply domain alive across sleep or
shutdown.  Passing 0 powers everything down.
@{
*/
#define PWR_RETAIN_XROM		(1u << 0)	/**< flash ROM */
#define PWR_RETAIN_RAM2K	(1u << 1)	/**< retention 2 KB SRAM */
#define PWR_RETAIN_CORE		(1u << 2)	/**< core and base peripherals */
#define PWR_RETAIN_EXTEND	(1u << 3)	/**< USB and BLE */
#define PWR_RETAIN_RAM30K	(1u << 4)	/**< main SRAM */
#define PWR_RETAIN_MASK		(0x1fu)
/**@}*/

/* --- Power plan and auxiliary power bits --------------------------------- */

/** @defgroup pwr_plan PWR Power Plan Bits
@ingroup pwr_defines
@{
*/
#define PWR_DCDC_EN		(1u << 9)	/**< DC/DC converter enable */
#define PWR_DCDC_PRE		(1u << 10)	/**< DC/DC pre-enable */
#define PWR_PLAN_MUST_0010	(1u << 12)	/**< reserved, must be 1 */
#define PWR_PLAN_EN		(1u << 15)	/**< apply the plan on WFI */

#define PWR_DCDC_CHARGE		(1u << 7)	/**< DC/DC auxiliary charge */
/** Ultra-low-power LDO adjust field of R16_AUX_POWER_ADJ. */
#define PWR_ULPLDO_ADJ_MASK	0x07u

/** R8_RST_WDOG_CTRL.RB_SOFTWARE_RESET: request a global software reset. */
#define PWR_SOFTWARE_RESET	(1u << 0)

/** RAM retention voltage: 1 = low voltage for low power. */
#define PWR_SLP_RAM_RET_LV	(1u << 6)
/**@}*/

/* --- Flag bits ----------------------------------------------------------- */

/** @defgroup pwr_flag PWR Flag Bits
@ingroup pwr_defines

Read back from R8_BAT_STATUS.  Both are high-active.
@{
*/
#define PWR_FLAG_BAT_LOWER	(1u << 0)	/**< lower voltage detected */
#define PWR_FLAG_BAT_LOW	(1u << 1)	/**< low voltage detected */
#define PWR_FLAG_MASK		(PWR_FLAG_BAT_LOWER | PWR_FLAG_BAT_LOW)
/**@}*/

BEGIN_DECLS

/* --- DC/DC converter ----------------------------------------------------- */

/** Enable the internal DC/DC converter to save power. */
void pwr_enable_dcdc(void);
/** Disable the DC/DC converter and fall back to the LDO. */
void pwr_disable_dcdc(void);

/* --- Clock units --------------------------------------------------------- */

/**
 * Power `unit` (one or more PWR_UNIT_* bits) up or down.
 * @param enable true to power on, false to power off.
 * @param unit PWR_UNIT_LSE / LSI / HSE / PLL.
 */
void pwr_set_unit(bool enable, uint32_t unit);

/* --- Peripheral clocks --------------------------------------------------- */

/**
 * Ungate a peripheral clock.  This CLEARS the clock-off bit in
 * R32_SLEEP_CONTROL, because those bits are clock-off flags.
 */
void pwr_periph_clock_enable(uint32_t periph);
/**
 * Gate a peripheral clock.  This SETS the clock-off bit in
 * R32_SLEEP_CONTROL.
 */
void pwr_periph_clock_disable(uint32_t periph);

/* --- Sleep wake-up sources ----------------------------------------------- */

/**
 * Enable or disable a peripheral as a sleep wake-up source.
 * @param enable true to arm the source, false to disarm it.
 * @param periph one or more PWR_WAKE_* bits.
 * @param mode wake-up delay, only used when enabling.
 */
void pwr_set_wakeup(bool enable, uint32_t periph, pwr_wakeup_delay_t mode);

/* --- Supply voltage monitor ---------------------------------------------- */

/** Arm the battery/supply voltage monitor at the given threshold. */
void pwr_enable_voltage_monitor(pwr_voltage_monitor_t level);
/** Disarm the battery/supply voltage monitor. */
void pwr_disable_voltage_monitor(void);

/* --- Flags --------------------------------------------------------------- */

/** True if any of the requested PWR_FLAG_* bits is set in R8_BAT_STATUS. */
bool pwr_get_flag(uint32_t flag);

/* --- Low-power entry points ---------------------------------------------- */

/*
 * These four functions power the flash down while the core sleeps, so their
 * code cannot be fetched from flash.  Every definition carries
 * OPENWCH_HIGH_CODE and is placed in .highcode, which the linker script
 * copies into RAM before main().  No flash-resident helper may be called from
 * them after the flash has been parked.
 */

/** Enter idle: gate the core clock only, everything stays powered. */
void pwr_enter_idle(void);
/** Enter halt: 32 kHz domain stays alive, the 32 MHz / PLL path stops. */
void pwr_enter_halt(void);

/**
 * Enter sleep with the given retention mask (PWR_RETAIN_*).  DCDC operation
 * is suspended for the duration and the system clock must be re-selected by
 * the caller after wake-up.
 */
void pwr_enter_sleep(uint8_t rm);

/**
 * Enter shutdown with the given retention mask (PWR_RETAIN_*).  As on the
 * reference part this ends in a software reset, so the function does not
 * return to its caller.
 */
void pwr_enter_shutdown(uint8_t rm);

END_DECLS

#endif
/** @cond */
#else
#warning "pwr_common_v1.h should not be included explicitly, only via pwr.h"
#endif
/** @endcond */
/**@}*/
