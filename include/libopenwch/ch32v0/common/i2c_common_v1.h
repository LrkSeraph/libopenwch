/** @addtogroup i2c_defines I2C Defines

@brief <b>Defined Constants and Types for the CH32V00x I2C</b>

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA I2C.H
The order of header inclusion is important: i2c.h includes the device
specific memorymap.h header before including this header file. */

/** @cond */
#if defined(LIBOPENWCH_I2C_H) || defined(LIBOPENWCH_I2C_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_I2C_COMMON_V1_H
#define LIBOPENWCH_I2C_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * The CH32V00x I2C is the classic STM32-style block, but the CH32 version
 * spaces the 16-bit registers out to 32-bit boundaries.  Every register is
 * therefore accessed as MMIO16 at an offset of 4 * n.
 *
 * The peripheral is a single instance (I2C1) and is clocked from APB1.  Its
 * timing is derived from the APB1 frequency, which CTLR2 must be told about
 * in whole MHz before CKCFGR can be programmed.
 */

/* --- Register accessors -------------------------------------------------- */

/* Control register 1 (I2Cx_CTLR1) */
#define I2C_CTLR1(i2c) MMIO16((i2c) + 0x00)
/* Control register 2 (I2Cx_CTLR2) */
#define I2C_CTLR2(i2c) MMIO16((i2c) + 0x04)
/* Own address register 1 (I2Cx_OADDR1) */
#define I2C_OADDR1(i2c) MMIO16((i2c) + 0x08)
/* Own address register 2 (I2Cx_OADDR2) */
#define I2C_OADDR2(i2c) MMIO16((i2c) + 0x0c)
/* Data register (I2Cx_DATAR) */
#define I2C_DATAR(i2c) MMIO16((i2c) + 0x10)
/* Status register 1 (I2Cx_STAR1) */
#define I2C_STAR1(i2c) MMIO16((i2c) + 0x14)
/* Status register 2 (I2Cx_STAR2) */
#define I2C_STAR2(i2c) MMIO16((i2c) + 0x18)
/* Clock control register (I2Cx_CKCFGR) */
#define I2C_CKCFGR(i2c) MMIO16((i2c) + 0x1c)

/* --- CTLR1 bits ---------------------------------------------------------- */

#define I2C_CTLR1_PE (1 << 0)	     /**< peripheral enable */
#define I2C_CTLR1_ENPEC (1 << 5)     /**< PEC enable */
#define I2C_CTLR1_ENGC (1 << 6)	     /**< general call enable */
#define I2C_CTLR1_NOSTRETCH (1 << 7) /**< clock stretching disable (slave) */
#define I2C_CTLR1_START (1 << 8)     /**< start generation */
#define I2C_CTLR1_STOP (1 << 9)	     /**< stop generation */
#define I2C_CTLR1_ACK (1 << 10)	     /**< acknowledge enable */
#define I2C_CTLR1_POS (1 << 11)	     /**< acknowledge/PEC position */
#define I2C_CTLR1_PEC (1 << 12)	     /**< packet error checking */
#define I2C_CTLR1_SWRST (1 << 15)    /**< software reset */

/* --- CTLR2 bits ---------------------------------------------------------- */

#define I2C_CTLR2_FREQ_SHIFT 0
#define I2C_CTLR2_FREQ_MASK 0x003fu /**< FREQ[5:0] */
#define I2C_CTLR2_ITERREN (1 << 8)  /**< error interrupt enable */
#define I2C_CTLR2_ITEVTEN (1 << 9)  /**< event interrupt enable */
#define I2C_CTLR2_ITBUFEN (1 << 10) /**< buffer interrupt enable */
#define I2C_CTLR2_DMAEN (1 << 11)   /**< DMA requests enable */
#define I2C_CTLR2_LAST (1 << 12)    /**< DMA last transfer */

/* Interrupt source mask used by i2c_enable_interrupt()/i2c_disable_interrupt(). */
#define I2C_CTLR2_IT_MASK                                                      \
	(I2C_CTLR2_ITERREN | I2C_CTLR2_ITEVTEN | I2C_CTLR2_ITBUFEN)

/* --- OADDR1 bits --------------------------------------------------------- */

#define I2C_OADDR1_ADD1_7 0x00feu    /**< interface address [7:1] */
#define I2C_OADDR1_ADD8_9 0x0300u    /**< interface address [9:8] */
#define I2C_OADDR1_ADDMODE (1 << 15) /**< 10-bit addressing mode */

