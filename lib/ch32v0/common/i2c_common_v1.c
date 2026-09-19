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

/** @addtogroup i2c_file I2C
 *
 * @ingroup CH32V0
 *
 * @brief <b>Inter-Integrated Circuit interface for the CH32V00x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * The CH32V00x I2C is the STM32-style block clocked from APB1.  Its timing is
 * derived entirely from the APB1 frequency, so the driver must be told that
 * frequency before the clock control register can be programmed:
 *
 *	CTLR2.FREQ = fPCLK1 / 1MHz
 *
 * In standard mode (100 kHz) the divider is
 *
 *	CCR = fPCLK1 / (2 * speed)
 *
 * and in fast mode (400 kHz) it is
 *
 *	CCR = fPCLK1 / (3 * speed)		for Tlow/Thigh = 2
 *	CCR = fPCLK1 / (25 * speed)		for Tlow/Thigh = 16/9
 *
 * The maximum rise time follows the reference manual: 1000 ns for standard
 * mode and 300 ns for fast mode, both expressed in APB1 periods:
 *
 *	TRISE = fPCLK1[MHz] + 1			standard
 *	TRISE = fPCLK1[MHz] * 300 / 1000 + 1	fast
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch32v0/i2c.h>
#include <libopenwch/ch32v0/rcc.h>
#include <libopenwch/qingke/assert.h>

/* Division constants of the CKCFGR CCR field, in units of CCR per Hz. */
#define I2C_CCR_DIV_STANDARD		2u
#define I2C_CCR_DIV_FAST_DUTY_2		3u
#define I2C_CCR_DIV_FAST_DUTY_16_9	25u

/** Largest value the 12-bit CCR field can represent. */
#define I2C_CCR_MAX			0x0fffu

/** Maximum rise time, in ns, for each speed. */
#define I2C_RISE_TIME_STANDARD_NS	1000u
#define I2C_RISE_TIME_FAST_NS		300u

/** One MHz, the unit of the CTLR2 FREQ field and of the TRISE calculation. */
#define I2C_MHZ				1000000u

/** Nanoseconds in one second; the rise times are expressed in ns. */
#define I2C_NS_PER_SECOND		1000000000u

/* --- Internal helpers ---------------------------------------------------- */

/*
 * Round-to-nearest integer division.  QingKe V2 has no hardware divide, so
 * this ends up in libgcc; that is expected and fine.
 */
static uint32_t i2c_div_round(uint32_t dividend, uint32_t divisor) {
	return (dividend + (divisor / 2u)) / divisor;
}

/*
 * TRISE is the maximum tolerated SCL rise time expressed in APB1 periods,
 * plus one:
 *
 *	TRISE = (rise_ns * fPCLK1 / 1e9) + 1
 *
 * Dividing the clock down to kHz first would lose too much precision at the
 * low APB1 frequencies the part supports, so the whole product is kept.
 */
static void i2c_set_rise_time(uint32_t i2c, uint32_t clock, uint32_t speed) {
	uint32_t rise_ns;

	if (speed > I2C_SPEED_STANDARD) {
		rise_ns = I2C_RISE_TIME_FAST_NS;
	} else {
		rise_ns = I2C_RISE_TIME_STANDARD_NS;
	}

	I2C_CKCFGR(i2c) = (uint16_t)((I2C_CKCFGR(i2c) & ~I2C_CKCFGR_CCR_MASK)
			| (i2c_div_round(clock, I2C_NS_PER_SECOND / rise_ns) + 1u));
}

/* --- Configuration ------------------------------------------------------- */

