/** @addtogroup adc_defines ADC Defines

@brief <b>Defined Constants and Types for the CH32V00x ADC</b>

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA ADC.H
The order of header inclusion is important: adc.h includes the device
specific memorymap.h header before including this header file. */

/** @cond */
#if defined(LIBOPENWCH_ADC_H) || defined(LIBOPENWCH_ADC_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_ADC_COMMON_V1_H
#define LIBOPENWCH_ADC_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * The CH32V00x has a single 12-bit successive-approximation ADC, ADC1, on
 * APB2.  Unlike the USART/I2C/TIM blocks it is a full 32-bit peripheral: the
 * registers sit on 4-byte boundaries and are accessed as MMIO32.
 *
 * The block is the classic STM32-style ADC (status, two control registers,
 * two sample-time registers, a regular and an injected sequence, four injected
 * data registers, an analog watchdog and a single regular data register).
 *
 * The CH32 version adds two things the STM32 original does not have:
 *
 *   * a built-in calibration voltage that can be routed to the internal
 *     Vcalint channel (channel 9) -- CTLR1.CALVOLSELECT, programmed with
 *     adc_set_calibration_voltage();
 *   * a programmable delay inserted between an external trigger edge and the
 *     start of the conversion -- DLYR, programmed with
 *     adc_set_external_trigger_delay().  The delay source bit selects whether
 *     the delay applies to the regular or the injected group.
 *
 * The regular sequence is 16 ranks deep and the injected sequence 4 ranks.
 * Ranks are 1-based here (rank 1 is the first conversion), while the register
 * fields L and JL hold length-1.
 */

/* --- Register accessors -------------------------------------------------- */

/* Status register (ADCx_STATR) */
#define ADC_STATR(adc) MMIO32((adc) + 0x00)
/* Control register 1 (ADCx_CTLR1) */
#define ADC_CTLR1(adc) MMIO32((adc) + 0x04)
/* Control register 2 (ADCx_CTLR2) */
#define ADC_CTLR2(adc) MMIO32((adc) + 0x08)
/* Sample time register 1, channels 10..15 (ADCx_SAMPTR1) */
#define ADC_SAMPTR1(adc) MMIO32((adc) + 0x0c)
/* Sample time register 2, channels 0..9 (ADCx_SAMPTR2) */
#define ADC_SAMPTR2(adc) MMIO32((adc) + 0x10)
/* Injected channel data offset register 1 (ADCx_IOFR1) */
#define ADC_IOFR1(adc) MMIO32((adc) + 0x14)
/* Injected channel data offset register 2 (ADCx_IOFR2) */
#define ADC_IOFR2(adc) MMIO32((adc) + 0x18)
/* Injected channel data offset register 3 (ADCx_IOFR3) */
#define ADC_IOFR3(adc) MMIO32((adc) + 0x1c)
/* Injected channel data offset register 4 (ADCx_IOFR4) */
#define ADC_IOFR4(adc) MMIO32((adc) + 0x20)
/* Analog watchdog high threshold register (ADCx_WDHTR) */
#define ADC_WDHTR(adc) MMIO32((adc) + 0x24)
/* Analog watchdog low threshold register (ADCx_WDLTR) */
#define ADC_WDLTR(adc) MMIO32((adc) + 0x28)
/* Regular sequence register 1, ranks 13..16 (ADCx_RSQR1) */
#define ADC_RSQR1(adc) MMIO32((adc) + 0x2c)
/* Regular sequence register 2, ranks 7..12 (ADCx_RSQR2) */
#define ADC_RSQR2(adc) MMIO32((adc) + 0x30)
/* Regular sequence register 3, ranks 1..6 (ADCx_RSQR3) */
#define ADC_RSQR3(adc) MMIO32((adc) + 0x34)
/* Injected sequence register (ADCx_ISQR) */
#define ADC_ISQR(adc) MMIO32((adc) + 0x38)
/* Injected data register 1 (ADCx_IDATAR1) */
#define ADC_IDATAR1(adc) MMIO32((adc) + 0x3c)
/* Injected data register 2 (ADCx_IDATAR2) */
#define ADC_IDATAR2(adc) MMIO32((adc) + 0x40)
/* Injected data register 3 (ADCx_IDATAR3) */
#define ADC_IDATAR3(adc) MMIO32((adc) + 0x44)
/* Injected data register 4 (ADCx_IDATAR4) */
#define ADC_IDATAR4(adc) MMIO32((adc) + 0x48)
/* Regular data register (ADCx_RDATAR) */
#define ADC_RDATAR(adc) MMIO32((adc) + 0x4c)
/* External trigger delay register (ADCx_DLYR) */
#define ADC_DLYR(adc) MMIO32((adc) + 0x50)

