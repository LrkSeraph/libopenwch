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
 * @ingroup CH5XX58X
 *
 * @brief <b>Analog to Digital Converter and touch key for the CH58x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * The conversion sequences follow WCH's EVT driver, which is the only public
 * description of the register order:
 *
 *   - initialise the block: power on the ADC, choose buffered or differential
 *     input, choose the sample clock and the PGA gain;
 *   - select the channel with adc_set_channel();
 *   - adc_start() writes RB_ADC_START, which auto-clears;
 *   - adc_is_ready() tests RB_ADC_IF_EOC and adc_read() reads R16_ADC_DATA.
 *
 * Writes to R8_ADC_CFG, R8_ADC_CONVERT and R8_TKEY_CFG go through the RWA_*
 * macros; see the note in adc_common_v1.h for why.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch5xx58x/adc.h>
#include <libopenwch/ch5xx58x/rwa.h>
#include <libopenwch/qingke/assert.h>

/* --- Initialisation ------------------------------------------------------ */

void adc_init_single_channel(uint32_t adc, adc_sample_clk_t sample_clk,
			     adc_pga_t pga)
{
	openwch_assert(adc == ADC_BASE);
	openwch_assert(sample_clk <= ADC_SAMPLE_CLK_4MHZ);
	openwch_assert(pga <= ADC_PGA_2);

	/* The touch-key block and the ADC share the input mux; make sure the
	 * touch-key front end is off before configuring the ADC. */
	RWA_CLEAR_BITS(ADC_TKEY_CFG(adc), RB_TKEY_PWR_ON);

	RWA_WRITE8(ADC_CFG(adc),
		   (uint8_t)(RB_ADC_POWER_ON | RB_ADC_BUF_EN |
			     ((uint8_t)sample_clk << ADC_CLK_DIV_SHIFT) |
			     ((uint8_t)pga << ADC_PGA_GAIN_SHIFT)));
}

void adc_init_differential(uint32_t adc, adc_sample_clk_t sample_clk,
			   adc_pga_t pga)
{
	openwch_assert(adc == ADC_BASE);
	openwch_assert(sample_clk <= ADC_SAMPLE_CLK_4MHZ);
	openwch_assert(pga <= ADC_PGA_2);

	RWA_CLEAR_BITS(ADC_TKEY_CFG(adc), RB_TKEY_PWR_ON);

	/* Differential mode replaces the input buffer, so RB_ADC_BUF_EN is
	 * deliberately not set here. */
	RWA_WRITE8(ADC_CFG(adc),
		   (uint8_t)(RB_ADC_POWER_ON | RB_ADC_DIFF_EN |
			     ((uint8_t)sample_clk << ADC_CLK_DIV_SHIFT) |
			     ((uint8_t)pga << ADC_PGA_GAIN_SHIFT)));
}

void adc_init_temperature(uint32_t adc)
{
	openwch_assert(adc == ADC_BASE);

	RWA_CLEAR_BITS(ADC_TKEY_CFG(adc), RB_TKEY_PWR_ON);

	/* R8_TEM_SENSOR is a plain RW register in the CH583 SFR header. */
	ADC_TEM_SENSOR(adc) = RB_TEM_SEN_PWR_ON;
	ADC_CHANNEL(adc) = (uint8_t)ADC_CH_VTEMP;

	/*
	 * WCH's sequence: differential mode and PGA = 0b11 (+6 dB), no
	 * sample-clock change.  The sensor output is a delta against the
	 * internal reference, which is why differential mode is used.
	 */
	RWA_WRITE8(ADC_CFG(adc),
		   (uint8_t)(RB_ADC_POWER_ON | RB_ADC_DIFF_EN |
			     (3u << ADC_PGA_GAIN_SHIFT)));
}

void adc_init_battery(uint32_t adc)
{
	openwch_assert(adc == ADC_BASE);

	RWA_CLEAR_BITS(ADC_TKEY_CFG(adc), RB_TKEY_PWR_ON);

	ADC_CHANNEL(adc) = (uint8_t)ADC_CH_VBAT;

	/*
	 * The battery rail can sit above the 3.3 V reference, so WCH uses the
	 * buffered input with the fixed 1/4x PGA setting (PGA = 0b00).
	 */
	RWA_WRITE8(ADC_CFG(adc),
		   (uint8_t)(RB_ADC_POWER_ON | RB_ADC_BUF_EN |
			     (ADC_PGA_1_4 << ADC_PGA_GAIN_SHIFT)));
}

/* --- Configuration ------------------------------------------------------- */

void adc_set_channel(uint32_t adc, adc_channel_t channel)
{
	openwch_assert(adc == ADC_BASE);
	openwch_assert(channel <= ADC_CH_VTEMP);

	ADC_CHANNEL(adc) = (uint8_t)channel;
}

void adc_set_sample_clock(uint32_t adc, adc_sample_clk_t clk)
{
	openwch_assert(adc == ADC_BASE);
	openwch_assert(clk <= ADC_SAMPLE_CLK_4MHZ);

	RWA_MODIFY(ADC_CFG(adc), RB_ADC_CLK_DIV,
		   (uint32_t)((uint32_t)clk << ADC_CLK_DIV_SHIFT));
}

