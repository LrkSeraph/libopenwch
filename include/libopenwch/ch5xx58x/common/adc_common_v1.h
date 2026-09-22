/** @addtogroup adc_defines ADC Defines

@brief <b>Defined Constants and Types for the CH58x ADC and touch-key block</b>

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA ADC.H */

/** @cond */
#if defined(LIBOPENWCH_ADC_H) || defined(LIBOPENWCH_ADC_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_ADC_COMMON_V1_H
#define LIBOPENWCH_ADC_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * The CH58x has a single 12-bit SAR ADC and a touch-key charge/discharge
 * counter, both in the SYS window at SYS_BASE.  There is no ADC instance
 * number: every register is addressed by one base plus a byte offset, which is
 * why the accessors below take the peripheral base as their first argument
 * even though there is only one instance.
 *
 * The block has two data paths:
 *
 *   - single conversions, started by writing RB_ADC_START to R8_ADC_CONVERT
 *     and read back from R16_ADC_DATA.  R8_ADC_INT_FLAG.RB_ADC_IF_EOC reports
 *     the end of conversion.
 *   - an auto/continuous mode that walks a DMA address window
 *     (R16_ADC_DMA_BEG .. R16_ADC_DMA_END) and writes each result into RAM.
 *
 * Analog inputs must have their digital input buffer disabled first through
 * R16_PIN_ANALOG_IE (see gpio_analog_enable()), or the reading will be pulled
 * around by the digital input.
 *
 * NOTE ON RWA: the CH583 SFR header labels R8_ADC_CFG, R8_ADC_CONVERT and
 * R8_TKEY_CFG as plain RW, while later CH584/CH585 documentation marks the
 * same three registers RWA (writes ignored unless the safe-access window is
 * open).  libopenwch writes them through the RWA_* macros unconditionally:
 * that is required on the parts that protect them and harmless on the parts
 * that do not.  Every other ADC/touch register is a plain read/write, and
 * R16_ADC_DATA is a plain read.
 */

/* --- Register accessors -------------------------------------------------- */

/* R32_TKEY_CTRL / R8_TKEY_COUNT (0x54) */
#define ADC_TKEY_CTRL(adc) MMIO32((adc) + 0x54)
#define ADC_TKEY_COUNT(adc) MMIO8((adc) + 0x54)
/* R8_TKEY_CONVERT (0x56) */
#define ADC_TKEY_CONVERT(adc) MMIO8((adc) + 0x56)
/* R8_TKEY_CFG (0x57) -- RWA */
#define ADC_TKEY_CFG(adc) MMIO8((adc) + 0x57)

/* R32_ADC_CTRL / R8_ADC_CHANNEL (0x58) */
#define ADC_CTRL(adc) MMIO32((adc) + 0x58)
#define ADC_CHANNEL(adc) MMIO8((adc) + 0x58)
/* R8_ADC_CFG (0x59) -- RWA */
#define ADC_CFG(adc) MMIO8((adc) + 0x59)
/* R8_ADC_CONVERT (0x5A) -- RWA */
#define ADC_CONVERT(adc) MMIO8((adc) + 0x5a)
/* R8_TEM_SENSOR (0x5B) */
#define ADC_TEM_SENSOR(adc) MMIO8((adc) + 0x5b)
/* R32_ADC_DATA / R16_ADC_DATA (0x5C), read only */
#define ADC_DATA32(adc) MMIO32((adc) + 0x5c)
#define ADC_DATA(adc) MMIO16((adc) + 0x5c)
/* R8_ADC_INT_FLAG (0x5E), read only */
#define ADC_INT_FLAG(adc) MMIO8((adc) + 0x5e)

