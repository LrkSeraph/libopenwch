/** @addtogroup gpio_defines GPIO Defines

@brief <b>Defined Constants and Types for the CH32V00x General Purpose I/O</b>

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

/* THIS FILE SHOULD NOT BE INCLUDED DIRECTLY, BUT ONLY VIA GPIO.H
The order of header inclusion is important. gpio.h includes the device
specific memorymap.h header before including this header file.*/

/** @cond */
#if defined(LIBOPENWCH_GPIO_H)
/** @endcond */
#ifndef LIBOPENWCH_GPIO_COMMON_V1_H
#define LIBOPENWCH_GPIO_COMMON_V1_H

/*
 * MMIO32, BEGIN_DECLS/END_DECLS and the stdint types come from here.  The
 * device header normally arrives first, but the common headers are also
 * reachable when the family's dispatch header is included on its own, so pull
 * the core definitions in explicitly.
 */
#include <libopenwch/qingke/common.h>

/**@{*/

/*
 * The CH32V00x GPIO block is the classic STM32-style arrangement: two bits of
 * MODE and two bits of CNF per pin, packed four bits per pin in a single
 * 32-bit configuration register.  The ports are only 8 bits wide, so there is
 * no CFGHR -- pins 0..7 live in CFGLR.
 *
 * A pin's four bits are at bit (pin * 4): the low two are MODE and the high
 * two are CNF.  gpio_set_mode() takes WCH's full GPIO_Mode_* token, not a
 * libopencm3 (mode, cnf) pair: the token's low four bits are the CNF/MODE
 * nibble, and its 0x10 bit marks an output/alternate mode whose speed is
 * fixed at 10 MHz by this API.
 */

/* --- Register accessors -------------------------------------------------- */

/* Port configuration register low (GPIOx_CFGLR) */
#define GPIO_CFGLR(port) MMIO32((port) + 0x00)
#define GPIOA_CFGLR GPIO_CFGLR(GPIOA)
#define GPIOC_CFGLR GPIO_CFGLR(GPIOC)
#define GPIOD_CFGLR GPIO_CFGLR(GPIOD)

/* Port input data register (GPIOx_INDR) */
#define GPIO_INDR(port) MMIO32((port) + 0x08)
#define GPIOA_INDR GPIO_INDR(GPIOA)
#define GPIOC_INDR GPIO_INDR(GPIOC)
#define GPIOD_INDR GPIO_INDR(GPIOD)

/* Port output data register (GPIOx_OUTDR) */
#define GPIO_OUTDR(port) MMIO32((port) + 0x0c)
#define GPIOA_OUTDR GPIO_OUTDR(GPIOA)
#define GPIOC_OUTDR GPIO_OUTDR(GPIOC)
#define GPIOD_OUTDR GPIO_OUTDR(GPIOD)

/* Port bit set/reset register (GPIOx_BSHR) */
#define GPIO_BSHR(port) MMIO32((port) + 0x10)
#define GPIOA_BSHR GPIO_BSHR(GPIOA)
#define GPIOC_BSHR GPIO_BSHR(GPIOC)
#define GPIOD_BSHR GPIO_BSHR(GPIOD)

/* Port bit reset register (GPIOx_BCR) */
#define GPIO_BCR(port) MMIO32((port) + 0x14)
#define GPIOA_BCR GPIO_BCR(GPIOA)
#define GPIOC_BCR GPIO_BCR(GPIOC)
#define GPIOD_BCR GPIO_BCR(GPIOD)

/* Port configuration lock register (GPIOx_LCKR) */
#define GPIO_LCKR(port) MMIO32((port) + 0x18)
#define GPIOA_LCKR GPIO_LCKR(GPIOA)
#define GPIOC_LCKR GPIO_LCKR(GPIOC)
#define GPIOD_LCKR GPIO_LCKR(GPIOD)

/* --- Pin identifiers ----------------------------------------------------- */

/** @defgroup gpio_pin_id GPIO Pin Identifiers
@ingroup gpio_defines

@{*/
#define GPIO0 (1 << 0)
#define GPIO1 (1 << 1)
#define GPIO2 (1 << 2)
#define GPIO3 (1 << 3)
#define GPIO4 (1 << 4)
#define GPIO5 (1 << 5)
#define GPIO6 (1 << 6)
#define GPIO7 (1 << 7)
#define GPIO_ALL 0x00ff
/**@}*/

/* --- Pin configuration nibbles ------------------------------------------ */

/*
 * Each pin owns one nibble of CFGLR.  The nibble is the register encoding
 * that WCH documents as `GPIO_Mode_*` in the CH32V00x reference manual, and
 * it is the value WCH's own EVT writes into CFGLR.
 *
 * It is NOT a simple (mode, configuration) bit packing.  Exhaustively
 * searching every possible pair of two-bit field positions shows that no
 * decomposition reproduces the documented values:
 *
 *	GPIO_MODE_AIN		0x0
 *	GPIO_MODE_IN_FLOATING	0x4
 *	GPIO_MODE_IPD		0x28
 *	GPIO_MODE_IPU		0x48
 *	GPIO_MODE_OUT_OD	0x14
 *	GPIO_MODE_OUT_PP	0x10
 *	GPIO_MODE_AF_OD		0x1c
 *	GPIO_MODE_AF_PP		0x18
 *
 * The tokens are therefore treated as opaque values, exactly as WCH's own
 * GPIO_Mode_* enumeration presents them, rather than being re-derived from
 * field positions.  A token copied out of WCH's EVT or the manual can be
 * passed to gpio_set_mode() unchanged.
 *
 * Pull direction is not part of the nibble: GPIO_MODE_IPD and GPIO_MODE_IPU
 * encode the same "input with pull" configuration, and the output data
 * register then chooses between pull-down (0) and pull-up (1).
 */