void i2c_set_clock_frequency(uint32_t i2c, uint32_t clock) {
	uint32_t freq_mhz;

	openwch_assert(clock != 0);
	openwch_assert(clock <= I2C_MAX_CLOCK_FREQUENCY);

	/*
	 * CTLR2 may only be written while the peripheral is disabled; the FREQ
	 * field is not used by the hardware otherwise, and re-writing it with
	 * the block enabled would restart the analogue filters.
	 */
	openwch_assert((I2C_CTLR1(i2c) & I2C_CTLR1_PE) == 0);

	freq_mhz = clock / I2C_MHZ;
	openwch_assert(freq_mhz >= 1);
	openwch_assert(freq_mhz <= I2C_CTLR2_FREQ_MASK);

	I2C_CTLR2(i2c) = (uint16_t)((I2C_CTLR2(i2c) & ~I2C_CTLR2_FREQ_MASK)
			| ((freq_mhz << I2C_CTLR2_FREQ_SHIFT)
				& I2C_CTLR2_FREQ_MASK));
}

void i2c_init_master(
	uint32_t i2c,
	uint32_t clock,
	uint32_t speed,
	uint32_t duty_cycle
) {
	uint16_t ckcfgr;
	uint32_t divider;

	openwch_assert(clock != 0);
	openwch_assert(clock <= I2C_MAX_CLOCK_FREQUENCY);
	openwch_assert(speed != 0);
	openwch_assert(speed <= I2C_SPEED_FAST);
	openwch_assert((duty_cycle == I2C_CCR_DUTY_2)
			|| (duty_cycle == I2C_CCR_DUTY_16_9));

	i2c_set_clock_frequency(i2c, clock);

	if (speed <= I2C_SPEED_STANDARD) {
		/* Standard mode: CCR = fPCLK1 / (2 * speed), DUTY is unused. */
		divider = I2C_CCR_DIV_STANDARD * speed;
		ckcfgr = I2C_CKCFGR(i2c) & ~(I2C_CKCFGR_CCR_MASK
				| I2C_CKCFGR_DUTY | I2C_CKCFGR_FS);
	} else {
		/* Fast mode: set FS, then the requested Tlow/Thigh ratio. */
		divider = (duty_cycle == I2C_CCR_DUTY_16_9)
				? I2C_CCR_DIV_FAST_DUTY_16_9
				: I2C_CCR_DIV_FAST_DUTY_2;
		divider *= speed;
		ckcfgr = (I2C_CKCFGR(i2c) & ~(I2C_CKCFGR_CCR_MASK
				| I2C_CKCFGR_DUTY | I2C_CKCFGR_FS))
				| I2C_CKCFGR_FS | (uint16_t)duty_cycle;
	}

	/* The requested speed can always be divided down: divider >= 2 * 1e5. */
	openwch_assert(divider >= I2C_CCR_DIV_STANDARD);

	ckcfgr |= (uint16_t)i2c_div_round(clock, divider);
	openwch_assert((ckcfgr & ~(I2C_CKCFGR_CCR_MASK | I2C_CKCFGR_DUTY
					| I2C_CKCFGR_FS)) == 0);
	I2C_CKCFGR(i2c) = ckcfgr;

	i2c_set_rise_time(i2c, clock, speed);
}

void i2c_init_slave(uint32_t i2c, uint32_t clock, uint8_t address) {
	openwch_assert(clock != 0);

	i2c_set_clock_frequency(i2c, clock);
	i2c_set_own_7bit_address(i2c, address);
}

/* --- Enable -------------------------------------------------------------- */

void i2c_enable(uint32_t i2c) {
	I2C_CTLR1(i2c) |= I2C_CTLR1_PE;
}

void i2c_disable(uint32_t i2c) {
	I2C_CTLR1(i2c) &= ~I2C_CTLR1_PE;
}

/* --- Bus control --------------------------------------------------------- */

void i2c_send_start(uint32_t i2c) {
	I2C_CTLR1(i2c) |= I2C_CTLR1_START;
}

void i2c_send_stop(uint32_t i2c) {
	I2C_CTLR1(i2c) |= I2C_CTLR1_STOP;
}

void i2c_send_data(uint32_t i2c, uint8_t data) {
	I2C_DATAR(i2c) = (uint16_t)(data & I2C_DATAR_MASK);
}

uint8_t i2c_read_data(uint32_t i2c) {
	return (uint8_t)(I2C_DATAR(i2c) & I2C_DATAR_MASK);
}

