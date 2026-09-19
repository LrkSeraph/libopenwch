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

/** @defgroup qingke_vector_file Vector table
 *
 * @ingroup qingke_file
 *
 * @brief <b>C part of the reset path for WCH QingKe RISC-V cores</b>
 *
 * @version 1.0.0
 *
 * @date 1 January 2025
 *
 * The vector table itself lives in lib/qingke/vector_chipset.S, because its
 * entries must be statically initialised in the same translation unit that
 * defines `_vector_base`.  This file provides the C half of the reset path,
 * which is reached from `_reset_entry` with gp and sp already established.
 *
 * LGPL License Terms @ref lgpl_license
 */
/**@{*/

#include <libopenwch/qingke/vector.h>
#include <libopenwch/qingke/csr.h>
#include <libopenwch/qingke/memorymap.h>
#include <libopenwch/qingke/nvic.h>
#include <libopenwch/qingke/assert.h>

/* Exported by the linker script.  See ld/linker.ld.S. */
extern uint32_t _data_lma[];
extern uint32_t _data_vma[];
extern uint32_t _edata[];
extern uint32_t _sbss[];
extern uint32_t _ebss[];

int main(void);

typedef void (*init_fn_t)(void);
extern init_fn_t __preinit_array_start[], __preinit_array_end[];
extern init_fn_t __init_array_start[], __init_array_end[];
extern init_fn_t __fini_array_start[], __fini_array_end[];

/*
 * Sanity check: the WCH vector table has room for at most 64 entries.
 */
_Static_assert(OPENWCH_VECTOR_ENTRY_COUNT <= 64,
	"vector table larger than the WCH 64-entry maximum");

/*
 * Default for the chip-specific core configuration.  Families whose QingKe
 * core has the pipeline control register (CH32V103/20x/30x, CH58x, CH59x)
 * override this in lib/<family>/common/vector_chipset.c.
 */
__attribute__((weak))
void openwch_chipset_core_configure(void) {
}

/*
 * Default for the chip-specific early initialisation.  Families that must run
 * code from RAM before .data is available (flash-controller reconfiguration)
 * override this.
 */
__attribute__((weak))
void openwch_chipset_early_init(void) {
}

/* Run the C++ static constructors, if any.  Mirrors libopencm3's reset path,
 * which always calls __libc_init_array(); here it is done directly so the
 * library does not depend on newlib. */
static void openwch_run_constructors(void) {
	init_fn_t *fn;

	for (fn = __preinit_array_start; fn < __preinit_array_end; fn++) {
		(*fn)();
	}
	for (fn = __init_array_start; fn < __init_array_end; fn++) {
		(*fn)();
	}
}

/*
 * Reset entry point, reached from _start with gp and sp already established
 * by the assembly prologue in vector_chipset.S.  Defined here in C so that the
 * order of the initialisation steps is readable; the tiny assembly shim exists
 * only because gp must be loaded before any C code runs.
 */
void _reset_entry(void) {
	openwch_reset_init();

	/*
	 * Enter main().  The WCH EVT startup does this through
	 * `csrw mepc, main; mret`; a plain call is simpler and behaves the same
	 * because no trap frame is active yet.  main() is not expected to
	 * return, but trap it if it does.
	 */
	main();

	for (;;) {
		;
	}
}

void openwch_reset_init(void) {
	uint32_t *src, *dst;

	/* Copy .data from its load address in flash to its run address in RAM. */
	for (src = _data_lma, dst = _data_vma; dst < _edata; src++, dst++) {
		*dst = *src;
	}

	/* Zero .bss. */
	for (dst = _sbss; dst < _ebss; dst++) {
		*dst = 0;
	}

	/*
	 * Chip-specific setup that has to happen before anything touches the
	 * peripheral bus (e.g. flash wait states, remapping, clock switch).
	 */
	openwch_chipset_early_init();

	/*
	 * Point the trap vector at the vector table.  Mode 3 means "vectored,
	 * absolute address": the core loads a full handler address from
	 * table[mcause].
	 */
	mtvec_write(((uint32_t)_vector_base) | 3);

	/*
	 * General interrupt setup: nesting and the hardware-pushed interrupt
	 * stack frame.  The latter is what makes OPENWCH_INTERRUPT able to
	 * skip saving the caller-saved registers, and it needs `_eusrstack`
	 * (published by the linker script) as the interrupt stack top.
	 */
	intsyscr_write(INTSYSCR_INEST | INTSYSCR_HWSTK);

	openwch_chipset_core_configure();

	openwch_run_constructors();
}

void openwch_vector_set(uint8_t irq, vector_table_entry_t handler) {
	openwch_assert(irq >= OPENWCH_VECTOR_EXC_COUNT);
	openwch_assert(irq < OPENWCH_VECTOR_EXC_COUNT + OPENWCH_IRQ_COUNT);
	vector_table.handler[irq - OPENWCH_VECTOR_EXC_COUNT] = handler;
}

vector_table_entry_t openwch_vector_get(uint8_t irq) {
	openwch_assert(irq >= OPENWCH_VECTOR_EXC_COUNT);
	openwch_assert(irq < OPENWCH_VECTOR_EXC_COUNT + OPENWCH_IRQ_COUNT);
	return vector_table.handler[irq - OPENWCH_VECTOR_EXC_COUNT];
}

const char *openwch_irq_name(uint8_t irq) {
	uint32_t i;

	for (i = 0; i < OPENWCH_IRQ_TABLE_SIZE; i++) {
		if (openwch_irq_table[i].id == irq) {
			const char *name = openwch_irq_names[openwch_irq_table[i].name];
			if (name[0] != '\0') {
				return name;
			}
			break;
		}
	}
	return "unknown";
}
/**@}*/