/** @defgroup gpio_mode GPIO Pin Configuration Nibbles
@ingroup gpio_defines

@{*/
#define GPIO_MODE_AIN 0x0u	   /**< analog input */
#define GPIO_MODE_IN_FLOATING 0x4u /**< floating input */
#define GPIO_MODE_IPD 0x28u	   /**< input, pull-down */
#define GPIO_MODE_IPU 0x48u	   /**< input, pull-up */
#define GPIO_MODE_OUT_OD 0x14u	   /**< open-drain output, 10 MHz */
#define GPIO_MODE_OUT_PP 0x10u	   /**< push-pull output, 10 MHz */
#define GPIO_MODE_AF_OD 0x1cu	   /**< alternate function open-drain */
#define GPIO_MODE_AF_PP 0x18u	   /**< alternate function push-pull */
/**@}*/

/*
 * Output drive strength is part of the nibble on this family (there is no
 * separate speed field): the 10 MHz nibbles above are the ones WCH's own
 * examples use, and the faster variants are listed here for completeness.
 */
#define GPIO_MODE_IPD_PULL GPIO_MODE_IPD
#define GPIO_MODE_IPU_PULL GPIO_MODE_IPU

/* Convenience aliases so that "output" reads naturally at a call site. */
#define GPIO_MODE_OUTPUT_PP GPIO_MODE_OUT_PP
#define GPIO_MODE_OUTPUT_OD GPIO_MODE_OUT_OD
#define GPIO_MODE_OUTPUT_AF_PP GPIO_MODE_AF_PP
#define GPIO_MODE_OUTPUT_AF_OD GPIO_MODE_AF_OD

/* Mask of one pin's nibble inside CFGLR. */
#define GPIO_CFGLR_NIBBLE_MASK 0xfu

/* --- Number of pins ------------------------------------------------------ */

/** Number of GPIO pins per port.  The CH32V00x ports are 8 bits wide. */
#define GPIO_PIN_COUNT 8

/* --- Alternate function remap -------------------------------------------- */

/** @defgroup gpio_remap_id GPIO Remap Identifiers
@ingroup gpio_defines

Single-bit remap flags for gpio_primary_remap() / gpio_secondary_remap(),
derived from the CH32V003 SVD.  For the peripherals whose remap is a two-bit
code (USART1, I2C1, TIM1, TIM2) prefer the dedicated gpio_<periph>_remap()
helpers, which clear and set the field in one step.

@{*/
#define GPIO_REMAP_SPI1 0x1u
#define GPIO_REMAP_I2C1_PARTIAL 0x2u
#define GPIO_REMAP_I2C1_FULL 0x3u
#define GPIO_REMAP_USART1_PARTIAL1 0x1u
#define GPIO_REMAP_USART1_PARTIAL2 0x2u
#define GPIO_REMAP_USART1_FULL 0x3u
#define GPIO_REMAP_TIM1_NONE 0x0u
#define GPIO_REMAP_TIM1_PARTIAL1 0x1u
#define GPIO_REMAP_TIM1_PARTIAL2 0x2u
#define GPIO_REMAP_TIM1_FULL 0x3u
#define GPIO_REMAP_TIM2_NONE 0x0u
#define GPIO_REMAP_TIM2_PARTIAL1 0x1u
#define GPIO_REMAP_TIM2_PARTIAL2 0x2u
#define GPIO_REMAP_TIM2_FULL 0x3u
#define GPIO_REMAP_PA12_ON_OSC 0x1u
#define GPIO_REMAP_ADC1_ETRGINJ 0x1u
#define GPIO_REMAP_ADC1_ETRGREG 0x1u
#define GPIO_REMAP_SDI_DISABLE 0x1u
#define GPIO_REMAP_LSI_CAL 0x1u
/**@}*/

BEGIN_DECLS

void gpio_set_mode(uint32_t gpioport, uint8_t mode, uint16_t gpios);
void gpio_set(uint32_t gpioport, uint16_t gpios);
void gpio_clear(uint32_t gpioport, uint16_t gpios);
uint16_t gpio_get(uint32_t gpioport, uint16_t gpios);
void gpio_toggle(uint32_t gpioport, uint16_t gpios);
uint16_t gpio_port_read(uint32_t gpioport);
void gpio_port_write(uint32_t gpioport, uint16_t data);
void gpio_port_config_lock(uint32_t gpioport, uint16_t gpios);
void gpio_primary_remap(uint32_t remap);
void gpio_secondary_remap(uint32_t remap);
void gpio_exti_select_source(uint32_t exti_line, uint32_t gpioport);

/*
 * Two-bit remap helpers.  Each takes the GPIO_REMAP_<periph>_* code and
 * programs the whole field, so a previous setting cannot leak through.
 */
void gpio_usart1_remap(uint32_t remap);
void gpio_i2c1_remap(uint32_t remap);
void gpio_tim1_remap(uint32_t remap);
void gpio_tim2_remap(uint32_t remap);

END_DECLS

#endif
/** @cond */
#else
#warning "gpio_common_v1.h should not be included explicitly, only via gpio.h"
#endif
/** @endcond */
/**@}*/