/* R32_ADC_DMA_CTRL / R8_ADC_CTRL_DMA (0x60/0x61) */
#define ADC_DMA_CTRL32(adc) MMIO32((adc) + 0x60)
#define ADC_DMA_CTRL(adc) MMIO8((adc) + 0x61)
/* R8_ADC_DMA_IF (0x62), write 1 to clear */
#define ADC_DMA_IF(adc) MMIO8((adc) + 0x62)
/* R8_ADC_AUTO_CYCLE (0x63) */
#define ADC_AUTO_CYCLE(adc) MMIO8((adc) + 0x63)
/* R32_ADC_DMA_NOW / R16_ADC_DMA_NOW (0x64) */
#define ADC_DMA_NOW32(adc) MMIO32((adc) + 0x64)
#define ADC_DMA_NOW(adc) MMIO16((adc) + 0x64)
/* R32_ADC_DMA_BEG / R16_ADC_DMA_BEG (0x68) */
#define ADC_DMA_BEG32(adc) MMIO32((adc) + 0x68)
#define ADC_DMA_BEG(adc) MMIO16((adc) + 0x68)
/* R32_ADC_DMA_END / R16_ADC_DMA_END (0x6C) */
#define ADC_DMA_END32(adc) MMIO32((adc) + 0x6c)
#define ADC_DMA_END(adc) MMIO16((adc) + 0x6c)

/*
 * R32_ADC_SCAN_CFG1 (0x70), R32_ADC_SCAN_CFG2 (0x74) and R32_TKEY_SEL (0x7C)
 * only exist on the CH584/CH585.  On the CH582/CH583 these addresses are
 * reserved and the registers must not be touched.  The field layout below is
 * the CH584/585 one; the CH583 SFR header predates the scan engine.
 */
#define ADC_SCAN_CFG1(adc) MMIO32((adc) + 0x70)
#define ADC_SCAN_CFG2(adc) MMIO32((adc) + 0x74)
#define ADC_TKEY_SEL(adc) MMIO32((adc) + 0x7c)

/* --- R8_ADC_CHANNEL bits ------------------------------------------------- */
#define RB_ADC_CH_INX 0x0f /**< channel index, CH583 */

/* --- R8_ADC_CFG bits ----------------------------------------------------- */
#define RB_ADC_POWER_ON 0x01 /**< ADC power control */
#define RB_ADC_BUF_EN 0x02   /**< input buffer enable */
#define RB_ADC_DIFF_EN 0x04  /**< 1 = differential input pair */
#define RB_ADC_OFS_TEST 0x08 /**< short the input to test offset */
#define RB_ADC_PGA_GAIN 0x30 /**< PGA gain field */
#define RB_ADC_CLK_DIV 0xc0  /**< sample clock field */

#define ADC_PGA_GAIN_SHIFT 4
#define ADC_CLK_DIV_SHIFT 6

/* --- R8_ADC_CONVERT bits ------------------------------------------------- */
#define RB_ADC_START 0x01 /**< start one conversion, auto clear */
#define RB_ADC_EOC_X 0x80 /**< end-of-conversion, read only */

/* --- R8_TEM_SENSOR bits -------------------------------------------------- */
#define RB_TEM_SEN_PWR_ON 0x80 /**< temperature sensor power on */

/* --- R16_ADC_DATA -------------------------------------------------------- */
#define RB_ADC_DATA 0x0fff /**< 12-bit conversion result */

/* --- R8_ADC_INT_FLAG / R8_ADC_DMA_IF bits -------------------------------- */
#define RB_ADC_IF_EOC 0x80     /**< end of conversion */
#define RB_ADC_IF_DMA_END 0x08 /**< DMA window complete */
#define RB_ADC_IF_END_ADC 0x10 /**< auto/continuous conversion end */

/* --- R8_TKEY_COUNT bits -------------------------------------------------- */
#define RB_TKEY_CHARG_CNT 0x1f /**< charge time, in ADC clocks */
#define RB_TKEY_DISCH_CNT 0xe0 /**< discharge time, in ADC clocks */
#define ADC_TKEY_DISCH_SHIFT 5

/* --- R8_TKEY_CONVERT bits ------------------------------------------------ */
#define RB_TKEY_START 0x01 /**< start one touch conversion */

