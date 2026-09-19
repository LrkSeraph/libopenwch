/** @addtogroup i2c_defines I2C Defines

@brief <b>Defined Constants and Types for the CH58x I2C</b>

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA I2C.H */

/** @cond */
#if defined(LIBOPENWCH_I2C_H) || defined(LIBOPENWCH_I2C_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_I2C_COMMON_V1_H
#define LIBOPENWCH_I2C_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * The CH58x I2C is the familiar STM32-style block with a single instance at
 * 0x40004800, but the CH58x spaces its 16-bit registers out to 32-bit
 * boundaries, so every register is an MMIO16 at 4 * n.
 *
 * Two details differ from the CH32V00x port of the same block:
 *
 *   - the maximum rise time lives in its own register (R16_I2C_RTR) instead
 *     of sharing CKCFGR with the clock divider;
 *   - OADDR1 bit 14 (RB_I2C_MUST1) must always be written as one, in both
 *     7- and 10-bit addressing modes.
 *
 * The peripheral is clocked straight from the system clock, which is what
 * R16_I2C_CTRL2's FREQ field must be told in whole MHz before CKCFGR can be
 * programmed.  FREQ is six bits wide, so the driver can describe a system
 * clock of 2..63 MHz; i2c_set_clock_frequency() derives the value from
 * clk_get_sys_clock().
 *
 * These registers are plain read/write, NOT RWA: no safe-access window is
 * needed.
 */

/* --- Register accessors -------------------------------------------------- */

/* Control register 1 (I2C_CTRL1) */
#define I2C_CTRL1(i2c) MMIO16((i2c) + 0x00)
/* Control register 2 (I2C_CTRL2) */
#define I2C_CTRL2(i2c) MMIO16((i2c) + 0x04)
/* Own address register 1 (I2C_OADDR1) */
#define I2C_OADDR1(i2c) MMIO16((i2c) + 0x08)
/* Own address register 2 (I2C_OADDR2) */
#define I2C_OADDR2(i2c) MMIO16((i2c) + 0x0c)
/* Data register (I2C_DATAR) */
#define I2C_DATAR(i2c) MMIO16((i2c) + 0x10)
/* Status register 1 (I2C_STAR1) */
#define I2C_STAR1(i2c) MMIO16((i2c) + 0x14)
/* Status register 2 (I2C_STAR2) */
#define I2C_STAR2(i2c) MMIO16((i2c) + 0x18)
/* Packet error checking register (I2C_PEC); the high byte of STAR2 */
#define I2C_PEC(i2c) MMIO8((i2c) + 0x19)
/* Clock control register (I2C_CKCFGR) */
#define I2C_CKCFGR(i2c) MMIO16((i2c) + 0x1c)
/* Rise time register (I2C_RTR) */
#define I2C_RTR(i2c) MMIO16((i2c) + 0x20)

/* --- CTRL1 bits ---------------------------------------------------------- */

#define RB_I2C_PE 0x0001	/**< peripheral enable */
#define RB_I2C_SMBUS 0x0002	/**< SMBus mode */
#define RB_I2C_SMBTYPE 0x0008	/**< SMBus host type */
#define RB_I2C_EBARP 0x0010	/**< ARP enable */
#define RB_I2C_ENPEC 0x0020	/**< PEC calculation enable */
#define RB_I2C_ENGC 0x0040	/**< general call enable */
#define RB_I2C_NOSTRETCH 0x0080 /**< clock stretching disable (slave) */
#define RB_I2C_START 0x0100	/**< start generation */
#define RB_I2C_STOP 0x0200	/**< stop generation */
#define RB_I2C_ACK 0x0400	/**< acknowledge enable */
#define RB_I2C_POS 0x0800	/**< acknowledge/PEC position */
#define RB_I2C_PEC 0x1000	/**< PEC transfer */
#define RB_I2C_ALERT 0x2000	/**< SMBus alert pin */
#define RB_I2C_SWRST 0x8000	/**< software reset */

/* --- CTRL2 bits ---------------------------------------------------------- */

#define RB_I2C_FREQ 0x003f    /**< FREQ[5:0], peripheral clock in MHz */
#define RB_I2C_ITERREN 0x0100 /**< error interrupt enable */
#define RB_I2C_ITEVTEN 0x0200 /**< event interrupt enable */
#define RB_I2C_ITBUFEN 0x0400 /**< buffer interrupt enable */
/** The interrupt enable bits of CTRL2. */
#define I2C_CTRL2_IT_MASK (RB_I2C_ITERREN | RB_I2C_ITEVTEN | RB_I2C_ITBUFEN)

