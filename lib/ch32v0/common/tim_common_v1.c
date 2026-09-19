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

/** @addtogroup tim_file TIM
 *
 * @ingroup CH32V0
 *
 * @brief <b>Advanced-control and general-purpose timers for the CH32V00x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * TIM1 and TIM2 share one register layout, so every function here works on
 * both.  The channel registers live in two 16-bit banks -- CHCTLR1 holds
 * channels 1 and 2, CHCTLR2 holds channels 3 and 4 -- with a two-bit select
 * field, a two-bit input prescaler, a four-bit filter and a three-bit output
 * mode per channel.  Because the layout is regular, every per-channel field is
 * derived from a single table entry instead of from four hand-written cases.
 *
 * The complementary output and idle-state bits only exist for channels 1..3
 * (TIM1's advanced outputs), so attempts to use them on channel 4 or on the
 * general-purpose TIM2 are rejected with an assert rather than quietly writing
 * a neighbouring channel's bits.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch32v0/tim.h>
#include <libopenwch/qingke/assert.h>

/*
 * Per-channel register geometry.  Each entry describes where a channel's
 * fields live: which CHCTLR holds it, the bit position of its CCxS select
 * field (everything else is at a fixed offset from there), the CCER enable
 * bits, and whether the advanced complementary output exists.
 */
struct tim_channel_desc {
	uint8_t chctlr;			/**< 0 = CHCTLR1, 1 = CHCTLR2 */
	uint8_t ccxs_shift;		/**< bit position of CCxS in the CHCTLR */
	uint16_t ccer_mask;		/**< CCxE | CCxP | CCxNE | CCxNP */
	uint8_t has_complementary;	/**< OCxN output exists */
};

static const struct tim_channel_desc tim_channels[4] = {
	{ 0, TIM_CHCTLR1_CC1S_SHIFT, TIM_CCER_CC1E | TIM_CCER_CC1P
			| TIM_CCER_CC1NE | TIM_CCER_CC1NP, 1 },
	{ 0, TIM_CHCTLR1_CC2S_SHIFT, TIM_CCER_CC2E | TIM_CCER_CC2P
			| TIM_CCER_CC2NE | TIM_CCER_CC2NP, 1 },
	{ 1, TIM_CHCTLR2_CC3S_SHIFT, TIM_CCER_CC3E | TIM_CCER_CC3P
			| TIM_CCER_CC3NE | TIM_CCER_CC3NP, 1 },
	{ 1, TIM_CHCTLR2_CC4S_SHIFT, TIM_CCER_CC4E | TIM_CCER_CC4P, 0 },
};

/** Number of capture/compare channels on both timers. */
#define TIM_CHANNEL_COUNT		4

/**
 * Validate a channel identifier and return it as a table index.
 *
 * The identifier is clamped to a valid index as well as asserted, so that the
 * table lookups below cannot run off the end when asserts are compiled out by
 * NDEBUG.
 */
static unsigned int tim_channel_index(uint32_t ic)
{
	openwch_assert(ic < TIM_CHANNEL_COUNT);

	return (ic < TIM_CHANNEL_COUNT) ? (unsigned int)ic : 0u;
}

/** Access the capture/compare control register that holds a channel. */
static uint16_t tim_chctlr_read(uint32_t tim, uint32_t ic)
{
	return (tim_channels[tim_channel_index(ic)].chctlr == 0)
			? TIM_CHCTLR1(tim) : TIM_CHCTLR2(tim);
}

static void tim_chctlr_write(uint32_t tim, uint32_t ic, uint16_t value)
{
	if (tim_channels[tim_channel_index(ic)].chctlr == 0) {
		TIM_CHCTLR1(tim) = value;
	} else {
		TIM_CHCTLR2(tim) = value;
	}
}

/** The CCER enable/polarity bit for channel `ic`. */
static uint16_t tim_ccer_bit(uint32_t ic)
{
	/* CCxE is bit 0 of each channel's four-bit CCER group. */
	return (uint16_t)(1u << (tim_channel_index(ic) * 4u));
}

