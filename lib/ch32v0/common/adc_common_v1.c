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

/** @addtogroup adc_file ADC
 *
 * @ingroup CH32V0
 *
 * @brief <b>Analog to Digital Converter for the CH32V00x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * The ADC1 block is a 12-bit switched-capacitor converter driven from APB2.
 * A conversion is started either in software (EXTTRIG + SWSTART/JSWSTART for
 * the regular group, JEXTTRIG + JSWSTART for the injected group) or by one of
 * the timer/external events selected through EXTSEL/JEXTSEL.
 *
 * Two setup steps must happen after the ADC is powered on and before the first
 * conversion:
 *
 *	adc_reset_calibration(adc);
 *	adc_start_calibration(adc);
 *
 * Both wait for the hardware to self-clear the RSTCAL/CAL bits, so a conversion
 * started after adc_start_calibration() returns uses the freshly measured
 * calibration factor.
 *
 * The regular sequence is programmed one rank at a time with
 * adc_set_channel(); the sequence length stored in RSQR1.L is grown to cover
 * the highest rank written.  Ranks are 1-based, matching the reference manual.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch32v0/adc.h>
#include <libopenwch/qingke/assert.h>

/*
 * The regular sequence is spread over three registers: RSQR3 covers ranks
 * 1..6, RSQR2 ranks 7..12 and RSQR1 ranks 13..16.  Each rank is five bits.
 */
#define ADC_RSQR_SQ_BITS		5u
#define ADC_RSQR3_RANKS			6u
#define ADC_RSQR23_RANKS		12u

/*
 * The sample-time registers are split by channel rather than by rank:
 * SAMPTR2 covers channels 0..9, SAMPTR1 channels 10..15, three bits each.
 */
#define ADC_SAMPTR_SPLIT_CHANNEL	10u
#define ADC_SAMPTR_SMP_BITS		3u

void adc_enable(uint32_t adc) {
	ADC_CTLR2(adc) |= ADC_CTLR2_ADON;
}

void adc_disable(uint32_t adc) {
	ADC_CTLR2(adc) &= ~ADC_CTLR2_ADON;
}

void adc_start_conversion_regular(uint32_t adc) {
	ADC_CTLR2(adc) |= ADC_CTLR2_SWSTART;
}

void adc_start_conversion_injected(uint32_t adc) {
	ADC_CTLR2(adc) |= ADC_CTLR2_JSWSTART;
}

void adc_set_continuous_conversion_mode(uint32_t adc) {
	ADC_CTLR2(adc) |= ADC_CTLR2_CONT;
}

void adc_set_single_conversion_mode(uint32_t adc) {
	ADC_CTLR2(adc) &= ~ADC_CTLR2_CONT;
}

void adc_set_scan_mode(uint32_t adc) {
	ADC_CTLR1(adc) |= ADC_CTLR1_SCAN;
}

void adc_set_channel(uint32_t adc, uint8_t channel, uint8_t rank) {
	uint32_t reg;
	uint32_t shift;
	uint32_t length;

	openwch_assert(channel <= ADC_CHANNEL_MAX);
	openwch_assert(rank >= 1u);
	openwch_assert(rank <= ADC_RANK_MAX);

	if (rank <= ADC_RSQR3_RANKS) {
		shift = ADC_RSQR_SQ_BITS * (uint32_t)(rank - 1u);
		reg = (ADC_RSQR3(adc) & ~(ADC_RSQR_SQ_MASK << shift))
			| (((uint32_t)channel & ADC_RSQR_SQ_MASK) << shift);
		ADC_RSQR3(adc) = reg;
	} else if (rank <= ADC_RSQR23_RANKS) {
		shift = ADC_RSQR_SQ_BITS * (uint32_t)(rank - 1u
						      - ADC_RSQR3_RANKS);
		reg = (ADC_RSQR2(adc) & ~(ADC_RSQR_SQ_MASK << shift))
			| (((uint32_t)channel & ADC_RSQR_SQ_MASK) << shift);
		ADC_RSQR2(adc) = reg;
	} else {
		shift = ADC_RSQR_SQ_BITS * (uint32_t)(rank - 1u
						      - ADC_RSQR23_RANKS);
		reg = (ADC_RSQR1(adc) & ~(ADC_RSQR_SQ_MASK << shift))
			| (((uint32_t)channel & ADC_RSQR_SQ_MASK) << shift);
		ADC_RSQR1(adc) = reg;
	}

	/*
	 * Grow the sequence length so that the rank just written is actually
	 * converted.  L holds length - 1, so a rank of n needs L >= n - 1.
	 */
	length = (ADC_RSQR1(adc) & ADC_RSQR1_L_MASK) >> ADC_RSQR1_L_SHIFT;
	if ((uint32_t)(rank - 1u) > length) {
		ADC_RSQR1(adc) = (ADC_RSQR1(adc) & ~ADC_RSQR1_L_MASK)
			| ((uint32_t)(rank - 1u) << ADC_RSQR1_L_SHIFT);
	}
}

