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
 * @ingroup CH5XX58X
 *
 * @brief <b>Inter-Integrated Circuit for the CH58x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * One instance of the STM32-style I2C block at 0x40004800.  The driver
 * exposes the classic libopencm3 entry points: configure, then drive the bus
 * by hand with start/stop/address/data, waiting on the STAR1/STAR2 flags.
 *
 * The clock divider and the maximum rise time are derived from
 * clk_get_sys_clock(), so a caller only names the bus speed it wants.  Note
 * that FREQ is six bits: the system clock must be between 2 and 63 MHz.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch5xx58x/i2c.h>
#include <libopenwch/ch5xx58x/clk.h>
#include <libopenwch/qingke/assert.h>

/** The CH58x has a single I2C block. */
static void i2c_assert_valid(uint32_t i2c)
{
	openwch_assert(i2c == I2C1_BASE);
}

/** TRISE value WCH's own initialisation sequence uses. */
static uint16_t i2c_rise_time(uint32_t sysclock, bool fast)
{
	uint32_t mhz = sysclock / 1000000u;
	uint32_t trise;

	if (fast) {
		/* 300 ns of rise time, expressed in units of the 1 MHz
		 * counter, plus one. */
		trise = ((mhz * 300u) / 1000u) + 1u;
	} else {
		/* 1000 ns at standard speed. */
		trise = mhz + 1u;
	}

	if (trise > RB_I2C_TRISE) {
		trise = RB_I2C_TRISE;
	}

	return (uint16_t)trise;
}

/* --- Clock generation ---------------------------------------------------- */

void i2c_set_clock_frequency(uint32_t i2c)
{
	uint32_t sysclock;

	i2c_assert_valid(i2c);

	sysclock = clk_get_sys_clock();

	openwch_assert(sysclock >= I2C_MIN_CLOCK_FREQUENCY);
	openwch_assert(sysclock <= I2C_MAX_CLOCK_FREQUENCY);

	/* FREQ is the peripheral clock in whole MHz; the caller does not get
	 * to name it because the block is fed straight from the system
	 * clock. */
	I2C_CTRL2(i2c) = (uint16_t)((I2C_CTRL2(i2c) & (uint16_t)~RB_I2C_FREQ)
			| ((sysclock / 1000000u) & RB_I2C_FREQ));
}

/* --- Initialisation ------------------------------------------------------ */

void i2c_init_master(uint32_t i2c, uint32_t speed)
{
	uint32_t sysclock;
	uint16_t ckcfgr;

	i2c_assert_valid(i2c);
	openwch_assert(speed != 0);
	openwch_assert(speed <= I2C_SPEED_FAST);

	sysclock = clk_get_sys_clock();

	/* Reset the block, then tell it the clock before anything else. */
	i2c_software_reset(i2c);
	i2c_set_clock_frequency(i2c);
	i2c_disable(i2c);

	if (speed <= I2C_SPEED_STANDARD) {
		uint32_t ccr = sysclock / (speed * 2u);

		/* The reference manual sets a floor of four on CCR. */
		if (ccr < 4u) {
			ccr = 4u;
		}

		ckcfgr = (uint16_t)(ccr & RB_I2C_CCR);
	} else {
		/* Fast mode with Tlow/Thigh = 2: SCL high and low together
		 * span three CCR counts. */
		uint32_t ccr = sysclock / (speed * 3u);

		if (ccr == 0u) {
			ccr = 1u;
		}

		ckcfgr = (uint16_t)((ccr & RB_I2C_CCR) | RB_I2C_F_S);
	}

	I2C_RTR(i2c) = i2c_rise_time(sysclock,
			speed > I2C_SPEED_STANDARD);
	I2C_CKCFGR(i2c) = ckcfgr;

	i2c_enable(i2c);

	/* Bit 14 is reserved and must always read back as one. */
	I2C_OADDR1(i2c) = RB_I2C_MUST1;
	i2c_enable_ack(i2c);
}