/* --- STATR bits ---------------------------------------------------------- */

#define ADC_STATR_AWD (1 << 0)	 /**< analog watchdog flag */
#define ADC_STATR_EOC (1 << 1)	 /**< end of regular conversion */
#define ADC_STATR_JEOC (1 << 2)	 /**< end of injected conversion */
#define ADC_STATR_JSTRT (1 << 3) /**< injected conversion started */
#define ADC_STATR_STRT (1 << 4)	 /**< regular conversion started */

/** Every defined STATR flag.  STATR is clear-on-write-0. */
#define ADC_STATR_FLAG_MASK                                                    \
	(ADC_STATR_AWD | ADC_STATR_EOC | ADC_STATR_JEOC | ADC_STATR_JSTRT |    \
	 ADC_STATR_STRT)

/* --- CTLR1 bits ---------------------------------------------------------- */

#define ADC_CTLR1_AWDCH_SHIFT 0
#define ADC_CTLR1_AWDCH_MASK 0x0000001fu /**< watchdog channel */
#define ADC_CTLR1_EOCIE (1 << 5)	 /**< EOC interrupt enable */
#define ADC_CTLR1_AWDIE (1 << 6)	 /**< watchdog interrupt enable */
#define ADC_CTLR1_JEOCIE (1 << 7)	 /**< injected EOC int enable */
#define ADC_CTLR1_SCAN (1 << 8)		 /**< scan mode */
#define ADC_CTLR1_AWDSGL (1 << 9)	 /**< watchdog on one channel */
#define ADC_CTLR1_JAUTO (1 << 10)	 /**< auto injected group */
#define ADC_CTLR1_DISCEN (1 << 11)	 /**< discontinuous, regular */
#define ADC_CTLR1_JDISCEN (1 << 12)	 /**< discontinuous, injected */
#define ADC_CTLR1_DISCNUM_SHIFT 13
#define ADC_CTLR1_DISCNUM_MASK (0x7u << 13) /**< discontinuous count */
#define ADC_CTLR1_JAWDEN (1 << 22)	    /**< watchdog on injected */
#define ADC_CTLR1_AWDEN (1 << 23)	    /**< watchdog on regular */
#define ADC_CTLR1_CALVOLSELECT_SHIFT 25
#define ADC_CTLR1_CALVOLSELECT_MASK (0x3u << 25) /**< calibration voltage */
#define ADC_CTLR1_CALVOLSELECT_0 (1 << 25)
#define ADC_CTLR1_CALVOLSELECT_1 (1 << 26)

/* --- CTLR2 bits ---------------------------------------------------------- */

