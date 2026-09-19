/** @defgroup qingke_memorymap_defines Memory map

@brief <b>Core peripheral memory map for WCH QingKe RISC-V cores</b>

@ingroup qingke_defines

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

#ifndef LIBOPENWCH_QINGKE_MEMORYMAP_H
#define LIBOPENWCH_QINGKE_MEMORYMAP_H

/*
 * WCH QingKe cores use the standard RISC-V private peripheral region at
 * 0xE0000000, but they do NOT implement the RISC-V CLINT/PLIC.  Instead:
 *
 *   0xE000E000  PFIC  - Programmable Fast Interrupt Controller
 *   0xE000F000  SysTick - WCH's own system timer (not the RISC-V mtime)
 *
 * The PFIC register block is laid out to mirror the ARM Cortex-M NVIC, which
 * is why libopenwch can keep the familiar nvic_*() API names.
 */

#define CORE_PERIPH_BASE	(0xE0000000U)

#define PFIC_BASE		(CORE_PERIPH_BASE + 0xE000)
#define SYS_TICK_BASE		(CORE_PERIPH_BASE + 0xF000)

/* WCH vendor-specific CSRs (machine mode). */
#define CSR_INTSYSCR		0x804	/* interrupt nesting + hardware stack */
#define CSR_QMISC		0xbc0	/* pipeline / dynamic-prediction control */
#define CSR_QSTATUS		0xfe0

#endif
/**@}*/