/* --- OADDR2 bits --------------------------------------------------------- */

#define I2C_OADDR2_ENDUAL (1 << 0) /**< dual addressing enable */
#define I2C_OADDR2_ADD2 0x00feu	   /**< second address [7:1] */

/* --- DATAR --------------------------------------------------------------- */

#define I2C_DATAR_MASK 0x00ffu

/* --- STAR1 bits ---------------------------------------------------------- */

#define I2C_STAR1_SB (1 << 0)	   /**< start bit generated */
#define I2C_STAR1_ADDR (1 << 1)	   /**< address sent / matched */
#define I2C_STAR1_BTF (1 << 2)	   /**< byte transfer finished */
#define I2C_STAR1_ADD10 (1 << 3)   /**< 10-bit header sent (master) */
#define I2C_STAR1_STOPF (1 << 4)   /**< stop detected (slave) */
#define I2C_STAR1_RXNE (1 << 6)	   /**< data register not empty */
#define I2C_STAR1_TXE (1 << 7)	   /**< data register empty */
#define I2C_STAR1_BERR (1 << 8)	   /**< bus error */
#define I2C_STAR1_ARLO (1 << 9)	   /**< arbitration lost */
#define I2C_STAR1_AF (1 << 10)	   /**< acknowledge failure */
#define I2C_STAR1_OVR (1 << 11)	   /**< overrun / underrun */
#define I2C_STAR1_PECERR (1 << 12) /**< PEC error */

/*
 * The write-one-to-clear flags of STAR1.  The remaining flags are cleared by
 * the hardware as a side effect of reading STAR1 then STAR2 or of reading or
 * writing DATAR, so they cannot be cleared by an explicit write.
 */
#define I2C_STAR1_ERR_MASK                                                     \
	(I2C_STAR1_BERR | I2C_STAR1_ARLO | I2C_STAR1_AF | I2C_STAR1_OVR |      \
	 I2C_STAR1_PECERR)
/** Default (acknowledge) set of master receive flags. */
#define I2C_STAR1_MASTER_RECEIVE_ACK                                           \
	(I2C_STAR1_ADDR | I2C_STAR1_RXNE | I2C_STAR1_BTF)
/** Set of master receive flags for the final byte, which is NACKed. */
#define I2C_STAR1_MASTER_RECEIVE_NACK (I2C_STAR1_ADDR | I2C_STAR1_RXNE)

/* --- STAR2 bits ---------------------------------------------------------- */

#define I2C_STAR2_MSL (1 << 0)	   /**< master / slave */
#define I2C_STAR2_BUSY (1 << 1)	   /**< bus busy */
#define I2C_STAR2_TRA (1 << 2)	   /**< transmitter / receiver */
#define I2C_STAR2_GENCALL (1 << 4) /**< general call address */
#define I2C_STAR2_DUALF (1 << 7)   /**< dual flag */
#define I2C_STAR2_PEC_SHIFT 8
#define I2C_STAR2_PEC_MASK 0xff00u /**< PEC[7:0] */

/* --- CKCFGR bits --------------------------------------------------------- */

#define I2C_CKCFGR_CCR_MASK 0x0fffu /**< CCR[11:0] */
#define I2C_CKCFGR_DUTY (1 << 14)   /**< fast mode duty cycle */
#define I2C_CKCFGR_FS (1 << 15)	    /**< fast mode selection */

/** @defgroup i2c_duty_cycle I2C Fast Mode Duty Cycle
@ingroup i2c_defines

The two legal fast mode duty cycles.  Tlow/Thigh = 2 gives the full 400 kHz
at fPCLK1 = 36 MHz and above; ratio 16/9 trades maximum speed for a longer
low phase.  Pass one of these to i2c_init_master().
@{*/
#define I2C_CCR_DUTY_2 0x0u
#define I2C_CCR_DUTY_16_9 I2C_CKCFGR_DUTY
/**@}*/

/** @defgroup i2c_speed I2C Bus Speeds
@ingroup i2c_defines
@{*/
#define I2C_SPEED_STANDARD 100000u /**< 100 kHz */
#define I2C_SPEED_FAST 400000u	   /**< 400 kHz */
/**@}*/

/** Highest APB1 frequency the CTLR2 FREQ field can describe, in Hz. */
#define I2C_MAX_CLOCK_FREQUENCY 36000000u

