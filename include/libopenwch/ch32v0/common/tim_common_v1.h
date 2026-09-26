/** @addtogroup tim_defines TIM Defines

@brief <b>Defined Constants and Types for the CH32V00x TIM</b>

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA TIM.H
The order of header inclusion is important: tim.h includes the device
specific memorymap.h header before including this header file. */

/** @cond */
#if defined(LIBOPENWCH_TIM_H) || defined(LIBOPENWCH_TIM_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_TIM_COMMON_V1_H
#define LIBOPENWCH_TIM_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * The CH32V00x TIM blocks are the classic STM32-style 16-bit timers, but the
 * CH32 version spaces the 16-bit registers out to 32-bit boundaries.  Every
 * register is therefore accessed as MMIO16 at an offset of 4 * n, except the
 * four capture/compare value registers, which the reference manual documents
 * (and the part decodes) as 32-bit.
 *
 * TIM1 is the advanced-control timer: four channels, complementary outputs on
 * channels 1..3 and a break input.  TIM2 is a general-purpose timer with four
 * independent channels and no BDTR/break machinery -- the register offsets are
 * identical, so the same accessors drive both; writes to the advanced-only
 * registers on TIM2 are simply ignored by the hardware.
 */

/* --- Register accessors -------------------------------------------------- */

/* Control register 1 (TIMx_CTLR1) */
#define TIM_CTLR1(tim) MMIO16((tim) + 0x00)
/* Control register 2 (TIMx_CTLR2) */
#define TIM_CTLR2(tim) MMIO16((tim) + 0x04)
/* Slave mode control register (TIMx_SMCFGR) */
#define TIM_SMCFGR(tim) MMIO16((tim) + 0x08)
/* DMA/Interrupt enable register (TIMx_DMAINTENR) */
#define TIM_DMAINTENR(tim) MMIO16((tim) + 0x0c)
/* Interrupt flag register (TIMx_INTFR) */
#define TIM_INTFR(tim) MMIO16((tim) + 0x10)
/* Event generation register (TIMx_SWEVGR) */
#define TIM_SWEVGR(tim) MMIO16((tim) + 0x14)
/* Capture/Compare control register 1 (TIMx_CHCTLR1) */
#define TIM_CHCTLR1(tim) MMIO16((tim) + 0x18)
/* Capture/Compare control register 2 (TIMx_CHCTLR2) */
#define TIM_CHCTLR2(tim) MMIO16((tim) + 0x1c)
/* Capture/Compare enable register (TIMx_CCER) */
#define TIM_CCER(tim) MMIO16((tim) + 0x20)
/* Counter (TIMx_CNT) */
#define TIM_CNT(tim) MMIO16((tim) + 0x24)
/* Prescaler (TIMx_PSC) */
#define TIM_PSC(tim) MMIO16((tim) + 0x28)
/* Auto-reload register (TIMx_ATRLR) */
#define TIM_ATRLR(tim) MMIO16((tim) + 0x2c)
/* Repetition counter register (TIMx_RPTCR) */
#define TIM_RPTCR(tim) MMIO16((tim) + 0x30)
/* Capture/Compare register 1 (TIMx_CH1CVR) */
#define TIM_CH1CVR(tim) MMIO32((tim) + 0x34)
/* Capture/Compare register 2 (TIMx_CH2CVR) */
#define TIM_CH2CVR(tim) MMIO32((tim) + 0x38)
/* Capture/Compare register 3 (TIMx_CH3CVR) */
#define TIM_CH3CVR(tim) MMIO32((tim) + 0x3c)
/* Capture/Compare register 4 (TIMx_CH4CVR) */
#define TIM_CH4CVR(tim) MMIO32((tim) + 0x40)
/* Break and dead-time register (TIMx_BDTR) */
#define TIM_BDTR(tim) MMIO16((tim) + 0x44)
/* DMA control register (TIMx_DMACFGR) */
#define TIM_DMACFGR(tim) MMIO16((tim) + 0x48)
/* DMA address for full transfer (TIMx_DMAADR) */
#define TIM_DMAADR(tim) MMIO16((tim) + 0x4c)

