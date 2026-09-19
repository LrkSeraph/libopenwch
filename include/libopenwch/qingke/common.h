/** @defgroup qingke_defines QingKe Defines

@brief <b>Defined Constants and Types common to all WCH QingKe RISC-V cores</b>

@ingroup peripheral_apis

@version 1.0.0

@date 1 January 2025

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

/**@{*/

#ifndef LIBOPENWCH_QINGKE_COMMON_H
#define LIBOPENWCH_QINGKE_COMMON_H

/*
 * This header plays the role that libopencm3's cm3/common.h plays for
 * Cortex-M: it provides the architecture-independent memory-mapped I/O
 * accessors, the BITn constants and the C/C++/assembler linkage helpers.
 *
 * The MMIO accessors themselves are architecture-neutral, so they are
 * deliberately identical to libopencm3's.
 */

#if defined(__ASSEMBLER__)
#define MMIO8(addr) (addr)
#define MMIO16(addr) (addr)
#define MMIO32(addr) (addr)
#define MMIO64(addr) (addr)
#else

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Generic memory-mapped I/O accessor functions */
#define MMIO8(addr) (*(volatile uint8_t *)(addr))
#define MMIO16(addr) (*(volatile uint16_t *)(addr))
#define MMIO32(addr) (*(volatile uint32_t *)(addr))
#define MMIO64(addr) (*(volatile uint64_t *)(addr))
#endif

/* Generic bit definition */
#define BIT0 (1 << 0)
#define BIT1 (1 << 1)
#define BIT2 (1 << 2)
#define BIT3 (1 << 3)
#define BIT4 (1 << 4)
#define BIT5 (1 << 5)
#define BIT6 (1 << 6)
#define BIT7 (1 << 7)
#define BIT8 (1 << 8)
#define BIT9 (1 << 9)
#define BIT10 (1 << 10)
#define BIT11 (1 << 11)
#define BIT12 (1 << 12)
#define BIT13 (1 << 13)
#define BIT14 (1 << 14)
#define BIT15 (1 << 15)
#define BIT16 (1 << 16)
#define BIT17 (1 << 17)
#define BIT18 (1 << 18)
#define BIT19 (1 << 19)
#define BIT20 (1 << 20)
#define BIT21 (1 << 21)
#define BIT22 (1 << 22)
#define BIT23 (1 << 23)
#define BIT24 (1 << 24)
#define BIT25 (1 << 25)
#define BIT26 (1 << 26)
#define BIT27 (1 << 27)
#define BIT28 (1 << 28)
#define BIT29 (1 << 29)
#define BIT30 (1 << 30)
#define BIT31 (1 << 31)

/* Linkage helpers.  Semantics identical to libopencm3's cm3/common.h. */
#ifdef __ASSEMBLER__
#define BEGIN_DECLS .if 0
#define END_DECLS .endif
#else
#ifdef __cplusplus
#define BEGIN_DECLS extern "C" {
#define END_DECLS }
#else
#define BEGIN_DECLS
#define END_DECLS
#endif
#endif

/* Full-featured deprecation attribute with fallback for older compilers. */
#ifdef __GNUC__
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 4)
#define LIBOPENWCH_DEPRECATED(x) __attribute__((deprecated(x)))
#else
#define LIBOPENWCH_DEPRECATED(x) __attribute__((deprecated))
#endif
#else
#define LIBOPENWCH_DEPRECATED(x)
#endif

/*
 * Compiler attributes used throughout the library.
 *
 * OPENWCH_INTERRUPT is what makes an ISR work on a QingKe core.  The WCH
 * hardware pushes a partial register frame on interrupt entry ("hardware
 * stack", enabled by INTSYSCR), and the interrupt attribute tells GCC not to
 * re-save it.  A plain C function without this attribute corrupts the
 * interrupted context.
 *
 * GCC only accepts the "WCH-Interrupt-fast" machine string when it was built
 * with WCH/QingKe support (the Debian riscv64-unknown-elf toolchain does not
 * have it).  Fall back to the portable RISC-V interrupt attribute, which saves
 * a full frame -- correct, just larger.
 */
#if defined(__riscv)
#define OPENWCH_IRQ_ATTR_WCH __attribute__((interrupt("WCH-Interrupt-fast")))
#define OPENWCH_IRQ_ATTR_PLAIN __attribute__((interrupt()))
#if defined(OPENWCH_USE_WCH_INTERRUPT)
#define OPENWCH_INTERRUPT OPENWCH_IRQ_ATTR_WCH
#else
#define OPENWCH_INTERRUPT OPENWCH_IRQ_ATTR_PLAIN
#endif
#define OPENWCH_NAKED __attribute__((naked))
#define OPENWCH_WEAK __attribute__((weak))
#define OPENWCH_ALIAS(x) __attribute__((weak, alias(#x)))
#define OPENWCH_USED __attribute__((used))
#else
#error "libopenwch targets RISC-V parts only"
#endif

/* Place code that must execute from RAM (see the .highcode section). */
#define OPENWCH_HIGH_CODE __attribute__((section(".highcode"), used))

/* Force a 2-byte-aligned object (the WCH vector table requirement). */
#define OPENWCH_ALIGN2 __attribute__((aligned(2)))

#endif
/**@}*/