void adc_set_sample_time(
	uint32_t adc,
	uint8_t channel,
	enum adc_sample_time time
) {
	uint32_t shift;
	uint32_t reg;

	openwch_assert(channel <= ADC_CHANNEL_MAX);
	openwch_assert(((uint32_t)time & ~ADC_SAMPTR_SMP_MASK) == 0);

	if (channel < ADC_SAMPTR_SPLIT_CHANNEL) {
		shift = ADC_SAMPTR_SMP_BITS * (uint32_t)channel;
		reg = (ADC_SAMPTR2(adc) & ~(ADC_SAMPTR_SMP_MASK << shift))
			| (((uint32_t)time & ADC_SAMPTR_SMP_MASK) << shift);
		ADC_SAMPTR2(adc) = reg;
	} else {
		shift = ADC_SAMPTR_SMP_BITS
			* (uint32_t)(channel - ADC_SAMPTR_SPLIT_CHANNEL);
		reg = (ADC_SAMPTR1(adc) & ~(ADC_SAMPTR_SMP_MASK << shift))
			| (((uint32_t)time & ADC_SAMPTR_SMP_MASK) << shift);
		ADC_SAMPTR1(adc) = reg;
	}
}

void adc_set_sample_time_on_all_channels(
	uint32_t adc,
	enum adc_sample_time time
) {
	uint32_t all;

	openwch_assert(((uint32_t)time & ~ADC_SAMPTR_SMP_MASK) == 0);

	/*
	 * Replicate the three-bit value into every 3-bit field of a 32-bit
	 * word: after each doubling shift the low N bits already hold the
	 * pattern, so the later shifts tile it across the whole register.
	 */
	all = (uint32_t)time;
	all |= all << 3;
	all |= all << 6;
	all |= all << 12;
	all |= all << 24;

	ADC_SAMPTR1(adc) = all & ADC_SAMPTR1_SMP_MASK;
	ADC_SAMPTR2(adc) = all & ADC_SAMPTR2_SMP_MASK;
}

void adc_set_right_aligned(uint32_t adc) {
	ADC_CTLR2(adc) &= ~ADC_CTLR2_ALIGN;
}

void adc_set_external_trigger_regular(uint32_t adc, uint32_t trigger) {
	openwch_assert((trigger & ~ADC_CTLR2_EXTSEL_MASK) == 0);

	ADC_CTLR2(adc) = (ADC_CTLR2(adc) & ~ADC_CTLR2_EXTSEL_MASK)
		| (trigger & ADC_CTLR2_EXTSEL_MASK);
}

void adc_set_external_trigger_injected(uint32_t adc, uint32_t trigger) {
	openwch_assert((trigger & ~ADC_CTLR2_JEXTSEL_MASK) == 0);

	ADC_CTLR2(adc) = (ADC_CTLR2(adc) & ~ADC_CTLR2_JEXTSEL_MASK)
		| (trigger & ADC_CTLR2_JEXTSEL_MASK);
}

void adc_enable_external_trigger_regular(uint32_t adc) {
	ADC_CTLR2(adc) |= ADC_CTLR2_EXTTRIG;
}

void adc_disable_external_trigger_regular(uint32_t adc) {
	ADC_CTLR2(adc) &= ~ADC_CTLR2_EXTTRIG;
}

uint16_t adc_read_regular(uint32_t adc) {
	return (uint16_t)(ADC_RDATAR(adc) & ADC_RDATAR_DATA_MASK);
}

uint16_t adc_read_injected(uint32_t adc, uint8_t rank) {
	switch (rank) {
	case 1:
		return (uint16_t)(ADC_IDATAR1(adc) & ADC_IDATAR_JDATA_MASK);
	case 2:
		return (uint16_t)(ADC_IDATAR2(adc) & ADC_IDATAR_JDATA_MASK);
	case 3:
		return (uint16_t)(ADC_IDATAR3(adc) & ADC_IDATAR_JDATA_MASK);
	case 4:
		return (uint16_t)(ADC_IDATAR4(adc) & ADC_IDATAR_JDATA_MASK);
	default:
		openwch_assert_not_reached();
		return 0;
	}
}

void adc_set_injected_offset(uint32_t adc, uint8_t rank, uint16_t offset) {
	uint32_t value;

	openwch_assert(rank >= 1u);
	openwch_assert(rank <= ADC_INJECTED_RANK_MAX);
	openwch_assert((offset & ~ADC_IOFR_JOFFSET_MASK) == 0);

	value = (uint32_t)offset & ADC_IOFR_JOFFSET_MASK;

	switch (rank) {
	case 1:
		ADC_IOFR1(adc) = value;
		break;
	case 2:
		ADC_IOFR2(adc) = value;
		break;
	case 3:
		ADC_IOFR3(adc) = value;
		break;
	default:
		ADC_IOFR4(adc) = value;
		break;
	}
}