/* --- CTLR1 bits ---------------------------------------------------------- */

#define TIM_CTLR1_CEN (1 << 0)	/**< counter enable */
#define TIM_CTLR1_UDIS (1 << 1) /**< update disable */
#define TIM_CTLR1_URS (1 << 2)	/**< update request source */
#define TIM_CTLR1_OPM (1 << 3)	/**< one pulse mode */
#define TIM_CTLR1_DIR (1 << 4)	/**< direction: 1 = down */
#define TIM_CTLR1_CMS_SHIFT 5
#define TIM_CTLR1_CMS_MASK (0x3u << 5) /**< center-aligned mode */
#define TIM_CTLR1_CMS_EDGE (0x0u << 5)
#define TIM_CTLR1_CMS_CENTER1 (0x1u << 5)
#define TIM_CTLR1_CMS_CENTER2 (0x2u << 5)
#define TIM_CTLR1_CMS_CENTER3 (0x3u << 5)
#define TIM_CTLR1_ARPE (1 << 7) /**< auto-reload preload enable */
#define TIM_CTLR1_CKD_SHIFT 8
#define TIM_CTLR1_CKD_MASK (0x3u << 8) /**< clock division */
#define TIM_CTLR1_CKD_DIV1 (0x0u << 8)
#define TIM_CTLR1_CKD_DIV2 (0x1u << 8)
#define TIM_CTLR1_CKD_DIV4 (0x2u << 8)

/* --- CTLR2 bits ---------------------------------------------------------- */

#define TIM_CTLR2_CCPC (1 << 0) /**< CC preloaded control */
#define TIM_CTLR2_CCUS (1 << 2) /**< CC control update selection */
#define TIM_CTLR2_CCDS (1 << 3) /**< CC DMA selection */
#define TIM_CTLR2_MMS_SHIFT 4
#define TIM_CTLR2_MMS_MASK (0x7u << 4) /**< master mode selection */
#define TIM_CTLR2_TI1S (1 << 7)	       /**< TI1 selection */
#define TIM_CTLR2_OIS1 (1 << 8)	       /**< output idle state 1 */
#define TIM_CTLR2_OIS1N (1 << 9)       /**< complementary idle state 1 */
#define TIM_CTLR2_OIS2 (1 << 10)       /**< output idle state 2 */
#define TIM_CTLR2_OIS2N (1 << 11)      /**< complementary idle state 2 */
#define TIM_CTLR2_OIS3 (1 << 12)       /**< output idle state 3 */
#define TIM_CTLR2_OIS3N (1 << 13)      /**< complementary idle state 3 */
#define TIM_CTLR2_OIS4 (1 << 14)       /**< output idle state 4 */

/* --- SMCFGR bits --------------------------------------------------------- */

#define TIM_SMCFGR_SMS_SHIFT 0
#define TIM_SMCFGR_SMS_MASK 0x7u /**< slave mode selection */
#define TIM_SMCFGR_TS_SHIFT 4
#define TIM_SMCFGR_TS_MASK (0x7u << 4) /**< trigger selection */
#define TIM_SMCFGR_MSM (1 << 7)	       /**< master/slave mode */
#define TIM_SMCFGR_ETF_SHIFT 8
#define TIM_SMCFGR_ETF_MASK (0xfu << 8) /**< external trigger filter */
#define TIM_SMCFGR_ETPS_SHIFT 12
#define TIM_SMCFGR_ETPS_MASK (0x3u << 12) /**< ext trigger prescaler */
#define TIM_SMCFGR_ECE (1 << 14)	  /**< external clock enable */
#define TIM_SMCFGR_ETP (1 << 15)	  /**< external trigger polarity */