/* --- R8_TKEY_CFG bits ---------------------------------------------------- */
#define RB_TKEY_PWR_ON 0x01  /**< touch-key power on */
#define RB_TKEY_CURRENT 0x02 /**< 0 = 35 uA, 1 = 70 uA */
#define RB_TKEY_DRV_EN 0x04  /**< drive-shield enable */
#define RB_TKEY_PGA_ADJ 0x08 /**< ADC PGA speed: 0 = slow */

/* --- R8_ADC_CTRL_DMA bits ------------------------------------------------ */
#define RB_ADC_DMA_ENABLE 0x01 /**< DMA window enable */
#define RB_ADC_DMA_LOOP 0x04   /**< wrap at the end address */
#define RB_ADC_IE_DMA_END 0x08 /**< DMA-complete interrupt enable */
#define RB_ADC_IE_EOC 0x10     /**< end-of-conversion int enable */
#define RB_ADC_CONT_EN 0x40    /**< continuous conversion */
#define RB_ADC_AUTO_EN 0x80    /**< auto conversion for DMA */

/* --- CH584/585 scan fields ----------------------------------------------- */

/**
 * One 4-bit scan channel field in R32_ADC_SCAN_CFG1/2.  Fields are packed
 * eight to a register; the field for channel n starts at bit
 * n * ADC_SCAN_CH_STRIDE.
 */
#define RB_ADC_SCAN_CH 0x0000000fu
#define ADC_SCAN_CH_STRIDE 4
/** Number of channels to scan, in R32_ADC_SCAN_CFG2 bits 27:24. */
#define RB_ADC_SCAN_NUM 0x0f000000
#define ADC_SCAN_NUM_SHIFT 24
/** 0 = the scan engine walks touch keys, 1 = it walks ADC channels. */
#define RB_ADC_SCAN_SEL 0x10000000
#define RB_ADC_IE_SCAN_END 0x20000000
#define RB_ADC_SCAN_MASK_DIS 0x40000000
#define RB_ADC_IF_SCAN_END 0x80000000

/** Touch-key multi-drive shield outputs, R32_TKEY_SEL bits 31:18. */
#define RB_TKEY_DRV_OUTEN 0xfffc0000u

/* --- Configuration identifiers ------------------------------------------- */

/** @defgroup adc_channel ADC Input Channels
@ingroup adc_defines

CH_EXTIN0..CH_EXTIN13 are the external analog pins, in the order the GPIO
analog-enable bits are numbered.  ADC_CH_VBAT and ADC_CH_VTEMP are the two
internal sources.

@{*/
typedef enum {
	ADC_CH_EXTIN0 = 0, /**< external channel 0 */
	ADC_CH_EXTIN1,	   /**< external channel 1 */
	ADC_CH_EXTIN2,	   /**< external channel 2 */
	ADC_CH_EXTIN3,	   /**< external channel 3 */
	ADC_CH_EXTIN4,	   /**< external channel 4 */
	ADC_CH_EXTIN5,	   /**< external channel 5 */
	ADC_CH_EXTIN6,	   /**< external channel 6 */
	ADC_CH_EXTIN7,	   /**< external channel 7 */
	ADC_CH_EXTIN8,	   /**< external channel 8 */
	ADC_CH_EXTIN9,	   /**< external channel 9 */
	ADC_CH_EXTIN10,	   /**< external channel 10 */
	ADC_CH_EXTIN11,	   /**< external channel 11 */
	ADC_CH_EXTIN12,	   /**< external channel 12 */
	ADC_CH_EXTIN13,	   /**< external channel 13 */
	ADC_CH_VBAT = 14,  /**< internal battery monitor */
	ADC_CH_VTEMP = 15, /**< internal temperature sensor */
} adc_channel_t;
/**@}*/

/** @defgroup adc_sample_clk ADC Sample Clock
@ingroup adc_defines

The ADC clock, written to RB_ADC_CLK_DIV.  It is divided down from the system
clock, so the selection is only nominal once the system clock changes.

@{*/
typedef enum {
	ADC_SAMPLE_CLK_3_2MHZ = 0, /**< 0b00, 3.2 MHz */
	ADC_SAMPLE_CLK_8MHZ,	   /**< 0b01, 8 MHz */
	ADC_SAMPLE_CLK_5_33MHZ,	   /**< 0b10, 5.33 MHz */
	ADC_SAMPLE_CLK_4MHZ,	   /**< 0b11, 4 MHz */
} adc_sample_clk_t;
/**@}*/

