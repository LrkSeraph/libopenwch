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

/** @addtogroup gpio_file General Purpose I/O
 *
 * @ingroup CH32V0
 *
 * @brief <b>General Purpose I/O for the CH32V00x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * The CH32V00x GPIO block packs MODE and CNF into one nibble per pin inside
 * CFGLR.  Pull-up and pull-down are not separate configuration bits: the pin
 * is configured as an input with CNF = pull-up/pull-down, and the *output*
 * data register then selects which of the two is active.  That is why
 * gpio_set_mode() writes OUTDR for GPIO_CNF_INPUT_PULL_UPDOWN.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch32v0/gpio.h>
#include <libopenwch/qingke/assert.h>

/*
 * Apply a WCH GPIO_Mode_* token to the pin-packing register `reg` for every
 * pin set in `gpios`.
 *
 * A pin occupies four CFGLR bits: two MODE bits and two CNF bits.  The WCH
 * token carries the CNF/MODE nibble in its low four bits and a fifth bit that
 * says "this is an output or alternate-function mode"; when that bit is set,
 * WCH's GPIO_Init() ORs in the selected speed.  This library's API has no
 * separate speed argument, so output modes use the 10 MHz selector (value 1),
 * which is what the documented WCH examples use.
 */
static void
gpio_nibble_apply(volatile uint32_t *reg, uint8_t mode, uint16_t gpios) {
	uint32_t nibble = (uint32_t)(mode & 0x0fu);
	uint32_t value = *reg;
	unsigned pin;

	if ((mode & 0x10u) != 0u) {
		nibble |= 1u; /* GPIO_Speed_10MHz */
	}

	for (pin = 0; pin < GPIO_PIN_COUNT; pin++) {
		if (gpios & (1 << pin)) {
			uint32_t shift = pin * 4;

			value &= ~(0xfu << shift);
			value |= (nibble & 0xfu) << shift;
		}
	}

	*reg = value;
}

void gpio_set_mode(uint32_t gpioport, uint8_t mode, uint16_t gpios) {
	/* The known WCH tokens are all at or below GPIO_MODE_IPU (0x48). */
	openwch_assert(mode <= GPIO_MODE_IPU);
	openwch_assert((gpios & ~GPIO_ALL) == 0);

	/*
	 * A port is 8 bits wide, so all eight nibbles live in CFGLR.  Requesting
	 * a pin outside 0..7 is a programming error, not a silent no-op.
	 */
	openwch_assert((gpios & 0xff00u) == 0);

	gpio_nibble_apply(&GPIO_CFGLR(gpioport), mode, gpios);

	/*
	 * Pull direction is selected through the output data register, so a
	 * pull-down nibble needs the pin driven low and a pull-up nibble needs
	 * it driven high.
	 */
	if (mode == GPIO_MODE_IPD) {
		GPIO_BCR(gpioport) = gpios;
	} else if (mode == GPIO_MODE_IPU) {
		GPIO_BSHR(gpioport) = gpios;
	}
}

void gpio_set(uint32_t gpioport, uint16_t gpios) {
	GPIO_BSHR(gpioport) = gpios;
}

void gpio_clear(uint32_t gpioport, uint16_t gpios) {
	GPIO_BCR(gpioport) = gpios;
}

uint16_t gpio_get(uint32_t gpioport, uint16_t gpios) {
	return (uint16_t)(GPIO_INDR(gpioport) & gpios);
}

void gpio_toggle(uint32_t gpioport, uint16_t gpios) {
	/* OUTDR is read/write, and BSHR/BCR are write-only, so a
	 * read-modify-write of OUTDR is the portable way to toggle. */
	GPIO_OUTDR(gpioport) ^= gpios;
}

uint16_t gpio_port_read(uint32_t gpioport) {
	return (uint16_t)GPIO_INDR(gpioport);
}

void gpio_port_write(uint32_t gpioport, uint16_t data) {
	GPIO_OUTDR(gpioport) = data;
}

void gpio_port_config_lock(uint32_t gpioport, uint16_t gpios) {
	uint32_t reg;

	openwch_assert((gpios & ~GPIO_ALL) == 0);

	/*
	 * The LCKR sequence is the standard STM32 one, which WCH copied:
	 * write with LCKK set, then the same value with LCKK clear, then with
	 * LCKK set again, and finally read LCKR twice so the read-back drains
	 * the write buffer.
	 */
	reg = (uint32_t)gpios;

	GPIO_LCKR(gpioport) = reg | (1u << 16);
	GPIO_LCKR(gpioport) = reg;
	GPIO_LCKR(gpioport) = reg | (1u << 16);
	(void)GPIO_LCKR(gpioport);
	(void)GPIO_LCKR(gpioport);
}

