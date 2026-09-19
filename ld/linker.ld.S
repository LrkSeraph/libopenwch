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

/*
 * Generic linker script for all WCH RISC-V targets.
 *
 * This file is a preprocessor template.  mk/genlink-rules.mk runs it through
 * $(CPP) with the definitions that scripts/genlink.py extracted from
 * ld/devices.data for the selected DEVICE:
 *
 *   _ROM, _ROM_OFF          flash region (only if defined)
 *   _RAM, _RAM_OFF          main RAM region
 *   _ENTRY                  entry symbol (default _start)
 *   _GP                     define to emit __global_pointer$
 *   _HICODE                 define to emit the .highcode section
 *   plus the optional regions _ROM1/_ROM2, _RAM1.._RAM5, _CCM, _EEP, _XSRAM,
 *   _XDRAM, _NFCRAM (same convention as libopencm3).
 *
 * Segment order matters and mirrors the WCH EVT scripts:
 *
 *   .init      executable entry (`j _reset_entry`) -- must be first
 *   .vector    the interrupt vector table (4-byte aligned, norvc entries)
 *   .highcode  code/data that must run from RAM (flash-controller and
 *              low-power paths); LMA in flash, VMA in RAM
 *   .text ...  the rest of the flash-resident content
 *   .data      LMA in flash, VMA in RAM
 *   .bss       RAM
 */

#ifndef _ENTRY
#define _ENTRY _start
#endif

/*
 * Program headers: keep the executable image (flash) and the initialised-data
 * image (RAM) in separate segments, and the vector table readable but not
 * executable.  Without this the linker emits a single RWX LOAD segment.
 */
PHDRS
{
	text PT_LOAD FLAGS(5);	/* R E */
	rodata PT_LOAD FLAGS(4);	/* R   */
	data PT_LOAD FLAGS(6);	/* RW  */
}

/* Enforce emission of the vector table. */
EXTERN(vector_table)
EXTERN(_vector_base)

/* Define the entry point of the output file. */
ENTRY(_ENTRY)

/* Define memory regions. */
MEMORY
{
	/* RAM is always used. */
	ram (rwx) : ORIGIN = _RAM_OFF, LENGTH = _RAM

#if defined(_ROM)
	rom (rx)  : ORIGIN = _ROM_OFF, LENGTH = _ROM
#endif
#if defined(_ROM1)
	rom1 (rx) : ORIGIN = _ROM1_OFF, LENGTH = _ROM1
#endif
#if defined(_ROM2)
	rom2 (rx) : ORIGIN = _ROM2_OFF, LENGTH = _ROM2
#endif
#if defined(_RAM1)
	ram1 (rwx) : ORIGIN = _RAM1_OFF, LENGTH = _RAM1
#endif
#if defined(_RAM2)
	ram2 (rwx) : ORIGIN = _RAM2_OFF, LENGTH = _RAM2
#endif
#if defined(_RAM3)
	ram3 (rwx) : ORIGIN = _RAM3_OFF, LENGTH = _RAM3
#endif
#if defined(_CCM)
	ccm (rwx) : ORIGIN = _CCM_OFF, LENGTH = _CCM
#endif
#if defined(_EEP)
	eep (r)   : ORIGIN = _EEP_OFF, LENGTH = _EEP
#endif
}