/** @defgroup adc_pga ADC Programmable Gain
@ingroup adc_defines

Input PGA gain, written to RB_ADC_PGA_GAIN.

@{*/
typedef enum {
	ADC_PGA_1_4 = 0, /**< -12 dB, 1/4x */
	ADC_PGA_1_2,	 /**< -6 dB, 1/2x */
	ADC_PGA_1,	 /**< 0 dB, unity */
	ADC_PGA_2,	 /**< +6 dB, 2x */
} adc_pga_t;
/**@}*/

/** @defgroup adc_dma_mode ADC DMA Modes
@ingroup adc_defines

@{*/
typedef enum {
	ADC_DMA_MODE_SINGLE = 0, /**< stop at the end address */
	ADC_DMA_MODE_LOOP,	 /**< wrap back to the begin address */
} adc_dma_mode_t;
/**@}*/

BEGIN_DECLS

/* --- Initialisation ------------------------------------------------------ */

/** Set up an external single-ended channel: power, input buffer, clock, PGA. */
void adc_init_single_channel(uint32_t adc,
			     adc_sample_clk_t sample_clk,
			     adc_pga_t pga);
/** Set up one differential input pair: power, differential mode, clock, PGA. */
void adc_init_differential(uint32_t adc,
			   adc_sample_clk_t sample_clk,
			   adc_pga_t pga);
/** Power the internal temperature sensor and select ADC_CH_VTEMP. */
void adc_init_temperature(uint32_t adc);
/** Select ADC_CH_VBAT with the fixed 1/4x divider the battery rail needs. */
void adc_init_battery(uint32_t adc);

/* --- Configuration ------------------------------------------------------- */

void adc_set_channel(uint32_t adc, adc_channel_t channel);
void adc_set_sample_clock(uint32_t adc, adc_sample_clk_t clk);
void adc_set_pga(uint32_t adc, adc_pga_t pga);

/* --- Single conversion --------------------------------------------------- */

/** Start one conversion.  Poll adc_is_ready() before calling adc_read(). */
void adc_start(uint32_t adc);
/** True once the conversion started by adc_start() has finished. */
bool adc_is_ready(uint32_t adc);
/** Read the last conversion result; does not start a conversion. */
uint16_t adc_read(uint32_t adc);

/* --- Auto conversion and DMA --------------------------------------------- */

/** Set the automatic conversion period, in units of 16 system clocks. */
void adc_set_auto_cycle(uint32_t adc, uint8_t cycles);
/** Enable the DMA window and the auto-conversion engine that fills it. */
void adc_enable_dma(uint32_t adc,
		    adc_dma_mode_t mode,
		    uint16_t start,
		    uint16_t end);
void adc_disable_dma(uint32_t adc);

/* --- Touch key ----------------------------------------------------------- */

/** Power the touch-key block and configure the ADC for it. */
void adc_enable_touchkey(uint32_t adc);
void adc_disable_touchkey(uint32_t adc);
/**
 * Run one touch-key conversion.
 *
 * @param adc         the ADC base address
 * @param charge      charge time, 5 bits, in ADC clocks
 * @param discharge   discharge time, 3 bits, in ADC clocks
 * @return            the equivalent touch-key data value
 */
uint16_t adc_read_touchkey(uint32_t adc, uint8_t charge, uint8_t discharge);

/* --- Conversion helpers -------------------------------------------------- */

/**
 * Convert a raw temperature-sensor reading to degrees Celsius, using the
 * factory calibration word at ROM_CFG_TMP_25C.
 */
int adc_to_celsius(uint16_t raw);

END_DECLS

#endif
/** @cond */
#else
#warning "adc_common_v1.h should not be included explicitly, only via adc.h"
#endif
/** @endcond */
/**@}*/
