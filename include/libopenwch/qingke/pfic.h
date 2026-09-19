/** @defgroup qingke_pfic_defines PFIC

@brief <b>Programmable Fast Interrupt Controller of WCH QingKe cores</b>

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

#ifndef LIBOPENWCH_QINGKE_PFIC_H
#define LIBOPENWCH_QINGKE_PFIC_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/qingke/memorymap.h>

/*
 * The WCH PFIC is deliberately register-compatible with the ARM Cortex-M
 * NVIC: the set-enable, clear-enable, set-pending, clear-pending, active and
 * priority arrays live at the same relative offsets.  This is what lets
 * libopenwch expose the familiar nvic_*() API (see qingke/nvic.h).
 */

typedef struct {
	volatile uint32_t isr[8];    /**< 0x000 interrupt state (read only) */
	volatile uint32_t ipr[8];    /**< 0x020 interrupt pending (read only) */
	volatile uint32_t ithresdr;  /**< 0x040 interrupt threshold */
	volatile uint32_t reserved0; /**< 0x044 */
	volatile uint32_t cfgr;	     /**< 0x048 configuration */
	volatile uint32_t gisr;	     /**< 0x04c global interrupt state */
	volatile uint8_t vtcfgr[4];  /**< 0x050 vector table free entries */
	uint8_t reserved1[12];	     /**< 0x054 */
	volatile uint32_t vtaddr[4]; /**< 0x060 fast interrupt addresses */
	uint8_t reserved2[0x90];     /**< 0x070 */
	volatile uint32_t ienr[8];   /**< 0x100 set enable (write only) */
	uint8_t reserved3[0x60];     /**< 0x120 */
	volatile uint32_t irer[8];   /**< 0x180 clear enable (write only) */
	uint8_t reserved4[0x60];     /**< 0x1a0 */
	volatile uint32_t ipsr[8];   /**< 0x200 set pending (write only) */
	uint8_t reserved5[0x60];     /**< 0x220 */
	volatile uint32_t iprr[8];   /**< 0x280 clear pending (write only) */
	uint8_t reserved6[0x60];     /**< 0x2a0 */
	volatile uint32_t iactr[8];  /**< 0x300 active state (read only) */
	uint8_t reserved7[0xe0];     /**< 0x320 */
	volatile uint8_t iprior[256]; /**< 0x400 interrupt priority */
	uint8_t reserved8[0x810];     /**< 0x500 */
	volatile uint32_t sctlr;      /**< 0xd10 system control */
} pfic_reg_t;

#define PFIC ((pfic_reg_t *)PFIC_BASE)

/* cfgr magic keys for privileged operations. */
#define PFIC_KEY1 (0xfa050000u)
#define PFIC_KEY2 (0xbcaf0000u)
#define PFIC_KEY3 (0xbeef0000u)

/* cfgr.SYS_RESET */
#define PFIC_CFGR_SYS_RESET (PFIC_KEY3 | (1 << 7))

/* sctlr bits */
#define PFIC_SCTLR_SLEEPONEXIT (1 << 1)
#define PFIC_SCTLR_SLEEPDEEP (1 << 2)
#define PFIC_SCTLR_WFE (1 << 3)
#define PFIC_SCTLR_EVENT (1 << 5)

#endif
/**@}*/
