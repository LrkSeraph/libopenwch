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
 * Reset entry point and the reserved part of the interrupt vector table.
 *
 * mtvec is programmed with `<address of _vector_base> | 3`, which on a QingKe
 * core means "vectored, absolute address": the core fetches a full 32-bit
 * handler address from entry `mcause`.  No jump instructions are involved, so
 * every entry is a plain word -- which also makes the whole table
 * runtime-patchable.
 *
 * The table is split across two object files and two input section names so
 * that the linker script can place the two halves unambiguously:
 *
 *   .init         _start: `j _reset_entry`  (the ELF entry point)
 *   .vector_exc   slots 0..15   (this file)
 *   .vector       slots 16..    (generated vector_handlers.c)
 *
 * The linker script concatenates .vector_exc then .vector, defines
 * _vector_base at the start of the former, and asserts that vector_table
 * lands exactly OPENWCH_VECTOR_EXC_OFFSET bytes later.
 *
 * NOTE: the first word of the image must be executable code (a jump), not the
 * vector table, because the WCH boot ROM enters the image at offset 0.
 */

#include <libopenwch/qingke/memorymap.h>

	.section .init, "ax", @progbits
	.globl _start
	.type _start, @function
	.align 2
_start:
	j _reset_entry
	.size _start, . - _start

	.section .vector_exc, "a", @progbits
	.globl _vector_base
	.type _vector_base, @object
	.align 2
_vector_base:
	.option push
	.option norvc
	.word _reset_entry		/* 0  instruction address misaligned */
	.word 0				/* 1  instruction access fault */
	.word 0				/* 2  illegal instruction / NMI */
	.word 0				/* 3  breakpoint / HardFault */
	.word 0xF3F9BDA9		/* 4  WCH magic */
	.word 0				/* 5  ecall from M mode */
	.word 0				/* 6 */
	.word 0				/* 7 */
	.word 0				/* 8  ecall from U mode */
	.word 0				/* 9  breakpoint */
	.word 0				/* 10 */
	.word 0				/* 11 */
	.word 0				/* 12 SysTick */
	.word 0				/* 13 */
	.word 0				/* 14 software interrupt */
	.word 0				/* 15 */
	.option pop
	.size _vector_base, . - _vector_base
