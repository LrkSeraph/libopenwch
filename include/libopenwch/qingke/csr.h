/** @defgroup qingke_csr_defines CSR access

@brief <b>Control and status register access for WCH QingKe RISC-V cores</b>

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

#ifndef LIBOPENWCH_QINGKE_CSR_H
#define LIBOPENWCH_QINGKE_CSR_H

#include <libopenwch/qingke/common.h>

/*
 * Only the machine-mode subset is provided: every WCH QingKe part runs
 * firmware in machine mode.
 */

/* mstatus */
#define CSR_MSTATUS_MIE		(1 << 3)	/* machine interrupt enable */
#define CSR_MSTATUS_MPIE	(1 << 7)	/* previous MIE */
#define CSR_MSTATUS_MPP		(3 << 11)	/* previous privilege */
#define CSR_MSTATUS_FS		(3 << 13)	/* floating point state */

/* INTSYSCR (0x804) */
#define INTSYSCR_INEST		(1 << 0)	/* interrupt nesting enable */
#define INTSYSCR_HWSTK		(1 << 1)	/* hardware stack push enable */

/* mcause */
#define CSR_MCAUSE_INT		(1u << 31)	/* 1 = interrupt, 0 = exception */
#define CSR_MCAUSE_CODE_MASK	0x7fffffffu

/* The QingKe exception/interrupt codes that have a fixed vector slot. */
#define QINGKE_EXC_INSN_MISALIGN	0
#define QINGKE_EXC_INSN_ACCESS		1
#define QINGKE_EXC_ILLEGAL_INSN		2
#define QINGKE_EXC_BREAKPOINT		3
#define QINGKE_EXC_LOAD_MISALIGN	4
#define QINGKE_EXC_LOAD_ACCESS		5
#define QINGKE_EXC_STORE_MISALIGN	6
#define QINGKE_EXC_STORE_ACCESS		7
#define QINGKE_EXC_ECALL_U		8
#define QINGKE_EXC_ECALL_M		11

/* The interrupt slots that are not peripheral interrupts. */
#define QINGKE_IRQ_SYSTICK	12
#define QINGKE_IRQ_SOFTWARE	14

#ifndef __ASSEMBLER__

BEGIN_DECLS

/* Raw CSR accessors.  The csr number must be a compile-time constant. */
#define OPENWCH_CSR_READ(csr) \
	({ uint32_t __v; __asm__ volatile ("csrr %0, " #csr : "=r" (__v)); __v; })

#define OPENWCH_CSR_WRITE(csr, val) \
	__asm__ volatile ("csrw " #csr ", %0" : : "rK" ((uint32_t)(val)) : "memory")

#define OPENWCH_CSR_SET(csr, bits) \
	__asm__ volatile ("csrs " #csr ", %0" : : "rK" ((uint32_t)(bits)) : "memory")

#define OPENWCH_CSR_CLEAR(csr, bits) \
	__asm__ volatile ("csrc " #csr ", %0" : : "rK" ((uint32_t)(bits)) : "memory")

#define OPENWCH_CSR_READ_NUM(num) \
	({ uint32_t __v; __asm__ volatile ("csrr %0, %1" : "=r" (__v) : "i" (num)); __v; })

#define OPENWCH_CSR_WRITE_NUM(num, val) \
	__asm__ volatile ("csrw %0, %1" : : "i" (num), "rK" ((uint32_t)(val)) : "memory")

#define OPENWCH_CSR_SET_NUM(num, bits) \
	__asm__ volatile ("csrs %0, %1" : : "i" (num), "rK" ((uint32_t)(bits)) : "memory")

#define OPENWCH_CSR_CLEAR_NUM(num, bits) \
	__asm__ volatile ("csrc %0, %1" : : "i" (num), "rK" ((uint32_t)(bits)) : "memory")

/* Named accessors.  Some of these CSRs exist only on newer QingKe cores, so
 * they are expressed with their numeric encoding. */
#define mstatus_read()		OPENWCH_CSR_READ_NUM(0x300)
#define mstatus_write(v)	OPENWCH_CSR_WRITE_NUM(0x300, (v))
#define mtvec_read()		OPENWCH_CSR_READ_NUM(0x305)
#define mtvec_write(v)		OPENWCH_CSR_WRITE_NUM(0x305, (v))
#define mepc_read()		OPENWCH_CSR_READ_NUM(0x341)
#define mepc_write(v)		OPENWCH_CSR_WRITE_NUM(0x341, (v))
#define mcause_read()		OPENWCH_CSR_READ_NUM(0x342)
#define mtval_read()		OPENWCH_CSR_READ_NUM(0x343)
#define intsyscr_read()		OPENWCH_CSR_READ_NUM(0x804)
#define intsyscr_write(v)	OPENWCH_CSR_WRITE_NUM(0x804, (v))

/* Memory ordering helper used around peripheral register sequences. */
static inline void qingke_fence_i(void)
{
	__asm__ volatile ("fence.i" ::: "memory");
}

static inline void qingke_nop(void)
{
	__asm__ volatile ("nop");
}

/*
 * The QingKe pipeline-configuration register (0xbc0) only exists on QingKe
 * V4-class cores (CH32V103/20x/30x, CH58x, CH59x).  On V2 cores
 * (CH32V003/002/004/005/006/007) writing it traps, so a family that needs it
 * must override the weak openwch_chipset_core_configure() hook from
 * lib/<family>/common/vector_chipset.c rather than calling it unconditionally.
 */

/* Software reset through the PFIC. */
void qingke_system_reset(void);

END_DECLS

#endif /* __ASSEMBLER__ */

#endif
/**@}*/