/* --- DMAINTENR bits ------------------------------------------------------ */

#define TIM_DMAINTENR_UIE (1 << 0)    /**< update interrupt enable */
#define TIM_DMAINTENR_CC1IE (1 << 1)  /**< CC1 interrupt enable */
#define TIM_DMAINTENR_CC2IE (1 << 2)  /**< CC2 interrupt enable */
#define TIM_DMAINTENR_CC3IE (1 << 3)  /**< CC3 interrupt enable */
#define TIM_DMAINTENR_CC4IE (1 << 4)  /**< CC4 interrupt enable */
#define TIM_DMAINTENR_COMIE (1 << 5)  /**< COM interrupt enable */
#define TIM_DMAINTENR_TIE (1 << 6)    /**< trigger interrupt enable */
#define TIM_DMAINTENR_BIE (1 << 7)    /**< break interrupt enable */
#define TIM_DMAINTENR_UDE (1 << 8)    /**< update DMA request enable */
#define TIM_DMAINTENR_CC1DE (1 << 9)  /**< CC1 DMA request enable */
#define TIM_DMAINTENR_CC2DE (1 << 10) /**< CC2 DMA request enable */
#define TIM_DMAINTENR_CC3DE (1 << 11) /**< CC3 DMA request enable */
#define TIM_DMAINTENR_CC4DE (1 << 12) /**< CC4 DMA request enable */
#define TIM_DMAINTENR_COMDE (1 << 13) /**< COM DMA request enable */
#define TIM_DMAINTENR_TDE (1 << 14)   /**< trigger DMA request enable */

/* --- INTFR bits ---------------------------------------------------------- */

#define TIM_INTFR_UIF (1 << 0)	  /**< update interrupt flag */
#define TIM_INTFR_CC1IF (1 << 1)  /**< CC1 interrupt flag */
#define TIM_INTFR_CC2IF (1 << 2)  /**< CC2 interrupt flag */
#define TIM_INTFR_CC3IF (1 << 3)  /**< CC3 interrupt flag */
#define TIM_INTFR_CC4IF (1 << 4)  /**< CC4 interrupt flag */
#define TIM_INTFR_COMIF (1 << 5)  /**< COM interrupt flag */
#define TIM_INTFR_TIF (1 << 6)	  /**< trigger interrupt flag */
#define TIM_INTFR_BIF (1 << 7)	  /**< break interrupt flag */
#define TIM_INTFR_CC1OF (1 << 9)  /**< CC1 overcapture flag */
#define TIM_INTFR_CC2OF (1 << 10) /**< CC2 overcapture flag */
#define TIM_INTFR_CC3OF (1 << 11) /**< CC3 overcapture flag */
#define TIM_INTFR_CC4OF (1 << 12) /**< CC4 overcapture flag */

/** All interrupt flag bits; INTFR is cleared by writing a 0 to the bit. */
#define TIM_INTFR_IRQ_MASK                                                     \
	(TIM_INTFR_UIF | TIM_INTFR_CC1IF | TIM_INTFR_CC2IF | TIM_INTFR_CC3IF | \
	 TIM_INTFR_CC4IF | TIM_INTFR_COMIF | TIM_INTFR_TIF | TIM_INTFR_BIF)

/** All overcapture flag bits. */
#define TIM_INTFR_OVERCAPTURE_MASK                                             \
	(TIM_INTFR_CC1OF | TIM_INTFR_CC2OF | TIM_INTFR_CC3OF | TIM_INTFR_CC4OF)

/* --- SWEVGR bits --------------------------------------------------------- */

