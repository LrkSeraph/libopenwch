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

/** @addtogroup gpio_file GPIO
 *
 * @ingroup CH5XX58X
 *
 * @brief <b>General Purpose I/O for the CH58x</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * Bit-parallel banks, so a whole port is configured with three
 * read-modify-writes rather than a loop over pins.  DIR, PU and PD_DRV are
 * separate registers, which means configuring one pin never disturbs another:
 * the write mask is exactly the requested pins.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/ch5xx58x/gpio.h>
#include <libopenwch/qingke/assert.h>

/** Pins that actually exist on a port, for argument validation. */
static uint32_t gpio_port_pin_mask(uint32_t gpioport) {
	if (gpioport == GPIOB_BASE) {
		return (1u << GPIOB_PIN_COUNT) - 1u;
	}
	if (gpioport == GPIOA_BASE) {
		return (1u << GPIOA_PIN_COUNT) - 1u;
	}

	openwch_assert_not_reached();
	return 0;
}

void gpio_set_mode(uint32_t gpioport, uint32_t mode, uint32_t gpios) {
	/*
	 * R32_Px_PD_DRV is overloaded: as an input it enables the pull-down,
	 * as an output it selects 20 mA instead of 5 mA.  Rather than juggling
	 * that bit per mode, clear both it and PU for the pins, then apply
	 * only what the requested mode needs.
	 */
	uint32_t mask = gpios;

	openwch_assert((gpios & ~gpio_port_pin_mask(gpioport)) == 0);

	GPIO_PD_DRV(gpioport) &= ~mask;
	GPIO_PU(gpioport) &= ~mask;

	switch (mode) {
	case GPIO_MODE_INPUT_FLOAT:
		GPIO_DIR(gpioport) &= ~mask;
		break;
	case GPIO_MODE_INPUT_PU:
		GPIO_DIR(gpioport) &= ~mask;
		GPIO_PU(gpioport) |= mask;
		break;
	case GPIO_MODE_INPUT_PD:
		GPIO_DIR(gpioport) &= ~mask;
		GPIO_PD_DRV(gpioport) |= mask;
		break;
	case GPIO_MODE_OUTPUT_PP_5MA:
		GPIO_DIR(gpioport) |= mask;
		break;
	case GPIO_MODE_OUTPUT_PP_20MA:
		GPIO_DIR(gpioport) |= mask;
		GPIO_PD_DRV(gpioport) |= mask;
		break;
	default:
		openwch_assert_not_reached();
		break;
	}
}

void gpio_set(uint32_t gpioport, uint32_t gpios) {
	GPIO_OUT(gpioport) |= gpios;
}

void gpio_clear(uint32_t gpioport, uint32_t gpios) {
	/* R32_Px_CLR is write-1-to-clear, so no read-modify-write is needed and
	 * clearing cannot disturb another pin. */
	GPIO_CLR(gpioport) = gpios;
}

void gpio_toggle(uint32_t gpioport, uint32_t gpios) {
	GPIO_OUT(gpioport) ^= gpios;
}

uint32_t gpio_get(uint32_t gpioport, uint32_t gpios) {
	return GPIO_PIN(gpioport) & gpios;
}

uint32_t gpio_port_read(uint32_t gpioport) {
	return GPIO_PIN(gpioport);
}

void gpio_port_write(uint32_t gpioport, uint32_t data) {
	GPIO_OUT(gpioport) = data;
}

/* --- Interrupts ---------------------------------------------------------- */

/** R16_Px_INT_EN for a port. */
static volatile uint16_t *gpio_irq_en_reg(uint32_t gpioport) {
	return (gpioport == GPIOB_BASE) ? &MMIO16(R16_PB_INT_EN)
					: &MMIO16(R16_PA_INT_EN);
}

static volatile uint16_t *gpio_irq_mode_reg(uint32_t gpioport) {
	return (gpioport == GPIOB_BASE) ? &MMIO16(R16_PB_INT_MODE)
					: &MMIO16(R16_PA_INT_MODE);
}

static volatile uint16_t *gpio_irq_flag_reg(uint32_t gpioport) {
	return (gpioport == GPIOB_BASE) ? &MMIO16(R16_PB_INT_IF)
					: &MMIO16(R16_PA_INT_IF);
}

/*
 * PB22 and PB23 have no interrupt bits of their own: their interrupt flags
 * alias PB8 and PB9 unless R16_PIN_ALTERNATE.RB_PIN_INTX moves them onto the
 * dedicated EXTI24/25 lines.  Fold the aliased bits so that a caller can keep
 * using the real pin numbers either way.
 *
 * WCH's EVT does the same shift: (pin & (Pin_22 | Pin_23)) >> 14.
 */
#define GPIO_IRQ_ALIAS_PINS	(GPIO22 | GPIO23)
#define GPIO_IRQ_ALIAS_SHIFT	14

static uint32_t gpio_irq_fold(uint32_t gpioport, uint32_t gpios) {
	if (gpioport != GPIOB_BASE) {
		return gpios;
	}

	/* With INTX remap enabled the alias is gone and the real bits are used. */
	if (GPIO_PIN_ALTERNATE & GPIO_REMAP_INTX) {
		return gpios;
	}

	if (!(gpios & GPIO_IRQ_ALIAS_PINS)) {
		return gpios;
	}

	return (gpios & ~GPIO_IRQ_ALIAS_PINS)
		| ((gpios & GPIO_IRQ_ALIAS_PINS) >> GPIO_IRQ_ALIAS_SHIFT);
}

void gpio_set_irq_mode(uint32_t gpioport, uint32_t gpios, uint32_t mode) {
	uint32_t mask = gpio_irq_fold(gpioport, gpios);

	openwch_assert((gpios & ~gpio_port_pin_mask(gpioport)) == 0);
	openwch_assert(mode <= GPIO_IRQ_RISING_EDGE);

	/* Level-triggered modes clear INT_MODE, edge-triggered set it. */
	if (mode >= GPIO_IRQ_FALLING_EDGE) {
		*gpio_irq_mode_reg(gpioport) |= (uint16_t)mask;
	} else {
		*gpio_irq_mode_reg(gpioport) &= (uint16_t)~mask;
	}

	/* Falling edge / low level drive the pin low; rising / high drive it
	 * high, which is how the trigger polarity is selected. */
	if (mode == GPIO_IRQ_FALLING_EDGE || mode == GPIO_IRQ_LOW_LEVEL) {
		GPIO_CLR(gpioport) = gpios;
	} else {
		GPIO_OUT(gpioport) |= gpios;
	}

	*gpio_irq_flag_reg(gpioport) = (uint16_t)mask;
	*gpio_irq_en_reg(gpioport) |= (uint16_t)mask;
}

uint32_t gpio_get_irq_flag(uint32_t gpioport) {
	return *gpio_irq_flag_reg(gpioport);
}

void gpio_clear_irq_flag(uint32_t gpioport, uint32_t gpios) {
	/* R16_Px_INT_IF is RW1: write the mask to clear. */
	*gpio_irq_flag_reg(gpioport) = (uint16_t)gpio_irq_fold(gpioport, gpios);
}

/* --- Alternate function and analog input --------------------------------- */

void gpio_pin_remap(uint32_t remap) {
	GPIO_PIN_ALTERNATE |= (uint16_t)remap;
}

void gpio_analog_enable(uint32_t analog) {
	GPIO_PIN_ANALOG_IE |= (uint16_t)analog;
}

void gpio_analog_disable(uint32_t analog) {
	GPIO_PIN_ANALOG_IE &= (uint16_t)~analog;
}
/**@}*/
