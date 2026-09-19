/*
 * This file is part of the libopenwch template.
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
 * CH58x core-layer bring-up.
 *
 * A full blink needs the CH58x clock and GPIO drivers, which are the P3
 * milestone: on this family almost every system, clock, power and GPIO
 * register is write-protected (RWA) and must be written inside the
 * 0x57/0xA8 safe-access window.  Rather than poke those registers without the
 * proper sequence, this example exercises what is already implemented and
 * verified:
 *
 *   - the reset path, .data/.bss initialisation and the vector table,
 *   - the PFIC-backed interrupt controller,
 *   - the SysTick timer as a time base,
 *   - .highcode, which this family's linker script emits.
 *
 * Once lib/ch5xx58x/clk.c and gpio.c land, replace the body of main() with the
 * equivalent of the CH32V003 blink.
 *
 * Build:   make
 * Size:    make size
 */

#include <libopenwch/qingke/nvic.h>
#include <libopenwch/qingke/sync.h>
#include <libopenwch/qingke/systick.h>
#include <libopenwch/qingke/vector.h>

/* A counter the SysTick handler bumps; proves interrupts are wired up. */
static volatile uint32_t ticks;

/* Defined below; the weak fallback lives in the generated vector handlers. */
void systick_isr(void);

/*
 * The timer interrupt.  The handler is weak-aliased to blocking_handler() by
 * the generated vector_handlers.c, so this non-weak definition wins at link
 * time.
 */
void systick_isr(void)
{
	systick_clear_interrupt();
	ticks++;
}

int main(void)
{
	/*
	 * Counter clock and reload: 1 ms at 48 MHz with the counter running
	 * from the system clock.  SysTick is a core exception, so it is enabled
	 * through its own control register rather than through nvic_enable_irq()
	 * (which drives the PFIC external-interrupt array).
	 */
	qingke_systick_set_frequency(48000000u);
	systick_set_clock_source(1);
	systick_set_reload(48000u);
	systick_clear_interrupt();
	systick_enable_interrupt();
	systick_enable_counter();

	qingke_irq_enable();

	for (;;) {
		/* Wait for the handler to advance the counter. */
		uint32_t before = ticks;

		while (ticks == before) {
			;
		}
	}

	return 0;
}