#define TIM_SWEVGR_UG (1 << 0)	 /**< update generation */
#define TIM_SWEVGR_CC1G (1 << 1) /**< CC1 generation */
#define TIM_SWEVGR_CC2G (1 << 2) /**< CC2 generation */
#define TIM_SWEVGR_CC3G (1 << 3) /**< CC3 generation */
#define TIM_SWEVGR_CC4G (1 << 4) /**< CC4 generation */
#define TIM_SWEVGR_COMG (1 << 5) /**< COM generation */
#define TIM_SWEVGR_TG (1 << 6)	 /**< trigger generation */
#define TIM_SWEVGR_BG (1 << 7)	 /**< break generation */

/* --- CCER bits (2 bits per channel, complementary only on 1..3) ---------- */

#define TIM_CCER_CC1E (1 << 0)	 /**< CC1 output enable */
#define TIM_CCER_CC1P (1 << 1)	 /**< CC1 polarity */
#define TIM_CCER_CC1NE (1 << 2)	 /**< CC1 complementary enable */
#define TIM_CCER_CC1NP (1 << 3)	 /**< CC1 complementary polarity */
#define TIM_CCER_CC2E (1 << 4)	 /**< CC2 output enable */
#define TIM_CCER_CC2P (1 << 5)	 /**< CC2 polarity */
#define TIM_CCER_CC2NE (1 << 6)	 /**< CC2 complementary enable */
#define TIM_CCER_CC2NP (1 << 7)	 /**< CC2 complementary polarity */
#define TIM_CCER_CC3E (1 << 8)	 /**< CC3 output enable */
#define TIM_CCER_CC3P (1 << 9)	 /**< CC3 polarity */
#define TIM_CCER_CC3NE (1 << 10) /**< CC3 complementary enable */
#define TIM_CCER_CC3NP (1 << 11) /**< CC3 complementary polarity */
#define TIM_CCER_CC4E (1 << 12)	 /**< CC4 output enable */
#define TIM_CCER_CC4P (1 << 13)	 /**< CC4 polarity */

/* --- Per-channel CHCTLR bit positions ------------------------------------ */

#define TIM_CHCTLR1_CC1S_SHIFT 0
#define TIM_CHCTLR1_CC1S_MASK 0x3u /**< CC1 selection */
#define TIM_CHCTLR1_OC1FE (1 << 2) /**< OC1 fast enable */
#define TIM_CHCTLR1_OC1PE (1 << 3) /**< OC1 preload enable */
#define TIM_CHCTLR1_OC1M_SHIFT 4
#define TIM_CHCTLR1_OC1M_MASK (0x7u << 4) /**< OC1 mode */
#define TIM_CHCTLR1_OC1CE (1 << 7)	  /**< OC1 clear enable */
#define TIM_CHCTLR1_CC2S_SHIFT 8
#define TIM_CHCTLR1_CC2S_MASK (0x3u << 8) /**< CC2 selection */
#define TIM_CHCTLR1_OC2FE (1 << 10)	  /**< OC2 fast enable */
#define TIM_CHCTLR1_OC2PE (1 << 11)	  /**< OC2 preload enable */
#define TIM_CHCTLR1_OC2M_SHIFT 12
#define TIM_CHCTLR1_OC2M_MASK (0x7u << 12) /**< OC2 mode */
#define TIM_CHCTLR1_OC2CE (1 << 15)	   /**< OC2 clear enable */

#define TIM_CHCTLR2_CC3S_SHIFT 0
#define TIM_CHCTLR2_CC3S_MASK 0x3u /**< CC3 selection */
#define TIM_CHCTLR2_OC3FE (1 << 2) /**< OC3 fast enable */
#define TIM_CHCTLR2_OC3PE (1 << 3) /**< OC3 preload enable */
#define TIM_CHCTLR2_OC3M_SHIFT 4
#define TIM_CHCTLR2_OC3M_MASK (0x7u << 4) /**< OC3 mode */
#define TIM_CHCTLR2_OC3CE (1 << 7)	  /**< OC3 clear enable */
#define TIM_CHCTLR2_CC4S_SHIFT 8
#define TIM_CHCTLR2_CC4S_MASK (0x3u << 8) /**< CC4 selection */
#define TIM_CHCTLR2_OC4FE (1 << 10)	  /**< OC4 fast enable */
#define TIM_CHCTLR2_OC4PE (1 << 11)	  /**< OC4 preload enable */
#define TIM_CHCTLR2_OC4M_SHIFT 12
#define TIM_CHCTLR2_OC4M_MASK (0x7u << 12) /**< OC4 mode */
#define TIM_CHCTLR2_OC4CE (1 << 15)	   /**< OC4 clear enable */