#define ADC_CTLR2_ADON (1 << 0)	  /**< ADC on */
#define ADC_CTLR2_CONT (1 << 1)	  /**< continuous conversion */
#define ADC_CTLR2_CAL (1 << 2)	  /**< start calibration */
#define ADC_CTLR2_RSTCAL (1 << 3) /**< reset calibration */
#define ADC_CTLR2_DMA (1 << 8)	  /**< DMA enable */
#define ADC_CTLR2_ALIGN (1 << 11) /**< 1 = left aligned */
#define ADC_CTLR2_JEXTSEL_SHIFT 12
#define ADC_CTLR2_JEXTSEL_MASK (0x7u << 12) /**< injected trigger select */
#define ADC_CTLR2_JEXTTRIG (1 << 15)	    /**< injected ext trigger en */
#define ADC_CTLR2_EXTSEL_SHIFT 17
#define ADC_CTLR2_EXTSEL_MASK (0x7u << 17) /**< regular trigger select */
#define ADC_CTLR2_EXTTRIG (1 << 20)	   /**< regular ext trigger en */
#define ADC_CTLR2_JSWSTART (1 << 21)	   /**< start injected conversion */
#define ADC_CTLR2_SWSTART (1 << 22)	   /**< start regular conversion */
#define ADC_CTLR2_TSVREFE (1 << 23)	   /**< temp sensor / Vrefint en */

/* --- SAMPTR1 / SAMPTR2 --------------------------------------------------- */

/** One 3-bit sample-time field, before shifting into place. */
#define ADC_SAMPTR_SMP_MASK 0x7u

/** SAMPTR1 holds channels 10..15, three bits each (bits 0..17). */
#define ADC_SAMPTR1_SMP_MASK 0x0003ffffu
/** SAMPTR2 holds channels 0..9, three bits each (bits 0..29). */
#define ADC_SAMPTR2_SMP_MASK 0x3fffffffu

/* --- IOFR1..IOFR4, WDHTR and WDLTR --------------------------------------- */

/** 10-bit injected offset field (all four IOFR registers). */
#define ADC_IOFR_JOFFSET_MASK 0x000003ffu
/** 10-bit analog watchdog high threshold. */
#define ADC_WDHTR_HT_MASK 0x000003ffu
/** 10-bit analog watchdog low threshold. */
#define ADC_WDLTR_LT_MASK 0x000003ffu

/* --- RSQR1..RSQR3 and ISQR ----------------------------------------------- */

/** One 5-bit sequence field, before shifting into place. */
#define ADC_RSQR_SQ_MASK 0x0000001fu
/** One 5-bit injected sequence field, before shifting into place. */
#define ADC_ISQR_JSQ_MASK 0x0000001fu

#define ADC_RSQR1_L_SHIFT 20
#define ADC_RSQR1_L_MASK (0xfu << 20) /**< regular length - 1 */
#define ADC_ISQR_JL_SHIFT 20
#define ADC_ISQR_JL_MASK (0x3u << 20) /**< injected length - 1 */

/* --- IDATAR1..IDATAR4 and RDATAR ----------------------------------------- */

/** 12-bit conversion result, in whichever alignment is selected. */
#define ADC_IDATAR_JDATA_MASK 0x0000ffffu
#define ADC_RDATAR_DATA_MASK 0x0000ffffu

/* --- DLYR ---------------------------------------------------------------- */

#define ADC_DLYR_DLYVLU_MASK 0x000001ffu /**< trigger delay value */
#define ADC_DLYR_DLYSRC (1 << 9)	 /**< delay on injected */

/* --- Limits -------------------------------------------------------------- */

/** Highest channel number the sample-time registers can describe. */
#define ADC_CHANNEL_MAX 15u
/** Highest regular sequence rank (1-based). */
#define ADC_RANK_MAX 16u
/** Highest injected sequence rank (1-based). */
#define ADC_INJECTED_RANK_MAX 4u

/* --- Configuration identifiers ------------------------------------------- */

/** @defgroup adc_sample_time ADC Sample Times
@ingroup adc_defines

The sampling time in ADC clock cycles.  Longer times suit higher source
impedance.

@{*/
enum adc_sample_time {
	ADC_SAMPLETIME_3CYCLES = 0, /**< 3 cycles */
	ADC_SAMPLETIME_9CYCLES,	    /**< 9 cycles */
	ADC_SAMPLETIME_15CYCLES,    /**< 15 cycles */
	ADC_SAMPLETIME_30CYCLES,    /**< 30 cycles */
	ADC_SAMPLETIME_43CYCLES,    /**< 43 cycles */
	ADC_SAMPLETIME_57CYCLES,    /**< 57 cycles */
	ADC_SAMPLETIME_73CYCLES,    /**< 73 cycles */
	ADC_SAMPLETIME_241CYCLES,   /**< 241 cycles */
};
/**@}*/