/* --- Remap helpers ------------------------------------------------------- */

/*
 * WCH's EVT encodes each remap option as a packed 32-bit token (value in
 * [15:0], bit position in [20:16], and class flags up at bits 21/28/31) and
 * unpacks it at run time, which is hard to read and easy to get wrong.
 *
 * libopenwch instead exposes the register fields as they appear in the SVD:
 * a single-bit flag for the simple cases, and a 2-bit field for the four
 * peripherals that have more than one remap option.  The helpers below
 * program the whole field, so switching remap never leaves a stale bit behind.
 */

void gpio_primary_remap(uint32_t remap) {
	AFIO_PCFR1 |= remap;
}

void gpio_secondary_remap(uint32_t remap) {
	AFIO_PCFR1 &= ~remap;
}

/** Program a 2-bit remap field inside AFIO_PCFR1. */
static void gpio_remap_field(uint32_t mask, uint32_t shift, uint32_t value) {
	uint32_t reg = AFIO_PCFR1;

	reg &= ~mask;
	reg |= (value << shift) & mask;
	AFIO_PCFR1 = reg;
}

void gpio_usart1_remap(uint32_t remap) {
	uint32_t reg = AFIO_PCFR1;

	/* USART1_RM is bit 2, USART1REMAP1 is bit 21; together they form
	 * the 2-bit code (REMAP1 << 1) | RM. */
	reg &= ~(AFIO_PCFR1_USART1_RM | AFIO_PCFR1_USART1_REMAP1);
	if (remap & GPIO_REMAP_USART1_PARTIAL1) {
		reg |= AFIO_PCFR1_USART1_RM;
	}
	if (remap & GPIO_REMAP_USART1_PARTIAL2) {
		reg |= AFIO_PCFR1_USART1_REMAP1;
	}
	AFIO_PCFR1 = reg;
}

void gpio_i2c1_remap(uint32_t remap) {
	uint32_t reg = AFIO_PCFR1;

	reg &= ~(AFIO_PCFR1_I2C1_RM | AFIO_PCFR1_I2C1_REMAP1);
	if (remap & GPIO_REMAP_I2C1_PARTIAL) {
		reg |= AFIO_PCFR1_I2C1_RM;
	}
	if (remap & GPIO_REMAP_I2C1_FULL) {
		reg |= AFIO_PCFR1_I2C1_REMAP1;
	}
	AFIO_PCFR1 = reg;
}

void gpio_tim1_remap(uint32_t remap) {
	openwch_assert(remap <= 3);
	gpio_remap_field(AFIO_PCFR1_TIM1_RM_MASK, AFIO_PCFR1_TIM1_RM_SHIFT,
			 remap);
}

void gpio_tim2_remap(uint32_t remap) {
	openwch_assert(remap <= 3);
	gpio_remap_field(AFIO_PCFR1_TIM2_RM_MASK, AFIO_PCFR1_TIM2_RM_SHIFT,
			 remap);
}

/* --- EXTI line source selection ------------------------------------------ */

/*
 * AFIO_EXTICR maps each EXTI line 0..7 to a port.  Keeping it here (rather
 * than in the EXTI driver) matches where libopencm3 puts it: the register is
 * part of the GPIO alternate-function block.
 */
void gpio_exti_select_source(uint32_t exti_line, uint32_t gpioport) {
	unsigned pin;
	uint32_t source;

	for (pin = 0; pin < GPIO_PIN_COUNT; pin++) {
		if (!(exti_line & (1 << pin))) {
			continue;
		}

		if (gpioport == GPIOA_BASE) {
			source = AFIO_EXTICR_PORTA;
		} else if (gpioport == GPIOC_BASE) {
			source = AFIO_EXTICR_PORTC;
		} else if (gpioport == GPIOD_BASE) {
			source = AFIO_EXTICR_PORTD;
		} else {
			openwch_assert(0);
			return;
		}

		AFIO_EXTICR &= ~(0x3u << (pin * 2));
		AFIO_EXTICR |= source << (pin * 2);
	}
}
/**@}*/