void adc_set_pga(uint32_t adc, adc_pga_t pga)
{
	openwch_assert(adc == ADC_BASE);
	openwch_assert(pga <= ADC_PGA_2);

	RWA_MODIFY(ADC_CFG(adc), RB_ADC_PGA_GAIN,
		   (uint32_t)((uint32_t)pga << ADC_PGA_GAIN_SHIFT));
}

/* --- Single conversion --------------------------------------------------- */

void adc_start(uint32_t adc)
{
	openwch_assert(adc == ADC_BASE);

	/* Writing RB_ADC_START also clears RB_ADC_IF_EOC. */
	RWA_WRITE8(ADC_CONVERT(adc), RB_ADC_START);
}

bool adc_is_ready(uint32_t adc)
{
	openwch_assert(adc == ADC_BASE);

	return (ADC_INT_FLAG(adc) & RB_ADC_IF_EOC) != 0;
}

uint16_t adc_read(uint32_t adc)
{
	openwch_assert(adc == ADC_BASE);

	return (uint16_t)(ADC_DATA(adc) & RB_ADC_DATA);
}

/* --- Auto conversion and DMA --------------------------------------------- */

void adc_set_auto_cycle(uint32_t adc, uint8_t cycles)
{
	openwch_assert(adc == ADC_BASE);

	ADC_AUTO_CYCLE(adc) = cycles;
}

void adc_enable_dma(uint32_t adc, adc_dma_mode_t mode, uint16_t start,
		    uint16_t end)
{
	uint8_t ctrl;

	openwch_assert(adc == ADC_BASE);
	openwch_assert(mode <= ADC_DMA_MODE_LOOP);

	ADC_DMA_BEG(adc) = start;
	ADC_DMA_END(adc) = end;

	ctrl = (uint8_t)(ADC_DMA_CTRL(adc) &
			 (uint8_t)~RB_ADC_DMA_LOOP);

	if (mode == ADC_DMA_MODE_LOOP) {
		ctrl = (uint8_t)(ctrl | RB_ADC_DMA_LOOP);
	}

	ADC_DMA_CTRL(adc) = (uint8_t)(ctrl |
				      RB_ADC_IE_DMA_END |
				      RB_ADC_DMA_ENABLE);
}

void adc_disable_dma(uint32_t adc)
{
	openwch_assert(adc == ADC_BASE);

	ADC_DMA_CTRL(adc) &= (uint8_t)~(RB_ADC_DMA_ENABLE | RB_ADC_IE_DMA_END);
}

/* --- Touch key ----------------------------------------------------------- */

void adc_enable_touchkey(uint32_t adc)
{
	openwch_assert(adc == ADC_BASE);

	/* WCH's TouchKey_ChSampInit(): buffered input at -6 dB, then power. */
	RWA_WRITE8(ADC_CFG(adc),
		   (uint8_t)(RB_ADC_POWER_ON | RB_ADC_BUF_EN |
			     (ADC_PGA_1_2 << ADC_PGA_GAIN_SHIFT)));
	RWA_SET_BITS(ADC_TKEY_CFG(adc), RB_TKEY_PWR_ON);
}

void adc_disable_touchkey(uint32_t adc)
{
	openwch_assert(adc == ADC_BASE);

	RWA_CLEAR_BITS(ADC_TKEY_CFG(adc), RB_TKEY_PWR_ON);
}

uint16_t adc_read_touchkey(uint32_t adc, uint8_t charge, uint8_t discharge)
{
	openwch_assert(adc == ADC_BASE);
	openwch_assert(charge <= RB_TKEY_CHARG_CNT);
	openwch_assert(discharge <= (RB_TKEY_DISCH_CNT >> ADC_TKEY_DISCH_SHIFT));

	ADC_TKEY_COUNT(adc) = (uint8_t)(((uint8_t)discharge <<
					 ADC_TKEY_DISCH_SHIFT) |
					((uint8_t)charge & RB_TKEY_CHARG_CNT));

	ADC_TKEY_CONVERT(adc) = RB_TKEY_START;

	/* RB_TKEY_START is cleared by hardware when the conversion finishes. */
	while (ADC_TKEY_CONVERT(adc) & RB_TKEY_START) {
		/* wait */
	}

	return (uint16_t)(ADC_DATA(adc) & RB_ADC_DATA);
}

/* --- Conversion helpers -------------------------------------------------- */

int adc_to_celsius(uint16_t raw)
{
	uint32_t cal = MMIO32(ROM_CFG_TMP_25C);
	uint32_t t_ref = (cal >> 16) & 0xffffu;
	uint32_t adc_25 = cal & 0xffffu;

	/*
	 * ROM_CFG_TMP_25C holds the ADC reading taken at 25 C in its low half
	 * and the reference temperature in its high half.  Unprogrammed parts
	 * read back zero, in which case 25 is the documented fallback.  The
	 * 10/27 slope is WCH's: 27 ADC counts per 10 C.
	 */
	if (t_ref == 0) {
		t_ref = 25;
	}

	return (int)t_ref + (((int)raw - (int)adc_25) * 10) / 27;
}
/**@}*/
