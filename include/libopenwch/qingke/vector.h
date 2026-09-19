/** @defgroup qingke_vector_defines Vector table

@brief <b>Interrupt vector table for WCH QingKe RISC-V cores</b>

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

#ifndef LIBOPENWCH_QINGKE_VECTOR_H
#define LIBOPENWCH_QINGKE_VECTOR_H

#include <libopenwch/qingke/common.h>
#include <libopenwch/qingke/nvic.h>

/*
 * How the WCH vector table works
 * ------------------------------
 *
 * mtvec is programmed as `<table address> | 3`, which on a QingKe core selects
 * "vectored, absolute address" mode: the core fetches a full 32-bit handler
 * address from table[cause], where `cause` is the exception/interrupt number.
 * Every entry is a plain word; the table is 4-byte aligned and all entries are
 * emitted uncompressed (`.option norvc`).
 *
 * Unlike the ARM Cortex-M table there is no leading stack-pointer word: a
 * QingKe core keeps using the current sp, and the hardware-push ("HPS/HPE")
 * feature maintains a separate interrupt stack whose top is the linker symbol
 * `_eusrstack`.
 *
 * The table is built from two link-time pieces:
 *
 *   lib/qingke/vector_chipset.S   slots 0..15   fixed RISC-V exception slots
 *   vector_handlers_table (below) slots 16..    external interrupts
 *
 * `_vector_base` is defined by the linker script at the start of `.vector`,
 * and the linker asserts that the C table lands exactly
 * OPENWCH_VECTOR_EXC_OFFSET bytes into it.
 *
 * Slot assignments for 0..15:
 *
 *   0            instruction address misaligned
 *   1            instruction access fault
 *   2            illegal instruction / NMI
 *   3            breakpoint / HardFault
 *   4  0xF3F9BDA9 (magic the WCH boot ROM and debug tooling expect)
 *   5            ecall from M mode
 *   6,7          reserved
 *   8            ecall from U mode
 *   9            breakpoint
 *   10,11        reserved
 *   12           SysTick
 *   13           reserved
 *   14           software interrupt
 *   15           reserved
 *   16..         external (peripheral) interrupts
 *
 * The public model is "the table is just data": a handler can be installed at
 * runtime with `OPENWCH_VECTOR_SET(irq, fn)`, or at link time by defining a
 * non-weak `<periph>_isr()` function.
 */

/** Index of the first external interrupt (and of the first PFIC interrupt). */
#define OPENWCH_VECTOR_EXC_COUNT 16

/** Offset of the first external interrupt slot, in bytes. */
#define OPENWCH_VECTOR_EXC_OFFSET (OPENWCH_VECTOR_EXC_COUNT * 4)

/** Total number of entries in the whole vector table. */
#define OPENWCH_VECTOR_ENTRY_COUNT                                             \
	(OPENWCH_VECTOR_EXC_COUNT + OPENWCH_IRQ_COUNT)

/* Indices (relative to the whole table) of the conventional exception slots. */
#define OPENWCH_VEC_INSN_MISALIGNED 0
#define OPENWCH_VEC_INSN_ACCESS 1
#define OPENWCH_VEC_ILLEGAL_INSN 2 /**< also the NMI slot */
#define OPENWCH_VEC_HARD_FAULT 3
#define OPENWCH_VEC_ECALL_M 5
#define OPENWCH_VEC_ECALL_U 8
#define OPENWCH_VEC_BREAKPOINT 9
#define OPENWCH_VEC_SYSTICK 12
#define OPENWCH_VEC_SOFTWARE 14

/* The WCH magic word expected in slot 4. */
#define OPENWCH_VECTOR_MAGIC 0xf3f9bda9u

typedef void (*vector_table_entry_t)(void);

/*
 * The external-interrupt half of the vector table: a dense array of function
 * pointers indexed by (interrupt id - OPENWCH_VECTOR_EXC_COUNT).  A device
 * therefore only pays for the slots it actually has, while element 0 still
 * corresponds to vector slot 16.
 */
typedef struct {
	/** External interrupt handlers. */
	vector_table_entry_t handler[OPENWCH_IRQ_COUNT];
} vector_table_t;

BEGIN_DECLS

/** The external-interrupt vector table, placed in the `.vector` section. */
extern vector_table_t vector_table;

/** Description of one external interrupt.  Used for diagnostics. */
typedef struct {
	uint16_t id;   /**< vector slot / PFIC interrupt id */
	uint16_t name; /**< index into the generated name table */
} openwch_irq_desc_t;

/** Generated descriptor array; `[0]` is vector slot 16. */
extern const openwch_irq_desc_t openwch_irq_table[];
extern const char *const openwch_irq_names[];

/** Base of the whole vector table; provided by the linker script. */
extern vector_table_entry_t _vector_base[];

/** Entry point; defined in qingke/vector_chipset.S. */
__attribute__((noreturn)) void _reset_entry(void);

/** C part of the reset path, called from _reset_entry() with gp and sp set. */
void openwch_reset_init(void);

/** Chip-specific early initialisation; the default implementation is weak. */
void openwch_chipset_early_init(void);

/** Chip-specific core configuration; the default implementation is weak. */
void openwch_chipset_core_configure(void);

/** Default handler installed for every unhandled interrupt. */
void blocking_handler(void);

/* Install / read a handler at runtime. */
void openwch_vector_set(uint8_t irq, vector_table_entry_t handler);
vector_table_entry_t openwch_vector_get(uint8_t irq);

/* Return a human-readable name for an interrupt id, or "unknown". */
const char *openwch_irq_name(uint8_t irq);

END_DECLS

#define OPENWCH_VECTOR_SET(irq, fn) (openwch_vector_set((irq), (fn)))

#endif
/**@}*/