/* --- BDTR bits ----------------------------------------------------------- */

#define TIM_BDTR_DTG_SHIFT 0
#define TIM_BDTR_DTG_MASK 0x00ffu /**< dead-time generator */
#define TIM_BDTR_LOCK_SHIFT 8
#define TIM_BDTR_LOCK_MASK (0x3u << 8) /**< lock configuration */
#define TIM_BDTR_OSSI (1 << 10)	       /**< off-state selection, idle */
#define TIM_BDTR_OSSR (1 << 11)	       /**< off-state selection, run */
#define TIM_BDTR_BKE (1 << 12)	       /**< break enable */
#define TIM_BDTR_BKP (1 << 13)	       /**< break polarity */
#define TIM_BDTR_AOE (1 << 14)	       /**< automatic output enable */
#define TIM_BDTR_MOE (1 << 15)	       /**< main output enable */

/** Largest repetition counter value (8 bits). */
#define TIM_RPTCR_MAX 0xffu

/* --- Configuration identifiers ------------------------------------------- */

/** @defgroup tim_channel TIM Channel Identifiers
@ingroup tim_defines

@{*/
enum tim_channel {
	TIM_CH1 = 0, /**< channel 1 */
	TIM_CH2,     /**< channel 2 */
	TIM_CH3,     /**< channel 3 */
	TIM_CH4,     /**< channel 4 */
};
/**@}*/

/** @defgroup tim_oc_id TIM Output Compare Identifiers
@ingroup tim_defines

@{*/
enum tim_oc_id {
	TIM_OC1 = TIM_CH1, /**< output compare 1 */
	TIM_OC2,	   /**< output compare 2 */
	TIM_OC3,	   /**< output compare 3 */
	TIM_OC4,	   /**< output compare 4 */
};
/**@}*/

/** @defgroup tim_ic_id TIM Input Capture Identifiers
@ingroup tim_defines

@{*/
enum tim_ic_id {
	TIM_IC1 = TIM_CH1, /**< input capture 1 */
	TIM_IC2,	   /**< input capture 2 */
	TIM_IC3,	   /**< input capture 3 */
	TIM_IC4,	   /**< input capture 4 */
};
/**@}*/

/** @defgroup tim_mode TIM Mode Flags
@ingroup tim_defines

These are the CTLR1 configuration bits that select how the counter runs.
They are combined with a bitwise OR and passed to timer_set_mode().

@{*/
#define TIM_MODE_EDGE_ALIGNED 0x0000u /**< edge-aligned */
#define TIM_MODE_CENTER_ALIGNED1 TIM_CTLR1_CMS_CENTER1
#define TIM_MODE_CENTER_ALIGNED2 TIM_CTLR1_CMS_CENTER2
#define TIM_MODE_CENTER_ALIGNED3 TIM_CTLR1_CMS_CENTER3
#define TIM_MODE_UP 0x0000u /**< up-counting */
#define TIM_MODE_DOWN TIM_CTLR1_DIR
#define TIM_MODE_ONE_PULSE TIM_CTLR1_OPM /**< stop after one period */
#define TIM_MODE_UPDATE_DISABLE TIM_CTLR1_UDIS
#define TIM_MODE_UPDATE_OVERFLOW TIM_CTLR1_URS
/**@}*/