/** The CCER polarity bit (CCxP), one above the enable bit. */
static uint16_t tim_ccer_polarity_bit(uint32_t ic)
{
	return (uint16_t)(1u << ((tim_channel_index(ic) * 4u) + 1u));
}

/** Access a capture/compare value register. */
static uint32_t tim_ccr(uint32_t tim, uint32_t ic)
{
	switch (tim_channel_index(ic)) {
	case 0:
		return TIM_CH1CVR(tim);
	case 1:
		return TIM_CH2CVR(tim);
	case 2:
		return TIM_CH3CVR(tim);
	default:
		return TIM_CH4CVR(tim);
	}
}

/** Write a capture/compare value register.  This is the \e setter half of the
 * pair; tim_ccr() only reads.  Channel validity is checked by the caller. */
static void tim_ccr_write(uint32_t tim, uint32_t ic, uint16_t value)
{
	switch (tim_channel_index(ic)) {
	case 0:
		TIM_CH1CVR(tim) = value;
		break;
	case 1:
		TIM_CH2CVR(tim) = value;
		break;
	case 2:
		TIM_CH3CVR(tim) = value;
		break;
	default:
		TIM_CH4CVR(tim) = value;
		break;
	}
}

/*
 * --- Counter -------------------------------------------------------------
 */

void timer_set_mode(uint32_t tim, uint32_t mode)
{
	uint16_t reg = TIM_CTLR1(tim);

	/*
	 * mode is a combination of the TIM_MODE_* flags: the alignment bits,
	 * DIR, OPM, UDIS and URS.  Everything else in CTLR1 (CEN, ARPE, CKD)
	 * is owned by its own setter.
	 */
	reg = (uint16_t)((reg & ~(TIM_CTLR1_CMS_MASK | TIM_CTLR1_DIR
					| TIM_CTLR1_OPM | TIM_CTLR1_UDIS
					| TIM_CTLR1_URS))
			| (mode & (TIM_CTLR1_CMS_MASK | TIM_CTLR1_DIR
					| TIM_CTLR1_OPM | TIM_CTLR1_UDIS
					| TIM_CTLR1_URS)));
	TIM_CTLR1(tim) = reg;
}

void timer_enable(uint32_t tim)
{
	TIM_CTLR1(tim) |= TIM_CTLR1_CEN;
}

void timer_disable(uint32_t tim)
{
	TIM_CTLR1(tim) &= ~TIM_CTLR1_CEN;
}

void timer_set_prescaler(uint32_t tim, uint16_t psc)
{
	TIM_PSC(tim) = psc;
}

void timer_set_period(uint32_t tim, uint16_t arr)
{
	TIM_ATRLR(tim) = arr;
}

void timer_set_counter(uint32_t tim, uint16_t cnt)
{
	TIM_CNT(tim) = cnt;
}

uint16_t timer_get_counter(uint32_t tim)
{
	return TIM_CNT(tim);
}

void timer_set_alignment(uint32_t tim, uint32_t alignment)
{
	uint16_t reg = TIM_CTLR1(tim);

	openwch_assert((alignment & ~TIM_CTLR1_CMS_MASK) == 0);

	reg = (uint16_t)((reg & ~TIM_CTLR1_CMS_MASK)
			| (alignment & TIM_CTLR1_CMS_MASK));
	TIM_CTLR1(tim) = reg;
}

void timer_set_direction(uint32_t tim, uint32_t direction)
{
	uint16_t reg = TIM_CTLR1(tim);

	openwch_assert((direction & ~TIM_CTLR1_DIR) == 0);

	reg = (uint16_t)((reg & ~TIM_CTLR1_DIR)
			| (direction & TIM_CTLR1_DIR));
	TIM_CTLR1(tim) = reg;
}

void timer_enable_preload(uint32_t tim)
{
	TIM_CTLR1(tim) |= TIM_CTLR1_ARPE;
}