void i2c_send_7bit_address(uint32_t i2c, uint8_t address, uint8_t read) {
	openwch_assert(address <= 0x7fu);
	openwch_assert(read <= 1u);

	/*
	 * The DATAR byte of an address phase is the 7-bit address in bits
	 * [7:1] with the direction in bit 0; the caller passes the address the
	 * way the reference manual writes it, i.e. unshifted in bits [6:0].
	 */
	I2C_DATAR(i2c) = (uint16_t)((address << 1) | (read & 1u));
}

/* --- Own address --------------------------------------------------------- */

void i2c_set_own_7bit_address(uint32_t i2c, uint8_t address) {
	openwch_assert(address <= 0x7fu);

	I2C_OADDR1(i2c) = (uint16_t)((address << 1) & I2C_OADDR1_ADD1_7);
}

void i2c_set_own_10bit_address(uint32_t i2c, uint16_t address) {
	openwch_assert(address <= 0x3ffu);

	I2C_OADDR1(i2c) = (uint16_t)((address << 1 & I2C_OADDR1_ADD8_9)
			| (address << 1 & I2C_OADDR1_ADD1_7)
			| I2C_OADDR1_ADDMODE);
}

void i2c_enable_dual_address(uint32_t i2c, uint8_t address) {
	openwch_assert(address <= 0x7fu);

	I2C_OADDR2(i2c) = (uint16_t)(((address << 1) & I2C_OADDR2_ADD2)
			| I2C_OADDR2_ENDUAL);
}

void i2c_enable_general_call(uint32_t i2c) {
	I2C_CTLR1(i2c) |= I2C_CTLR1_ENGC;
}

/* --- Clock generation ---------------------------------------------------- */

void i2c_set_ccr(uint32_t i2c, uint32_t ccr) {
	openwch_assert((ccr & I2C_CKCFGR_CCR_MASK) == ccr);

	I2C_CKCFGR(i2c) = (uint16_t)((I2C_CKCFGR(i2c) & ~I2C_CKCFGR_CCR_MASK)
			| ccr);
}

void i2c_set_trise(uint32_t i2c, uint32_t trise) {
	openwch_assert((trise & I2C_CKCFGR_CCR_MASK) == trise);

	/*
	 * The CH32V00x has no separate TRISE register: both TRISE and CCR live
	 * in the low twelve bits of CKCFGR and are swapped in and out while the
	 * block is disabled.  This entry point therefore writes the same field
	 * as i2c_set_ccr(), and the two are set in turn during configuration,
	 * as WCH's own initialisation sequence does.
	 */
	I2C_CKCFGR(i2c) = (uint16_t)((I2C_CKCFGR(i2c) & ~I2C_CKCFGR_CCR_MASK)
			| trise);
}

/* --- Acknowledge --------------------------------------------------------- */

void i2c_enable_ack(uint32_t i2c) {
	I2C_CTLR1(i2c) |= I2C_CTLR1_ACK;
}

void i2c_disable_ack(uint32_t i2c) {
	I2C_CTLR1(i2c) &= ~I2C_CTLR1_ACK;
}

void i2c_nack_current(uint32_t i2c) {
	/*
	 * POS = 0: the ACK bit applies to the byte currently being received,
	 * so clearing it NACKs that byte.
	 */
	I2C_CTLR1(i2c) &= ~I2C_CTLR1_POS;
	I2C_CTLR1(i2c) &= ~I2C_CTLR1_ACK;
}

void i2c_nack_next(uint32_t i2c) {
	/* POS = 1: ACK applies to the byte after the next one. */
	I2C_CTLR1(i2c) |= I2C_CTLR1_POS;
	I2C_CTLR1(i2c) &= ~I2C_CTLR1_ACK;
}

/* --- PEC ----------------------------------------------------------------- */

void i2c_enable_pec(uint32_t i2c) {
	I2C_CTLR1(i2c) |= I2C_CTLR1_ENPEC;
	/*
	 * PEC must be set for the transfer that computes the CRC; ENPEC alone
	 * only routes the resulting byte.
	 */
	I2C_CTLR1(i2c) |= I2C_CTLR1_PEC;
}