void adc_enable_dma(uint32_t adc) {
	ADC_CTLR2(adc) |= ADC_CTLR2_DMA;
}

void adc_disable_dma(uint32_t adc) {
	ADC_CTLR2(adc) &= ~ADC_CTLR2_DMA;
}

/*
 * CTLR2.TSVREFE gates both the on-die temperature sensor and the internal
 * reference, as on the STM32 parts the block derives from: a single bit
 * enables the pair, so both helpers touch the same bit.
 */
void adc_enable_temperature_sensor(uint32_t adc) {
	ADC_CTLR2(adc) |= ADC_CTLR2_TSVREFE;
}

void adc_enable_vrefint(uint32_t adc) {
	ADC_CTLR2(adc) |= ADC_CTLR2_TSVREFE;
}

void adc_reset_calibration(uint32_t adc) {
	ADC_CTLR2(adc) |= ADC_CTLR2_RSTCAL;

	/* The hardware clears RSTCAL when the reset sequence has finished. */
	while (ADC_CTLR2(adc) & ADC_CTLR2_RSTCAL) {
		;
	}
}

void adc_start_calibration(uint32_t adc) {
	ADC_CTLR2(adc) |= ADC_CTLR2_CAL;

	/* The hardware clears CAL when the calibration has finished. */
	while (ADC_CTLR2(adc) & ADC_CTLR2_CAL) {
		;
	}
}

bool adc_is_calibration_complete(uint32_t adc) {
	return (ADC_CTLR2(adc) & ADC_CTLR2_CAL) == 0;
}

void adc_set_calibration_voltage(uint32_t adc, uint32_t calvol) {
	openwch_assert(calvol == ADC_CALVOL_DISABLE
		       || calvol == ADC_CALVOL_50PERCENT
		       || calvol == ADC_CALVOL_75PERCENT);

	ADC_CTLR1(adc) = (ADC_CTLR1(adc) & ~ADC_CTLR1_CALVOLSELECT_MASK)
		| (calvol & ADC_CTLR1_CALVOLSELECT_MASK);
}

void adc_set_external_trigger_delay(
	uint32_t adc,
	uint32_t source,
	uint16_t delay
) {
	openwch_assert(source == ADC_DLYR_SOURCE_REGULAR
		       || source == ADC_DLYR_SOURCE_INJECTED);
	openwch_assert((delay & ~ADC_DLYR_DLYVLU_MASK) == 0);

	ADC_DLYR(adc) = (ADC_DLYR(adc)
			 & ~(ADC_DLYR_DLYSRC | ADC_DLYR_DLYVLU_MASK))
		| (source & ADC_DLYR_DLYSRC)
		| ((uint32_t)delay & ADC_DLYR_DLYVLU_MASK);
}

void adc_enable_analog_watchdog_regular(uint32_t adc) {
	ADC_CTLR1(adc) |= ADC_CTLR1_AWDEN;
}

void adc_set_watchdog_high_threshold(uint32_t adc, uint16_t threshold) {
	openwch_assert((threshold & ~ADC_WDHTR_HT_MASK) == 0);

	ADC_WDHTR(adc) = (uint32_t)threshold & ADC_WDHTR_HT_MASK;
}

void adc_set_watchdog_low_threshold(uint32_t adc, uint16_t threshold) {
	openwch_assert((threshold & ~ADC_WDLTR_LT_MASK) == 0);

	ADC_WDLTR(adc) = (uint32_t)threshold & ADC_WDLTR_LT_MASK;
}

void adc_enable_irq(uint32_t adc, uint32_t irq) {
	openwch_assert((irq & ~ADC_IRQ_MASK) == 0);

	ADC_CTLR1(adc) |= irq;
}

void adc_disable_irq(uint32_t adc, uint32_t irq) {
	openwch_assert((irq & ~ADC_IRQ_MASK) == 0);

	ADC_CTLR1(adc) &= ~irq;
}

uint32_t adc_get_flag(uint32_t adc, uint32_t flag) {
	return ADC_STATR(adc) & flag;
}

void adc_clear_flag(uint32_t adc, uint32_t flag) {
	/*
	 * STATR is clear-on-write-0, so the bits to clear are written as 0
	 * and every other bit as 1.  The reserved bits read as zero and
	 * ignore writes.
	 */
	ADC_STATR(adc) = ~(flag & ADC_STATR_FLAG_MASK);
}
/**@}*/