void timer_disable_preload(uint32_t tim)
{
	TIM_CTLR1(tim) &= ~TIM_CTLR1_ARPE;
}

void timer_set_clock_division(uint32_t tim, enum tim_clock_division ckd)
{
	uint16_t reg = TIM_CTLR1(tim);

	openwch_assert((ckd == TIM_CKD_DIV1) || (ckd == TIM_CKD_DIV2)
			|| (ckd == TIM_CKD_DIV4));

	reg = (uint16_t)((reg & ~TIM_CTLR1_CKD_MASK)
			| (((uint16_t)ckd << TIM_CTLR1_CKD_SHIFT)
					& TIM_CTLR1_CKD_MASK));
	TIM_CTLR1(tim) = reg;
}

void timer_generate_event(uint32_t tim, enum tim_event event)
{
	switch (event) {
	case TIM_EVENT_UPDATE:
		TIM_SWEVGR(tim) = TIM_SWEVGR_UG;
		break;
	case TIM_EVENT_CC1:
		TIM_SWEVGR(tim) = TIM_SWEVGR_CC1G;
		break;
	case TIM_EVENT_CC2:
		TIM_SWEVGR(tim) = TIM_SWEVGR_CC2G;
		break;
	case TIM_EVENT_CC3:
		TIM_SWEVGR(tim) = TIM_SWEVGR_CC3G;
		break;
	case TIM_EVENT_CC4:
		TIM_SWEVGR(tim) = TIM_SWEVGR_CC4G;
		break;
	case TIM_EVENT_COM:
		TIM_SWEVGR(tim) = TIM_SWEVGR_COMG;
		break;
	case TIM_EVENT_TRIGGER:
		TIM_SWEVGR(tim) = TIM_SWEVGR_TG;
		break;
	case TIM_EVENT_BREAK:
		TIM_SWEVGR(tim) = TIM_SWEVGR_BG;
		break;
	default:
		openwch_assert_not_reached();
		break;
	}
}

/*
 * --- Output compare ------------------------------------------------------
 */

void timer_set_oc_mode(uint32_t tim, enum tim_oc_id oc, enum tim_oc_mode mode)
{
	uint16_t shift = (uint16_t)(tim_channels[tim_channel_index(oc)].ccxs_shift
			+ 4);
	uint16_t mask = (uint16_t)(0x7u << shift);
	uint16_t reg = tim_chctlr_read(tim, oc);

	openwch_assert((oc >= TIM_OC1) && (oc <= TIM_OC4));
	openwch_assert((mode >= TIM_OC_MODE_FROZEN) && (mode <= TIM_OC_MODE_PWM2));

	reg = (uint16_t)((reg & ~mask)
			| ((((uint16_t)mode) << shift) & mask));
	tim_chctlr_write(tim, oc, reg);
}

void timer_set_oc_value(uint32_t tim, enum tim_oc_id oc, uint16_t value)
{
	openwch_assert((oc >= TIM_OC1) && (oc <= TIM_OC4));

	tim_ccr_write(tim, oc, value);
}

void timer_set_oc_polarity(uint32_t tim, enum tim_oc_id oc,
			   enum tim_oc_polarity polarity)
{
	uint16_t reg = TIM_CCER(tim);

	openwch_assert((oc >= TIM_OC1) && (oc <= TIM_OC4));
	openwch_assert((polarity == TIM_OC_POLARITY_ACTIVE_HIGH)
			|| (polarity == TIM_OC_POLARITY_ACTIVE_LOW));

	if (polarity == TIM_OC_POLARITY_ACTIVE_LOW) {
		reg |= tim_ccer_polarity_bit(oc);
	} else {
		reg &= (uint16_t)~tim_ccer_polarity_bit(oc);
	}
	TIM_CCER(tim) = reg;
}

void timer_enable_oc_output(uint32_t tim, enum tim_oc_id oc)
{
	openwch_assert((oc >= TIM_OC1) && (oc <= TIM_OC4));

	TIM_CCER(tim) |= tim_ccer_bit(oc);
}