/* --- OADDR1 bits --------------------------------------------------------- */

#define RB_I2C_ADD0 0x0001    /**< address bit 0 in 10-bit mode */
#define RB_I2C_ADD7_1 0x00fe  /**< address bits [7:1] */
#define RB_I2C_ADD9_8 0x0300  /**< address bits [9:8] in 10-bit mode */
#define RB_I2C_MUST1 0x4000   /**< reserved, always keep at 1 */
#define RB_I2C_ADDMODE 0x8000 /**< 10-bit addressing mode */

/* --- OADDR2 bits --------------------------------------------------------- */

#define RB_I2C_ENDUAL 0x0001 /**< dual addressing enable */
#define RB_I2C_ADD2 0x00fe   /**< second address [7:1] */

/* --- DATAR --------------------------------------------------------------- */

#define I2C_DATAR_MASK 0x00ffu

/* --- STAR1 bits ---------------------------------------------------------- */

#define RB_I2C_SB 0x0001       /**< start bit generated (master) */
#define RB_I2C_ADDR 0x0002     /**< address sent / matched */
#define RB_I2C_BTF 0x0004      /**< byte transfer finished */
#define RB_I2C_ADD10 0x0008    /**< 10-bit header sent (master) */
#define RB_I2C_STOPF 0x0010    /**< stop detected (slave) */
#define RB_I2C_RxNE 0x0040     /**< data register not empty */
#define RB_I2C_TxE 0x0080      /**< data register empty */
#define RB_I2C_BERR 0x0100     /**< bus error */
#define RB_I2C_ARLO 0x0200     /**< arbitration lost */
#define RB_I2C_AF 0x0400       /**< acknowledge failure */
#define RB_I2C_OVR 0x0800      /**< overrun / underrun */
#define RB_I2C_PECERR 0x1000   /**< PEC error in reception */
#define RB_I2C_TIMEOUT 0x4000  /**< timeout / Tlow error */
#define RB_I2C_SMBALERT 0x8000 /**< SMBus alert */

/*
 * The write-zero-to-clear flags of STAR1.  SB, ADDR, BTF and STOPF are
 * retired by reading STAR1 and then STAR2, which i2c_clear_flag() does, so
 * only these are written back.
 */
#define I2C_STAR1_CLEAR_MASK                                                   \
	(RB_I2C_BERR | RB_I2C_ARLO | RB_I2C_AF | RB_I2C_OVR | RB_I2C_PECERR |  \
	 RB_I2C_TIMEOUT | RB_I2C_SMBALERT)
/** Every flag bit STAR1 defines. */
#define I2C_STAR1_FLAG_MASK                                                    \
	(RB_I2C_SB | RB_I2C_ADDR | RB_I2C_BTF | RB_I2C_ADD10 | RB_I2C_STOPF |  \
	 RB_I2C_RxNE | RB_I2C_TxE | RB_I2C_BERR | RB_I2C_ARLO | RB_I2C_AF |    \
	 RB_I2C_OVR | RB_I2C_PECERR | RB_I2C_TIMEOUT | RB_I2C_SMBALERT)

/* --- STAR2 bits ---------------------------------------------------------- */

#define RB_I2C_MSL 0x0001	 /**< 0 = slave, 1 = master */
#define RB_I2C_BUSY 0x0002	 /**< bus busy */
#define RB_I2C_TRA 0x0004	 /**< 0 = receiver, 1 = transmitter */
#define RB_I2C_GENCALL 0x0010	 /**< general call received (slave) */
#define RB_I2C_SMBDEFAULT 0x0020 /**< SMBus default address (slave) */
#define RB_I2C_SMBHOST 0x0040	 /**< SMBus host header (slave) */
#define RB_I2C_DUALF 0x0080	 /**< dual flag (slave) */
#define RB_I2C_PECX 0xff00	 /**< PEC[7:0] */

/** Every flag bit STAR2 defines. */
#define I2C_STAR2_FLAG_MASK                                                    \
	(RB_I2C_MSL | RB_I2C_BUSY | RB_I2C_TRA | RB_I2C_GENCALL |              \
	 RB_I2C_SMBDEFAULT | RB_I2C_SMBHOST | RB_I2C_DUALF | RB_I2C_PECX)

