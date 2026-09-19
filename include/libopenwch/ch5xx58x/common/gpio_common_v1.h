/** @addtogroup gpio_defines GPIO Defines

@brief <b>Defined Constants and Types for the CH58x General Purpose I/O</b>

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA GPIO.H */

/** @cond */
#if defined(LIBOPENWCH_GPIO_H) || defined(LIBOPENWCH_GPIO_COMMON_V1_H)
/** @endcond */
#ifndef LIBOPENWCH_GPIO_COMMON_V1_H
#define LIBOPENWCH_GPIO_COMMON_V1_H

#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * The CH58x GPIO is completely different from the CH32V00x one.
 *
 * Instead of a CNF/MODE nibble per pin, each port is a set of BIT-PARALLEL
 * 32-bit banks: one bit per pin, no fields, no configuration register.
 *
 * Two consequences worth knowing:
 *
 *   - the same R32_Px_PD_DRV bit means "input pull-down enable" when
 *     R32_Px_DIR says input, and "output drive strength" (0 = 5 mA,
 *     1 = 20 mA) when it says output.  One bit, two meanings, selected by the
 *     direction bit.
 *   - GPIOB is 24 bits wide (PB0..PB23) while GPIOA is 16.  The 32-bit
 *     registers have reserved upper bits on GPIOB.
 *
 * These registers are plain read/write, NOT RWA: no safe-access window is
 * needed to configure a pin.
 */

/* --- Register accessors -------------------------------------------------- */

#define GPIO_DIR(port) MMIO32((port) + GPIO_DIR_OFFSET)
#define GPIO_PIN(port) MMIO32((port) + GPIO_PIN_OFFSET)
#define GPIO_OUT(port) MMIO32((port) + GPIO_OUT_OFFSET)
#define GPIO_CLR(port) MMIO32((port) + GPIO_CLR_OFFSET)
#define GPIO_PU(port) MMIO32((port) + GPIO_PU_OFFSET)
#define GPIO_PD_DRV(port) MMIO32((port) + GPIO_PD_DRV_OFFSET)

/* --- Pin identifiers ----------------------------------------------------- */

/** @defgroup gpio_pin_id GPIO Pin Identifiers
@ingroup gpio_defines
@{*/
#define GPIO0 (1u << 0)
#define GPIO1 (1u << 1)
#define GPIO2 (1u << 2)
#define GPIO3 (1u << 3)
#define GPIO4 (1u << 4)
#define GPIO5 (1u << 5)
#define GPIO6 (1u << 6)
#define GPIO7 (1u << 7)
#define GPIO8 (1u << 8)
#define GPIO9 (1u << 9)
#define GPIO10 (1u << 10)
#define GPIO11 (1u << 11)
#define GPIO12 (1u << 12)
#define GPIO13 (1u << 13)
#define GPIO14 (1u << 14)
#define GPIO15 (1u << 15)

/* GPIOB goes up to PB23. */
#define GPIO16 (1u << 16)
#define GPIO17 (1u << 17)
#define GPIO18 (1u << 18)
#define GPIO19 (1u << 19)
#define GPIO20 (1u << 20)
#define GPIO21 (1u << 21)
#define GPIO22 (1u << 22)
#define GPIO23 (1u << 23)

#define GPIO_ALL 0xffffffffu
/**@}*/

/** Number of pins on GPIOA. */
#define GPIOA_PIN_COUNT 16
/** Number of pins on GPIOB. */
#define GPIOB_PIN_COUNT 24

/* --- Pin modes ----------------------------------------------------------- */

/** @defgroup gpio_mode GPIO Pin Modes
@ingroup gpio_defines

Unlike the CH32V00x, the mode here also selects the output drive strength,
because that is the same register bit as the input pull-down.
@{*/
#define GPIO_MODE_INPUT_FLOAT 0	   /**< floating input */
#define GPIO_MODE_INPUT_PU 1	   /**< input with pull-up */
#define GPIO_MODE_INPUT_PD 2	   /**< input with pull-down */
#define GPIO_MODE_OUTPUT_PP_5MA 3  /**< push-pull output, 5 mA */
#define GPIO_MODE_OUTPUT_PP_20MA 4 /**< push-pull output, 20 mA */
/**@}*/