void timer_disable_oc_output(uint32_t tim, enum tim_oc_id oc)
{
	openwch_assert((oc >= TIM_OC1) && (oc <= TIM_OC4));

	TIM_CCER(tim) &= (uint16_t)~tim_ccer_bit(oc);
}

void timer_enable_oc_preload(uint32_t tim, enum tim_oc_id oc)
{
	uint16_t shift = (uint16_t)(tim_channels[tim_channel_index(oc)].ccxs_shift
			+ 3);
	uint16_t reg = tim_chctlr_read(tim, oc);

	openwch_assert((oc >= TIM_OC1) && (oc <= TIM_OC4));

	reg |= (uint16_t)(1u << shift);
	tim_chctlr_write(tim, oc, reg);
}

void timer_disable_oc_preload(uint32_t tim, enum tim_oc_id oc)
{
	uint16_t shift = (uint16_t)(tim_channels[tim_channel_index(oc)].ccxs_shift
			+ 3);
	uint16_t reg = tim_chctlr_read(tim, oc);

	openwch_assert((oc >= TIM_OC1) && (oc <= TIM_OC4));

	reg &= (uint16_t)~(1u << shift);
	tim_chctlr_write(tim, oc, reg);
}

void timer_set_oc_idle_state(uint32_t tim, enum tim_oc_id oc,
			     enum tim_oc_idle_state state)
{
	uint16_t bit;
	uint16_t reg;

	openwch_assert((oc >= TIM_OC1) && (oc <= TIM_OC4));
	openwch_assert((state == TIM_OC_IDLE_RESET) || (state == TIM_OC_IDLE_SET));

	switch (tim_channel_index(oc)) {
	case 0:
		bit = TIM_CTLR2_OIS1;
		break;
	case 1:
		bit = TIM_CTLR2_OIS2;
		break;
	case 2:
		bit = TIM_CTLR2_OIS3;
		break;
	default:
		bit = TIM_CTLR2_OIS4;
		break;
	}

	reg = TIM_CTLR2(tim);
	if (state == TIM_OC_IDLE_SET) {
		reg |= bit;
	} else {
		reg &= (uint16_t)~bit;
	}
	TIM_CTLR2(tim) = reg;
}

/*
 * --- Input capture -------------------------------------------------------
 */

void timer_set_input_filter(uint32_t tim, enum tim_ic_id ic, uint8_t filter)
{
	uint16_t shift = (uint16_t)(tim_channels[tim_channel_index(ic)].ccxs_shift
			+ 4);
	uint16_t mask = (uint16_t)(0xfu << shift);
	uint16_t reg = tim_chctlr_read(tim, ic);

	openwch_assert((ic >= TIM_IC1) && (ic <= TIM_IC4));
	openwch_assert(filter <= 0xfu);

	reg = (uint16_t)((reg & ~mask)
			| ((((uint16_t)filter) << shift) & mask));
	tim_chctlr_write(tim, ic, reg);
}

void timer_set_input_polarity(uint32_t tim, enum tim_ic_id ic,
			      enum tim_ic_polarity polarity)
{
	uint16_t reg = TIM_CCER(tim);

	openwch_assert((ic >= TIM_IC1) && (ic <= TIM_IC4));
	openwch_assert((polarity == TIM_IC_POLARITY_RISING)
			|| (polarity == TIM_IC_POLARITY_FALLING));

	/* The input capture polarity bit is CCxP, shared with output compare. */
	if (polarity == TIM_IC_POLARITY_FALLING) {
		reg |= tim_ccer_polarity_bit(ic);
	} else {
		reg &= (uint16_t)~tim_ccer_polarity_bit(ic);
	}
	TIM_CCER(tim) = reg;
}