/** @defgroup tim_oc_mode TIM Output Compare Modes
@ingroup tim_defines

@{*/
enum tim_oc_mode {
	TIM_OC_MODE_FROZEN = 0,	    /**< compare has no effect on the output */
	TIM_OC_MODE_ACTIVE,	    /**< match forces the output active */
	TIM_OC_MODE_INACTIVE,	    /**< match forces the output inactive */
	TIM_OC_MODE_TOGGLE,	    /**< match toggles the output */
	TIM_OC_MODE_FORCE_INACTIVE, /**< output forced inactive */
	TIM_OC_MODE_FORCE_ACTIVE,   /**< output forced active */
	TIM_OC_MODE_PWM1,	    /**< PWM, active while CNT < CCR */
	TIM_OC_MODE_PWM2,	    /**< PWM, inactive while CNT < CCR */
};
/**@}*/

/** @defgroup tim_oc_polarity TIM Output Compare Polarity
@ingroup tim_defines

@{*/
enum tim_oc_polarity {
	TIM_OC_POLARITY_ACTIVE_HIGH = 0, /**< active high */
	TIM_OC_POLARITY_ACTIVE_LOW,	 /**< active low */
};
/**@}*/

/** @defgroup tim_input_polarity TIM Input Capture Polarity
@ingroup tim_defines

@{*/
enum tim_ic_polarity {
	TIM_IC_POLARITY_RISING = 0, /**< capture on rising edge */
	TIM_IC_POLARITY_FALLING,    /**< capture on falling edge */
};
/**@}*/

/** @defgroup tim_clock_division TIM Clock Division
@ingroup tim_defines

@{*/
enum tim_clock_division {
	TIM_CKD_DIV1 = 0, /**< tDTS = tCK_INT */
	TIM_CKD_DIV2,	  /**< tDTS = 2 * tCK_INT */
	TIM_CKD_DIV4,	  /**< tDTS = 4 * tCK_INT */
};
/**@}*/

/** @defgroup tim_event TIM Event Generation Identifiers
@ingroup tim_defines

@{*/
enum tim_event {
	TIM_EVENT_UPDATE = 0, /**< reinitialise the counter */
	TIM_EVENT_CC1,	      /**< capture/compare 1 */
	TIM_EVENT_CC2,	      /**< capture/compare 2 */
	TIM_EVENT_CC3,	      /**< capture/compare 3 */
	TIM_EVENT_CC4,	      /**< capture/compare 4 */
	TIM_EVENT_COM,	      /**< capture/compare control update */
	TIM_EVENT_TRIGGER,    /**< trigger */
	TIM_EVENT_BREAK,      /**< break */
};
/**@}*/

/** @defgroup tim_irq TIM Interrupt Identifiers
@ingroup tim_defines

@{*/
enum tim_irq {
	TIM_IRQ_UPDATE = 0, /**< update */
	TIM_IRQ_CC1,	    /**< capture/compare 1 */
	TIM_IRQ_CC2,	    /**< capture/compare 2 */
	TIM_IRQ_CC3,	    /**< capture/compare 3 */
	TIM_IRQ_CC4,	    /**< capture/compare 4 */
	TIM_IRQ_COM,	    /**< capture/compare control update */
	TIM_IRQ_TRIGGER,    /**< trigger */
	TIM_IRQ_BREAK,	    /**< break */
};
/**@}*/

/** @defgroup tim_flag TIM Flag Identifiers
@ingroup tim_defines

@{*/
enum tim_flag {
	TIM_FLAG_UPDATE = 0,	  /**< update occurred */
	TIM_FLAG_CC1,		  /**< CC1 occurred */
	TIM_FLAG_CC2,		  /**< CC2 occurred */
	TIM_FLAG_CC3,		  /**< CC3 occurred */
	TIM_FLAG_CC4,		  /**< CC4 occurred */
	TIM_FLAG_COM,		  /**< COM occurred */
	TIM_FLAG_TRIGGER,	  /**< trigger occurred */
	TIM_FLAG_BREAK,		  /**< break occurred */
	TIM_FLAG_CC1_OVERCAPTURE, /**< CC1 overcapture */
	TIM_FLAG_CC2_OVERCAPTURE, /**< CC2 overcapture */
	TIM_FLAG_CC3_OVERCAPTURE, /**< CC3 overcapture */
	TIM_FLAG_CC4_OVERCAPTURE, /**< CC4 overcapture */
};
/**@}*/