/** @defgroup gpio_irq_mode GPIO Interrupt Modes
@ingroup gpio_defines
@{*/
#define GPIO_IRQ_LOW_LEVEL 0
#define GPIO_IRQ_HIGH_LEVEL 1
#define GPIO_IRQ_FALLING_EDGE 2
#define GPIO_IRQ_RISING_EDGE 3
/**@}*/

/* --- Pin alternate function remap ---------------------------------------- */

/** @defgroup gpio_remap_id GPIO Remap Identifiers
@ingroup gpio_defines

The CH58x has no per-pin alternate-function number.  A function is enabled by
(a) setting its remap bit in R16_PIN_ALTERNATE, (b) enabling the peripheral's
own output driver, and (c) configuring the pin with gpio_set_mode().  These are
the R16_PIN_ALTERNATE bits.
@{*/
#define GPIO_REMAP_TMR0 (1u << 0)
#define GPIO_REMAP_TMR1 (1u << 1)
#define GPIO_REMAP_TMR2 (1u << 2)
#define GPIO_REMAP_TMR3 (1u << 3)
#define GPIO_REMAP_UART0 (1u << 4)
#define GPIO_REMAP_UART1 (1u << 5)
#define GPIO_REMAP_UART2 (1u << 6)
#define GPIO_REMAP_UART3 (1u << 7)
#define GPIO_REMAP_SPI0 (1u << 8)
#define GPIO_REMAP_PWMX (1u << 10)
#define GPIO_REMAP_I2C (1u << 11)
#define GPIO_REMAP_MODEM (1u << 12)
#define GPIO_REMAP_INTX (1u << 13) /**< PB22/23 as EXTI24/25 */
#define GPIO_REMAP_UART0_INV (1u << 14)
#define GPIO_REMAP_RF_ANT_SW (1u << 15)
/**@}*/

/* --- Analog input enable ------------------------------------------------- */

/** @defgroup gpio_analog_id GPIO Analog Input Identifiers
@ingroup gpio_defines

R16_PIN_ANALOG_IE disables the digital input buffer on pins used as analog
inputs (ADC / touch-key channels), which is required for an accurate reading.
@{*/
#define GPIO_ANALOG_ADC0 (1u << 0)
#define GPIO_ANALOG_ADC1 (1u << 1)
#define GPIO_ANALOG_ADC2 (1u << 2)
#define GPIO_ANALOG_ADC3 (1u << 3)
#define GPIO_ANALOG_ADC4 (1u << 4)
#define GPIO_ANALOG_ADC5 (1u << 5)
#define GPIO_ANALOG_ADC6 (1u << 6)
#define GPIO_ANALOG_ADC7 (1u << 7)
#define GPIO_ANALOG_ADC8 (1u << 8)
#define GPIO_ANALOG_ADC9 (1u << 9)
#define GPIO_ANALOG_ADC10 (1u << 10)
#define GPIO_ANALOG_ADC11 (1u << 11)
#define GPIO_ANALOG_ADC12 (1u << 12)
#define GPIO_ANALOG_ADC13 (1u << 13)
#define GPIO_ANALOG_USB (1u << 14)
#define GPIO_ANALOG_USB_DP_PU (1u << 15)
#define GPIO_ANALOG_XT32K (1u << 16)
/**@}*/

BEGIN_DECLS

void gpio_set_mode(uint32_t gpioport, uint32_t mode, uint32_t gpios);
void gpio_set(uint32_t gpioport, uint32_t gpios);
void gpio_clear(uint32_t gpioport, uint32_t gpios);
void gpio_toggle(uint32_t gpioport, uint32_t gpios);
uint32_t gpio_get(uint32_t gpioport, uint32_t gpios);
uint32_t gpio_port_read(uint32_t gpioport);
void gpio_port_write(uint32_t gpioport, uint32_t data);

/* Interrupt configuration for a whole port's pin mask. */
void gpio_set_irq_mode(uint32_t gpioport, uint32_t gpios, uint32_t mode);
uint32_t gpio_get_irq_flag(uint32_t gpioport);
void gpio_clear_irq_flag(uint32_t gpioport, uint32_t gpios);

/* Alternate function and analog input selection. */
void gpio_pin_remap(uint32_t remap);
void gpio_analog_enable(uint32_t analog);
void gpio_analog_disable(uint32_t analog);

END_DECLS

#endif
/** @cond */
#else
#warning "gpio_common_v1.h should not be included explicitly, only via gpio.h"
#endif
/** @endcond */
/**@}*/