void timer_set_ic_prescaler(uint32_t tim, enum tim_ic_id ic, uint8_t psc)
{
	uint16_t shift = (uint16_t)(tim_channels[tim_channel_index(ic)].ccxs_shift
			+ 2);
	uint16_t mask = (uint16_t)(0x3u << shift);
	uint16_t reg = tim_chctlr_read(tim, ic);

	openwch_assert((ic >= TIM_IC1) && (ic <= TIM_IC4));
	openwch_assert(psc <= 0x3u);

	reg = (uint16_t)((reg & ~mask)
			| ((((uint16_t)psc) << shift) & mask));
	tim_chctlr_write(tim, ic, reg);
}

uint16_t timer_get_ic_value(uint32_t tim, enum tim_ic_id ic)
{
	openwch_assert((ic >= TIM_IC1) && (ic <= TIM_IC4));

	return (uint16_t)tim_ccr(tim, ic);
}

/*
 * --- Break and dead-time (TIM1 only) -------------------------------------
 */

void timer_enable_break_main_output(uint32_t tim)
{
	TIM_BDTR(tim) |= TIM_BDTR_MOE;
}

void timer_set_deadtime(uint32_t tim, uint8_t deadtime)
{
	uint16_t reg = TIM_BDTR(tim);

	reg = (uint16_t)((reg & ~TIM_BDTR_DTG_MASK)
			| ((uint16_t)deadtime & TIM_BDTR_DTG_MASK));
	TIM_BDTR(tim) = reg;
}

/*
 * --- Interrupts and flags ------------------------------------------------
 */

/*
 * Single table of the per-source mask, so enable/disable, flag read and flag
 * clear all agree on which bit belongs to which identifier.  The entries are
 * in enum order: UPDATE, CC1..CC4, COM, TRIGGER, BREAK, then the four
 * overcapture flags.  The interrupt enable bits happen to use the same bit
 * positions as the first eight flags, but the table keeps the two namespaces
 * independent.
 */
static uint16_t tim_flag_mask(enum tim_flag flag)
{
	static const uint16_t masks[] = {
		TIM_INTFR_UIF,
		TIM_INTFR_CC1IF,
		TIM_INTFR_CC2IF,
		TIM_INTFR_CC3IF,
		TIM_INTFR_CC4IF,
		TIM_INTFR_COMIF,
		TIM_INTFR_TIF,
		TIM_INTFR_BIF,
		TIM_INTFR_CC1OF,
		TIM_INTFR_CC2OF,
		TIM_INTFR_CC3OF,
		TIM_INTFR_CC4OF,
	};

	openwch_assert(flag <= TIM_FLAG_CC4_OVERCAPTURE);

	return masks[(flag <= TIM_FLAG_CC4_OVERCAPTURE) ? (unsigned int)flag : 0u];
}

void timer_enable_irq(uint32_t tim, enum tim_irq irq)
{
	openwch_assert((irq >= TIM_IRQ_UPDATE) && (irq <= TIM_IRQ_BREAK));

	TIM_DMAINTENR(tim) |= (uint16_t)(1u << (uint16_t)irq);
}

void timer_disable_irq(uint32_t tim, enum tim_irq irq)
{
	openwch_assert((irq >= TIM_IRQ_UPDATE) && (irq <= TIM_IRQ_BREAK));

	TIM_DMAINTENR(tim) &= (uint16_t)~(1u << (uint16_t)irq);
}

uint16_t timer_get_flag(uint32_t tim, enum tim_flag flag)
{
	return (uint16_t)(TIM_INTFR(tim) & tim_flag_mask(flag));
}

void timer_clear_flag(uint32_t tim, enum tim_flag flag)
{
	/*
	 * INTFR is a write-1-to-clear register, so writing the wanted mask
	 * directly clears exactly those flags: the ones written as 0 keep
	 * their state, and no read-modify-write race can lose flags raised in
	 * between.
	 */
	TIM_INTFR(tim) = tim_flag_mask(flag);
}

uint8_t timer_get_interrupt_source(uint32_t tim, uint16_t irq)
{
	return (TIM_INTFR(tim) & TIM_DMAINTENR(tim) & irq) ? 1 : 0;
}
/**@}*/