/** @defgroup adc_exttrig_regular ADC Regular External Triggers
@ingroup adc_defines

Values for the EXTSEL field, passed to adc_set_external_trigger_regular().

@{*/
#define ADC_EXTTRIG_REGULAR_T1_TRGO (0x0u << ADC_CTLR2_EXTSEL_SHIFT)
#define ADC_EXTTRIG_REGULAR_T1_CC1 (0x1u << ADC_CTLR2_EXTSEL_SHIFT)
#define ADC_EXTTRIG_REGULAR_T1_CC2 (0x2u << ADC_CTLR2_EXTSEL_SHIFT)
#define ADC_EXTTRIG_REGULAR_T2_TRGO (0x3u << ADC_CTLR2_EXTSEL_SHIFT)
#define ADC_EXTTRIG_REGULAR_T2_CC1 (0x4u << ADC_CTLR2_EXTSEL_SHIFT)
#define ADC_EXTTRIG_REGULAR_T2_CC2 (0x5u << ADC_CTLR2_EXTSEL_SHIFT)
#define ADC_EXTTRIG_REGULAR_EXT_PD3_PC2 (0x6u << ADC_CTLR2_EXTSEL_SHIFT)
#define ADC_EXTTRIG_REGULAR_NONE (0x7u << ADC_CTLR2_EXTSEL_SHIFT)
/**@}*/

/** @defgroup adc_exttrig_injected ADC Injected External Triggers
@ingroup adc_defines

Values for the JEXTSEL field, passed to adc_set_external_trigger_injected().

@{*/
#define ADC_EXTTRIG_INJECTED_T1_CC3 (0x0u << ADC_CTLR2_JEXTSEL_SHIFT)
#define ADC_EXTTRIG_INJECTED_T1_CC4 (0x1u << ADC_CTLR2_JEXTSEL_SHIFT)
#define ADC_EXTTRIG_INJECTED_T2_CC3 (0x2u << ADC_CTLR2_JEXTSEL_SHIFT)
#define ADC_EXTTRIG_INJECTED_T2_CC4 (0x3u << ADC_CTLR2_JEXTSEL_SHIFT)
#define ADC_EXTTRIG_INJECTED_EXT_PD1_PA2 (0x6u << ADC_CTLR2_JEXTSEL_SHIFT)
#define ADC_EXTTRIG_INJECTED_NONE (0x7u << ADC_CTLR2_JEXTSEL_SHIFT)
/**@}*/

/** @defgroup adc_calvol ADC Calibration Voltage
@ingroup adc_defines

The internally generated calibration voltage, available on the Vcalint
channel (channel 9).  Values for adc_set_calibration_voltage().

@{*/
#define ADC_CALVOL_DISABLE 0x00000000u
#define ADC_CALVOL_50PERCENT ADC_CTLR1_CALVOLSELECT_0
#define ADC_CALVOL_75PERCENT ADC_CTLR1_CALVOLSELECT_1
/**@}*/

/** @defgroup adc_dly_source ADC External Trigger Delay Source
@ingroup adc_defines

Selects which group the DLYR trigger delay applies to, passed to
adc_set_external_trigger_delay().

@{*/
#define ADC_DLYR_SOURCE_REGULAR 0x00000000u
#define ADC_DLYR_SOURCE_INJECTED ADC_DLYR_DLYSRC
/**@}*/