void i2c_init_slave(uint32_t i2c, uint8_t address)
{
	i2c_assert_valid(i2c);

	i2c_software_reset(i2c);
	i2c_set_clock_frequency(i2c);
	i2c_disable(i2c);
	i2c_set_own_7bit_address(i2c, address);
	i2c_enable(i2c);
	i2c_enable_ack(i2c);
}

/* --- Enable -------------------------------------------------------------- */

void i2c_enable(uint32_t i2c)
{
	i2c_assert_valid(i2c);

	I2C_CTRL1(i2c) |= RB_I2C_PE;
}

void i2c_disable(uint32_t i2c)
{
	i2c_assert_valid(i2c);

	I2C_CTRL1(i2c) &= (uint16_t)~RB_I2C_PE;
}

/* --- Bus control --------------------------------------------------------- */

void i2c_send_start(uint32_t i2c)
{
	i2c_assert_valid(i2c);

	I2C_CTRL1(i2c) |= RB_I2C_START;
}

void i2c_send_stop(uint32_t i2c)
{
	i2c_assert_valid(i2c);

	I2C_CTRL1(i2c) |= RB_I2C_STOP;
}

void i2c_send_data(uint32_t i2c, uint8_t data)
{
	i2c_assert_valid(i2c);

	I2C_DATAR(i2c) = (uint16_t)(data & I2C_DATAR_MASK);
}

uint8_t i2c_read_data(uint32_t i2c)
{
	i2c_assert_valid(i2c);

	return (uint8_t)(I2C_DATAR(i2c) & I2C_DATAR_MASK);
}

void i2c_send_7bit_address(uint32_t i2c, uint8_t address, uint8_t read)
{
	i2c_assert_valid(i2c);
	openwch_assert(address <= 0x7fu);
	openwch_assert(read <= 1u);

	/* The address phase is the 7-bit address in bits [7:1] with the
	 * direction in bit 0; the caller passes it unshifted in bits [6:0]. */
	I2C_DATAR(i2c) = (uint16_t)((address << 1) | (read & 1u));
}

/* --- Own address --------------------------------------------------------- */

void i2c_set_own_7bit_address(uint32_t i2c, uint8_t address)
{
	i2c_assert_valid(i2c);
	openwch_assert(address <= 0x7fu);

	I2C_OADDR1(i2c) = (uint16_t)(RB_I2C_MUST1
			| ((address << 1) & RB_I2C_ADD7_1));
}

void i2c_set_own_10bit_address(uint32_t i2c, uint16_t address)
{
	i2c_assert_valid(i2c);
	openwch_assert(address <= 0x3ffu);

	I2C_OADDR1(i2c) = (uint16_t)(RB_I2C_MUST1 | RB_I2C_ADDMODE
			| ((address << 1) & (RB_I2C_ADD9_8 | RB_I2C_ADD7_1)));
}

void i2c_enable_dual_address(uint32_t i2c, uint8_t address)
{
	i2c_assert_valid(i2c);
	openwch_assert(address <= 0x7fu);

	I2C_OADDR2(i2c) = (uint16_t)(((address << 1) & RB_I2C_ADD2)
			| RB_I2C_ENDUAL);
}

void i2c_enable_general_call(uint32_t i2c)
{
	i2c_assert_valid(i2c);

	I2C_CTRL1(i2c) |= RB_I2C_ENGC;
}

/* --- Acknowledge --------------------------------------------------------- */

void i2c_enable_ack(uint32_t i2c)
{
	i2c_assert_valid(i2c);

	I2C_CTRL1(i2c) |= RB_I2C_ACK;
}

void i2c_disable_ack(uint32_t i2c)
{
	i2c_assert_valid(i2c);

	I2C_CTRL1(i2c) &= (uint16_t)~RB_I2C_ACK;
}

/* --- PEC ----------------------------------------------------------------- */

void i2c_enable_pec(uint32_t i2c)
{
	i2c_assert_valid(i2c);

	/* ENPEC computes the CRC; PEC asks for the byte to be transferred. */
	I2C_CTRL1(i2c) |= RB_I2C_ENPEC | RB_I2C_PEC;
}