/** @defgroup i2c_interrupt I2C Interrupt Sources
@ingroup i2c_defines

Masks for i2c_enable_interrupt()/i2c_disable_interrupt().
@{*/
#define I2C_IT_ERR I2C_CTLR2_ITERREN
#define I2C_IT_EVT I2C_CTLR2_ITEVTEN
#define I2C_IT_BUF I2C_CTLR2_ITBUFEN
/**@}*/

/** @defgroup i2c_flags I2C Status Flags
@ingroup i2c_defines

The flag set, as returned by i2c_get_flag().  Flags taken from STAR1 and
STAR2 are deliberately kept in separate groups because the two registers are
not contiguous in the same functional block; the values never collide.
@{*/
#define I2C_SR1_SB I2C_STAR1_SB
#define I2C_SR1_ADDR I2C_STAR1_ADDR
#define I2C_SR1_BTF I2C_STAR1_BTF
#define I2C_SR1_ADD10 I2C_STAR1_ADD10
#define I2C_SR1_STOPF I2C_STAR1_STOPF
#define I2C_SR1_RXNE I2C_STAR1_RXNE
#define I2C_SR1_TXE I2C_STAR1_TXE
#define I2C_SR1_BERR I2C_STAR1_BERR
#define I2C_SR1_ARLO I2C_STAR1_ARLO
#define I2C_SR1_AF I2C_STAR1_AF
#define I2C_SR1_OVR I2C_STAR1_OVR
#define I2C_SR1_PECERR I2C_STAR1_PECERR
#define I2C_SR2_MSL I2C_STAR2_MSL
#define I2C_SR2_BUSY I2C_STAR2_BUSY
#define I2C_SR2_TRA I2C_STAR2_TRA
#define I2C_SR2_GENCALL I2C_STAR2_GENCALL
#define I2C_SR2_DUALF I2C_STAR2_DUALF
/**@}*/

BEGIN_DECLS

/* --- Configuration ------------------------------------------------------- */

void i2c_init_master(uint32_t i2c,
		     uint32_t clock,
		     uint32_t speed,
		     uint32_t duty_cycle);
void i2c_init_slave(uint32_t i2c, uint32_t clock, uint8_t address);

/* --- Enable -------------------------------------------------------------- */

void i2c_enable(uint32_t i2c);
void i2c_disable(uint32_t i2c);

/* --- Bus control --------------------------------------------------------- */

void i2c_send_start(uint32_t i2c);
void i2c_send_stop(uint32_t i2c);
void i2c_send_data(uint32_t i2c, uint8_t data);
uint8_t i2c_read_data(uint32_t i2c);
void i2c_send_7bit_address(uint32_t i2c, uint8_t address, uint8_t read);

/* --- Own address --------------------------------------------------------- */

void i2c_set_own_7bit_address(uint32_t i2c, uint8_t address);
void i2c_set_own_10bit_address(uint32_t i2c, uint16_t address);
void i2c_enable_dual_address(uint32_t i2c, uint8_t address);
void i2c_enable_general_call(uint32_t i2c);

/* --- Clock generation ---------------------------------------------------- */

void i2c_set_clock_frequency(uint32_t i2c, uint32_t clock);
void i2c_set_ccr(uint32_t i2c, uint32_t ccr);
void i2c_set_trise(uint32_t i2c, uint32_t trise);

/* --- Acknowledge and PEC ------------------------------------------------- */

void i2c_enable_ack(uint32_t i2c);
void i2c_disable_ack(uint32_t i2c);
void i2c_nack_current(uint32_t i2c);
void i2c_nack_next(uint32_t i2c);
void i2c_enable_pec(uint32_t i2c);
void i2c_disable_pec(uint32_t i2c);
uint8_t i2c_get_pec(uint32_t i2c);

/* --- Reset --------------------------------------------------------------- */

void i2c_software_reset(uint32_t i2c);

/* --- Interrupts ---------------------------------------------------------- */

void i2c_enable_interrupt(uint32_t i2c, uint32_t interrupt);
void i2c_disable_interrupt(uint32_t i2c, uint32_t interrupt);

/* --- Flags and interrupt status ------------------------------------------ */

uint16_t i2c_get_flag(uint32_t i2c, uint32_t flag);
void i2c_clear_flag(uint32_t i2c, uint32_t flag);
uint16_t i2c_get_interrupt_status(uint32_t i2c);
void i2c_clear_interrupt_pending_bit(uint32_t i2c);

END_DECLS

#endif
/** @cond */
#else
#warning "i2c_common_v1.h should not be included explicitly, only via i2c.h"
#endif
/** @endcond */
/**@}*/