/** @defgroup adc_flag ADC Flags
@ingroup adc_defines

Flag bits for adc_get_flag() and adc_clear_flag().

@{*/
#define ADC_FLAG_AWD ADC_STATR_AWD
#define ADC_FLAG_EOC ADC_STATR_EOC
#define ADC_FLAG_JEOC ADC_STATR_JEOC
#define ADC_FLAG_JSTRT ADC_STATR_JSTRT
#define ADC_FLAG_STRT ADC_STATR_STRT
/**@}*/

/** @defgroup adc_irq ADC Interrupts
@ingroup adc_defines

Interrupt-enable bits for adc_enable_irq() and adc_disable_irq().

@{*/
#define ADC_IRQ_EOC ADC_CTLR1_EOCIE
#define ADC_IRQ_AWD ADC_CTLR1_AWDIE
#define ADC_IRQ_JEOC ADC_CTLR1_JEOCIE
/**@}*/

/** Every interrupt-enable bit in CTLR1. */
#define ADC_IRQ_MASK (ADC_IRQ_EOC | ADC_IRQ_AWD | ADC_IRQ_JEOC)

BEGIN_DECLS

/* --- Enable and conversion ----------------------------------------------- */

void adc_enable(uint32_t adc);
void adc_disable(uint32_t adc);
void adc_start_conversion_regular(uint32_t adc);
void adc_start_conversion_injected(uint32_t adc);
void adc_set_continuous_conversion_mode(uint32_t adc);
void adc_set_single_conversion_mode(uint32_t adc);
void adc_set_scan_mode(uint32_t adc);

/* --- Regular sequence and sample times ----------------------------------- */

void adc_set_channel(uint32_t adc, uint8_t channel, uint8_t rank);
void adc_set_sample_time(uint32_t adc,
			 uint8_t channel,
			 enum adc_sample_time time);
void adc_set_sample_time_on_all_channels(uint32_t adc,
					 enum adc_sample_time time);
void adc_set_right_aligned(uint32_t adc);

/* --- External triggers --------------------------------------------------- */

void adc_set_external_trigger_regular(uint32_t adc, uint32_t trigger);
void adc_set_external_trigger_injected(uint32_t adc, uint32_t trigger);
void adc_enable_external_trigger_regular(uint32_t adc);
void adc_disable_external_trigger_regular(uint32_t adc);

/* --- Data ---------------------------------------------------------------- */

uint16_t adc_read_regular(uint32_t adc);
uint16_t adc_read_injected(uint32_t adc, uint8_t rank);
void adc_set_injected_offset(uint32_t adc, uint8_t rank, uint16_t offset);

/* --- DMA ----------------------------------------------------------------- */

void adc_enable_dma(uint32_t adc);
void adc_disable_dma(uint32_t adc);

/* --- Internal channels --------------------------------------------------- */

void adc_enable_temperature_sensor(uint32_t adc);
void adc_enable_vrefint(uint32_t adc);

/* --- Calibration --------------------------------------------------------- */

void adc_reset_calibration(uint32_t adc);
void adc_start_calibration(uint32_t adc);
bool adc_is_calibration_complete(uint32_t adc);
void adc_set_calibration_voltage(uint32_t adc, uint32_t calvol);
void adc_set_external_trigger_delay(uint32_t adc,
				    uint32_t source,
				    uint16_t delay);

/* --- Analog watchdog ----------------------------------------------------- */

void adc_enable_analog_watchdog_regular(uint32_t adc);
void adc_set_watchdog_high_threshold(uint32_t adc, uint16_t threshold);
void adc_set_watchdog_low_threshold(uint32_t adc, uint16_t threshold);

/* --- Interrupts and flags ------------------------------------------------ */

void adc_enable_irq(uint32_t adc, uint32_t irq);
void adc_disable_irq(uint32_t adc, uint32_t irq);
uint32_t adc_get_flag(uint32_t adc, uint32_t flag);
void adc_clear_flag(uint32_t adc, uint32_t flag);

END_DECLS

#endif
/** @cond */
#else
#warning "adc_common_v1.h should not be included explicitly, only via adc.h"
#endif
/** @endcond */
/**@}*/