void i2c_disable_pec(uint32_t i2c)
{
	i2c_assert_valid(i2c);

	I2C_CTRL1(i2c) &= (uint16_t)~(RB_I2C_ENPEC | RB_I2C_PEC);
}

uint8_t i2c_get_pec(uint32_t i2c)
{
	i2c_assert_valid(i2c);

	return I2C_PEC(i2c);
}

/* --- Reset --------------------------------------------------------------- */

void i2c_software_reset(uint32_t i2c)
{
	i2c_assert_valid(i2c);

	/* SWRST is self-clearing once the block has been re-initialised, but
	 * WCH's sequence sets it and then clears it explicitly. */
	I2C_CTRL1(i2c) |= RB_I2C_SWRST;
	I2C_CTRL1(i2c) &= (uint16_t)~RB_I2C_SWRST;
}

/* --- Interrupts ---------------------------------------------------------- */

void i2c_enable_interrupt(uint32_t i2c, uint32_t interrupt)
{
	i2c_assert_valid(i2c);

	I2C_CTRL2(i2c) |= (uint16_t)(interrupt & I2C_CTRL2_IT_MASK);
}

void i2c_disable_interrupt(uint32_t i2c, uint32_t interrupt)
{
	i2c_assert_valid(i2c);

	I2C_CTRL2(i2c) &= (uint16_t)~(interrupt & I2C_CTRL2_IT_MASK);
}

/* --- Flags and interrupt status ------------------------------------------ */

uint16_t i2c_get_flag(uint32_t i2c, uint32_t flag)
{
	uint16_t status = 0;

	i2c_assert_valid(i2c);
	openwch_assert((flag & ~(uint32_t)I2C_FLAG_MASK) == 0);

	/*
	 * STAR1 and STAR2 are separate registers whose bit positions overlap,
	 * so read whichever the mask can select and report the intersection.
	 */
	if (flag & I2C_STAR1_FLAG_MASK) {
		status |= I2C_STAR1(i2c);
	}

	if (flag & I2C_STAR2_FLAG_MASK) {
		status |= I2C_STAR2(i2c);
	}

	return (uint16_t)(status & (uint16_t)flag);
}

void i2c_clear_flag(uint32_t i2c, uint32_t flag)
{
	i2c_assert_valid(i2c);
	openwch_assert((flag & ~(uint32_t)I2C_FLAG_MASK) == 0);

	/*
	 * SB, ADDR, BTF and STOPF are retired by reading STAR1 and then
	 * STAR2; do that first so one call settles every kind of flag.
	 */
	(void)I2C_STAR1(i2c);
	(void)I2C_STAR2(i2c);

	/*
	 * The error flags are write-zero-to-clear, so writing a one to every
	 * bit other than the requested ones leaves the rest untouched.
	 */
	if (flag & I2C_STAR1_CLEAR_MASK) {
		I2C_STAR1(i2c) = (uint16_t)~((uint16_t)flag
				& I2C_STAR1_CLEAR_MASK);
	}
}

uint16_t i2c_get_interrupt_status(uint32_t i2c)
{
	i2c_assert_valid(i2c);

	/* The error flags live in STAR1 and the state flags in STAR2. */
	return (uint16_t)(I2C_STAR1(i2c) | I2C_STAR2(i2c));
}

void i2c_clear_interrupt_pending_bit(uint32_t i2c)
{
	uint16_t star1;

	i2c_assert_valid(i2c);

	/*
	 * There is no interrupt-pending register: reading STAR1 then STAR2
	 * retires the sequenced flags, and writing zeros to the error flags
	 * that are set clears those.
	 */
	star1 = I2C_STAR1(i2c);
	(void)I2C_STAR2(i2c);

	if (star1 & I2C_STAR1_CLEAR_MASK) {
		I2C_STAR1(i2c) = (uint16_t)~(star1 & I2C_STAR1_CLEAR_MASK);
	}
}
/**@}*/