/** @defgroup tim_idle_state TIM Output Idle State
@ingroup tim_defines

@{*/
enum tim_oc_idle_state {
	TIM_OC_IDLE_RESET = 0, /**< idle output is inactive */
	TIM_OC_IDLE_SET,       /**< idle output is active */
};
/**@}*/

BEGIN_DECLS

/* --- Counter ------------------------------------------------------------- */

void timer_set_mode(uint32_t tim, uint32_t mode);
void timer_enable(uint32_t tim);
void timer_disable(uint32_t tim);
void timer_set_prescaler(uint32_t tim, uint16_t psc);
void timer_set_period(uint32_t tim, uint16_t arr);
void timer_set_counter(uint32_t tim, uint16_t cnt);
uint16_t timer_get_counter(uint32_t tim);
void timer_set_alignment(uint32_t tim, uint32_t alignment);
void timer_set_direction(uint32_t tim, uint32_t direction);
void timer_enable_preload(uint32_t tim);
void timer_disable_preload(uint32_t tim);
void timer_set_clock_division(uint32_t tim, enum tim_clock_division ckd);
void timer_generate_event(uint32_t tim, enum tim_event event);

/* --- Output compare ------------------------------------------------------ */

void timer_set_oc_mode(uint32_t tim, enum tim_oc_id oc, enum tim_oc_mode mode);
void timer_set_oc_value(uint32_t tim, enum tim_oc_id oc, uint16_t value);
void timer_set_oc_polarity(uint32_t tim,
			   enum tim_oc_id oc,
			   enum tim_oc_polarity polarity);
void timer_enable_oc_output(uint32_t tim, enum tim_oc_id oc);
void timer_disable_oc_output(uint32_t tim, enum tim_oc_id oc);
void timer_enable_oc_preload(uint32_t tim, enum tim_oc_id oc);
void timer_disable_oc_preload(uint32_t tim, enum tim_oc_id oc);
void timer_set_oc_idle_state(uint32_t tim,
			     enum tim_oc_id oc,
			     enum tim_oc_idle_state state);

/* --- Input capture ------------------------------------------------------- */

void timer_set_input_filter(uint32_t tim, enum tim_ic_id ic, uint8_t filter);
void timer_set_input_polarity(uint32_t tim,
			      enum tim_ic_id ic,
			      enum tim_ic_polarity polarity);
void timer_set_ic_prescaler(uint32_t tim, enum tim_ic_id ic, uint8_t psc);
uint16_t timer_get_ic_value(uint32_t tim, enum tim_ic_id ic);

/* --- Break and dead-time (TIM1 only) ------------------------------------- */

void timer_enable_break_main_output(uint32_t tim);
void timer_set_deadtime(uint32_t tim, uint8_t deadtime);

/* --- Interrupts and flags ------------------------------------------------ */

void timer_enable_irq(uint32_t tim, enum tim_irq irq);
void timer_disable_irq(uint32_t tim, enum tim_irq irq);
uint16_t timer_get_flag(uint32_t tim, enum tim_flag flag);
void timer_clear_flag(uint32_t tim, enum tim_flag flag);
uint8_t timer_get_interrupt_source(uint32_t tim, uint16_t irq);

END_DECLS

#endif
/** @cond */
#else
#warning "tim_common_v1.h should not be included explicitly, only via tim.h"
#endif
/** @endcond */
/**@}*/