/* Define sections. */
SECTIONS
{
	/*
	 * Entry code.  The first word of the image must be executable, because
	 * the WCH boot ROM and the debug tooling both enter at offset 0.
	 */
	.init : {
		KEEP(*(.init))
		. = ALIGN(4);
	} >rom :text

	/*
	 * The interrupt vector table.  _vector_base is the address programmed
	 * into mtvec (with the low two bits set to 3), and the C vector_table
	 * object must land exactly OPENWCH_VECTOR_EXC_OFFSET bytes into it.
	 * .option norvc in the source keeps every entry a full word.
	 */
	.vector : ALIGN(4) {
		_vector_base = .;
		/*
		 * The reserved RISC-V exception slots (slots 0..15) come from
		 * lib/qingke/vector_chipset.S and MUST be first; the generated
		 * external-interrupt table (slots 16..) follows.  Keeping them
		 * in two named input sections makes the order explicit instead
		 * of depending on archive member ordering.
		 */
		KEEP(*(.vector_exc))
		KEEP(*(.vector))
		. = ALIGN(4);
	} >rom :rodata

	/*
	 * Code and data that must execute from RAM.  Used by the CH5xx and the
	 * CH32V103/20x/30x families, whose flash controller (and low-power
	 * entry) cannot be used while running from flash.
	 */
#if defined(_HICODE)
	.highcodelalign : {
		. = ALIGN(4);
		PROVIDE(_highcode_lma = .);
	} >rom

	.highcode : {
		. = ALIGN(4);
		PROVIDE(_highcode_vma_start = .);
		KEEP(*(.highcode))
		KEEP(*(.highcode.*))
		. = ALIGN(4);
		PROVIDE(_highcode_vma_end = .);
	} >ram AT >rom
#endif

	.text : {
		*(.text)
		*(.text.*)
		*(.gnu.linkonce.t.*)
		. = ALIGN(4);
		*(.rodata)
		*(.rodata.*)
		*(.gnu.linkonce.r.*)
		. = ALIGN(4);
	} >rom :text

	.fini : {
		KEEP(*(SORT_NONE(.fini)))
		. = ALIGN(4);
	} >rom

	. = ALIGN(4);
	PROVIDE(_etext = .);

	/*
	 * C++ / GCC static constructors and destructors.  The reset path
	 * (openwch_reset_init) walks __preinit_array and __init_array.
	 */
	.preinit_array : {
		PROVIDE_HIDDEN(__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN(__preinit_array_end = .);
	} >rom

	.init_array : {
		PROVIDE_HIDDEN(__init_array_start = .);
		KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN(__init_array_end = .);
	} >rom

	.fini_array : {
		PROVIDE_HIDDEN(__fini_array_start = .);
		KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*)))
		KEEP(*(.fini_array))
		PROVIDE_HIDDEN(__fini_array_end = .);
	} >rom

	/* Legacy GCC constructor/destructor sections. */
	.ctors : {
		KEEP(*(SORT(.ctors.*)))
		KEEP(*(.ctors))
		. = ALIGN(4);
	} >rom

	.dtors : {
		KEEP(*(SORT(.dtors.*)))
		KEEP(*(.dtors))
		. = ALIGN(4);
	} >rom

	/* Location of the .data initialiser image in flash. */
	.dlalign : {
		. = ALIGN(4);
		PROVIDE(_data_lma = .);
	} >rom

	/* Run address of .data in RAM. */
	.dalign : {
		. = ALIGN(4);
		PROVIDE(_data_vma = .);
	} >ram

	.data : AT(_data_lma) {
		. = ALIGN(4);
		_data = .;
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d.*)
		*(.ramtext*)
		. = ALIGN(8);
		*(.sdata)
		*(.sdata.*)
		*(.gnu.linkonce.s.*)
		*(.sdata2)
		*(.sdata2.*)
		. = ALIGN(8);
		*(.srodata.cst16)
		*(.srodata.cst8)
		*(.srodata.cst4)
		*(.srodata.cst2)
		*(.srodata)
		*(.srodata.*)
		. = ALIGN(4);
		_edata = .;
		PROVIDE(edata = .);
	} >ram :data

	/*
	 * The global pointer must be inside the ±2 KiB window of the small
	 * data sections.  The reset path loads it before touching any C code.
	 *
	 * NOTE: this is a plain assignment, not PROVIDE().  PROVIDE only takes
	 * effect for otherwise-undefined symbols, so it would be silently
	 * ignored as soon as any object file carries the (default) implicit
	 * reference to __global_pointer$ that GCC emits.
	 */