/* --- CKCFGR bits --------------------------------------------------------- */

#define RB_I2C_CCR 0x0fff  /**< CCR[11:0], SCL divider */
#define RB_I2C_DUTY 0x4000 /**< fast mode duty cycle 16/9 */
#define RB_I2C_F_S 0x8000  /**< 0 = standard mode, 1 = fast mode */

/* --- RTR bits ------------------------------------------------------------ */

#define RB_I2C_TRISE 0x003f /**< TRISE[5:0], maximum rise time */

/* --- Bus speeds ---------------------------------------------------------- */

/** @defgroup i2c_speed I2C Bus Speeds
@ingroup i2c_defines
@{*/
#define I2C_SPEED_STANDARD 100000u /**< 100 kHz */
#define I2C_SPEED_FAST 400000u	   /**< 400 kHz */
/**@}*/

/** Lowest peripheral clock the FREQ field can describe, in Hz. */
#define I2C_MIN_CLOCK_FREQUENCY 2000000u
/**
 * Highest peripheral clock the FREQ field can describe, in Hz.  FREQ is six
 * bits, so the ceiling is 63 MHz; the reference manual quotes a lower
 * "maximum" of 36 MHz, but WCH's own examples clock the block from a 60 MHz
 * PLL and the divider arithmetic is exact either way.
 */
#define I2C_MAX_CLOCK_FREQUENCY 63000000u

/* --- Interrupt sources --------------------------------------------------- */

/** @defgroup i2c_interrupt I2C Interrupt Sources
@ingroup i2c_defines

Masks for i2c_enable_interrupt() / i2c_disable_interrupt().
@{*/
#define I2C_IT_ERR RB_I2C_ITERREN
#define I2C_IT_EVT RB_I2C_ITEVTEN
#define I2C_IT_BUF RB_I2C_ITBUFEN
/**@}*/

/* --- Flags --------------------------------------------------------------- */

/** @defgroup i2c_flags I2C Status Flags
@ingroup i2c_defines

The flag set accepted by i2c_get_flag() and i2c_clear_flag().  STAR1 and
STAR2 bit positions overlap in value, so i2c_get_flag() reads both registers
and ORs whatever the mask selects; i2c_clear_flag() only acts on the STAR1
bits that are actually clearable by a write.
@{*/
#define I2C_FLAG_SB RB_I2C_SB
#define I2C_FLAG_ADDR RB_I2C_ADDR
#define I2C_FLAG_BTF RB_I2C_BTF
#define I2C_FLAG_ADD10 RB_I2C_ADD10
#define I2C_FLAG_STOPF RB_I2C_STOPF
#define I2C_FLAG_RXNE RB_I2C_RxNE
#define I2C_FLAG_TXE RB_I2C_TxE
#define I2C_FLAG_BERR RB_I2C_BERR
#define I2C_FLAG_ARLO RB_I2C_ARLO
#define I2C_FLAG_AF RB_I2C_AF
#define I2C_FLAG_OVR RB_I2C_OVR
#define I2C_FLAG_PECERR RB_I2C_PECERR
#define I2C_FLAG_TIMEOUT RB_I2C_TIMEOUT
#define I2C_FLAG_SMBALERT RB_I2C_SMBALERT
#define I2C_FLAG_MSL RB_I2C_MSL
#define I2C_FLAG_BUSY RB_I2C_BUSY
#define I2C_FLAG_TRA RB_I2C_TRA
#define I2C_FLAG_GENCALL RB_I2C_GENCALL
#define I2C_FLAG_SMBDEFAULT RB_I2C_SMBDEFAULT
#define I2C_FLAG_SMBHOST RB_I2C_SMBHOST
#define I2C_FLAG_DUALF RB_I2C_DUALF
#define I2C_FLAG_PEC RB_I2C_PECX
/** Every bit i2c_get_flag() understands. */
#define I2C_FLAG_MASK (I2C_STAR1_FLAG_MASK | I2C_STAR2_FLAG_MASK)
/**@}*/

BEGIN_DECLS

/* --- Configuration ------------------------------------------------------- */

void i2c_init_master(uint32_t i2c, uint32_t speed);
void i2c_init_slave(uint32_t i2c, uint8_t address);
void i2c_set_clock_frequency(uint32_t i2c);

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

/* --- Acknowledge and PEC ------------------------------------------------- */

void i2c_enable_ack(uint32_t i2c);
void i2c_disable_ack(uint32_t i2c);
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