void i2c_disable_pec(uint32_t i2c) {
	I2C_CTLR1(i2c) &= ~(I2C_CTLR1_ENPEC | I2C_CTLR1_PEC);
}

uint8_t i2c_get_pec(uint32_t i2c) {
	return (uint8_t)((I2C_STAR2(i2c) & I2C_STAR2_PEC_MASK)
			>> I2C_STAR2_PEC_SHIFT);
}

/* --- Reset --------------------------------------------------------------- */

void i2c_software_reset(uint32_t i2c) {
	I2C_CTLR1(i2c) |= I2C_CTLR1_SWRST;
	I2C_CTLR1(i2c) &= ~I2C_CTLR1_SWRST;
}

/* --- Interrupts ---------------------------------------------------------- */

void i2c_enable_interrupt(uint32_t i2c, uint32_t interrupt) {
	I2C_CTLR2(i2c) |= (uint16_t)(interrupt & I2C_CTLR2_IT_MASK);
}

void i2c_disable_interrupt(uint32_t i2c, uint32_t interrupt) {
	I2C_CTLR2(i2c) &= (uint16_t)~(interrupt & I2C_CTLR2_IT_MASK);
}

/* --- Flags --------------------------------------------------------------- */

uint16_t i2c_get_flag(uint32_t i2c, uint32_t flag) {
	uint16_t status = 0;

	if (flag & I2C_CTLR2_IT_MASK) {
		openwch_assert_not_reached();
	}

	if (flag & (I2C_STAR1_SB | I2C_STAR1_ADDR | I2C_STAR1_BTF
			| I2C_STAR1_ADD10 | I2C_STAR1_STOPF | I2C_STAR1_RXNE
			| I2C_STAR1_TXE | I2C_STAR1_BERR | I2C_STAR1_ARLO
			| I2C_STAR1_AF | I2C_STAR1_OVR | I2C_STAR1_PECERR)) {
		status = I2C_STAR1(i2c);
	}

	if (flag & (I2C_STAR2_MSL | I2C_STAR2_BUSY | I2C_STAR2_TRA
			| I2C_STAR2_GENCALL | I2C_STAR2_DUALF)) {
		status |= I2C_STAR2(i2c);
	}

	return (uint16_t)(status & flag);
}

void i2c_clear_flag(uint32_t i2c, uint32_t flag) {
	/*
	 * Several STAR1 flags (ADDR, STOPF, BTF) are cleared by reading STAR1
	 * and then STAR2, which must be done in that order.  Do it first so a
	 * single call settles every clearable flag.
	 */
	(void)I2C_STAR1(i2c);
	(void)I2C_STAR2(i2c);

	/*
	 * Of the remaining flags only the error flags are write-one-to-clear;
	 * writing a 1 to any other STAR1 bit is harmless, but masking keeps
	 * the intent explicit.
	 */
	if (flag & I2C_STAR1_ERR_MASK) {
		I2C_STAR1(i2c) = (uint16_t)(flag & I2C_STAR1_ERR_MASK);
	}
}

uint16_t i2c_get_interrupt_status(uint32_t i2c) {
	/*
	 * STAR1 and STAR2 together form the interrupt status; the error flags
	 * live in STAR1 and the state flags in STAR2, so report both.
	 */
	return (uint16_t)(I2C_STAR1(i2c) | I2C_STAR2(i2c));
}

void i2c_clear_interrupt_pending_bit(uint32_t i2c) {
	/*
	 * There is no explicit interrupt-pending register: a pending event is
	 * retired by reading STAR1 and then STAR2, and the error flags are
	 * additionally cleared by writing ones back to STAR1.
	 */
	uint16_t star1 = I2C_STAR1(i2c);

	(void)I2C_STAR2(i2c);
	I2C_STAR1(i2c) = (uint16_t)(star1 & I2C_STAR1_ERR_MASK);
}
/**@}*/