#if defined(_GP)
	/*
	 * gp must be 4-byte aligned and within +/-2 KiB of every small-data
	 * object.  Clamp into [ORIGIN(ram), ORIGIN(ram)+LENGTH(ram)-4] so
	 * that a tiny RAM part (CH32V003 has only 2 KiB) cannot produce a
	 * value outside the region.
	 */
	__global_pointer$ = MIN(ORIGIN(ram) + LENGTH(ram) - 4,
			MAX(ORIGIN(ram), _data + 0x800));
#endif

	.bss (NOLOAD) : {
		. = ALIGN(4);
		_sbss = .;
		PROVIDE(__bss_start = .);
		*(.sbss)
		*(.sbss.*)
		*(.gnu.linkonce.sb.*)
		*(.bss)
		*(.bss.*)
		*(.gnu.linkonce.b.*)
		*(COMMON)
		. = ALIGN(4);
		_ebss = .;
		PROVIDE(__bss_end = .);
	} >ram

	/* RAM that survives a reset (bootloader hand-off, retained state). */
	.noinit (NOLOAD) : {
		. = ALIGN(4);
		*(.noinit*)
		. = ALIGN(4);
	} >ram

#if defined(_RAM1)
	.ram1 (NOLOAD) : {
		_ram1 = .;
		*(.ram1*)
		. = ALIGN(4);
		_eram1 = .;
	} >ram1
#endif
#if defined(_RAM2)
	.ram2 (NOLOAD) : {
		_ram2 = .;
		*(.ram2*)
		. = ALIGN(4);
		_eram2 = .;
	} >ram2
#endif
#if defined(_RAM3)
	.ram3 (NOLOAD) : {
		_ram3 = .;
		*(.ram3*)
		. = ALIGN(4);
		_eram3 = .;
	} >ram3
#endif
#if defined(_CCM)
	.ccm (NOLOAD) : {
		_ccm = .;
		*(.ccmram*)
		. = ALIGN(4);
		_eccm = .;
	} >ccm
#endif

	. = ALIGN(4);
	PROVIDE(_end = .);
	PROVIDE(end = .);

	/*
	 * The user stack grows down from the top of RAM.  _eusrstack is also
	 * the top of the hardware-pushed interrupt stack (INTSYSCR.HWSTK), so
	 * every consumer of it must agree on the value.
	 */
	PROVIDE(_stack = ORIGIN(ram) + LENGTH(ram));
	PROVIDE(_eusrstack = ORIGIN(ram) + LENGTH(ram));

	/* Nothing in the vector table may be garbage-collected. */
	/DISCARD/ : {
		*(.eh_frame)
		*(.eh_frame.*)
		*(.note)
		*(.note.*)
		*(.comment)
		*(.ARM.*)
	}
}

/*
 * Build-time validity checks.
 *
 * 1. The C vector table must start exactly OPENWCH_VECTOR_EXC_OFFSET bytes
 *    after _vector_base.  A failure means the generated vector_handlers.c
 *    disagrees with lib/qingke/vector_chipset.S about the number of reserved
 *    exception slots.
 * 2. _vector_base must be 4-byte aligned (mtvec mode 3 requires it).
 * 3. __global_pointer$ must be 4-byte aligned.
 */
ASSERT((_vector_base & 3) == 0,
	"libopenwch: _vector_base is not 4-byte aligned")
/*
 * The stack grows down from _eusrstack and shares RAM with .bss and .noinit.
 * Reserve room for the interrupt stack and complain early if the image grows
 * into it, instead of corrupting memory at run time.
 */
ASSERT((_ebss <= _eusrstack - 64),
	"libopenwch: .bss reaches into the stack; reduce RAM usage or grow the part")
ASSERT((vector_table - _vector_base) == 64,
	"libopenwch: vector_table is not at _vector_base + 64")
#if defined(_GP)
ASSERT((__global_pointer$ & 3) == 0,
	"libopenwch: __global_pointer$ is not 4-byte aligned")
#endif
