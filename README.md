# libopenwch

An open-source peripheral driver library for **WCH (Nanjing Qinheng) RISC-V
microcontrollers**, written in the style of
[libopencm3](https://github.com/libopencm3/libopencm3): a small, direct
register-level API with **lowercase_snake_case** names, a family/subfamily
directory layout, and a `Makefile`-based build that generates the right linker
script from the part number.

```
    libopencm3                    libopenwch
    ----------                    ----------
    arm-none-eabi-gcc      ->     riscv64-unknown-elf-gcc
    cm3/  (Cortex-M core)  ->     qingke/  (WCH QingKe RISC-V core)
    stm32/f1, efm32/tg     ->     ch32v0, ch5xx58x
    nvic_enable_irq()      ->     nvic_enable_irq()   (PFIC-backed)
    ld/devices.data        ->     ld/devices.data     (+ RISC-V keys)
```

## Status

Early incubation.  See `status.md` for the current progress, `phase.md` for the
staged plan, and `project.md` for the full design.

| Family | Parts | Core | State |
|---|---|---|---|
| `ch32v0` | CH32V003, CH32V002, CH32V004, CH32V005, CH32V006, CH32V007 | QingKe V2, RV32EC | **done** — 316 public functions across 15 peripherals |
| `ch5xx58x` | CH582, CH583, CH584, CH585 | QingKe V4, RV32IMAC | **done** — 202 public functions across 12 peripherals |

## Toolchain

A bare-metal RISC-V toolchain is required.  On Debian/Kali/Ubuntu:

```sh
sudo apt-get install -y gcc-riscv64-unknown-elf
```

`mk/gcc-config.mk` probes for `riscv64-unknown-elf`, `riscv64-none-elf`,
`riscv32-unknown-elf`, `riscv-none-elf`, `riscv64-elf` and `riscv32-elf`, in
that order.  Override explicitly with:

```sh
make PREFIX=/opt/xpack-riscv-none-elf-gcc/bin/riscv-none-elf-
```

`riscv64-linux-gnu-` is deliberately **not** probed: its startup files and libc
conventions are different enough to break bare-metal firmware builds.

## Building the library

```sh
git clone <this repo>
cd libopenwch

make                     # builds every family in TARGETS
make TARGETS=ch32v0      # one family
make list-targets        # show what can be built
make genlinktests        # validate ld/devices.data (no toolchain needed)
make stylecheck          # Linux kernel style check
make clean
```

Artifacts land in `lib/`:

```
lib/libopenwch_ch32v0.a
lib/libopenwch_ch5xx58x.a
```

## Using the library

```make
OPENWCH_DIR ?= /path/to/libopenwch
DEVICE      ?= ch32v003f4p6

CFLAGS   += -Os -g -Wall -std=c99
OBJS     += main.o

include $(OPENWCH_DIR)/mk/genlink-config.mk
include $(OPENWCH_DIR)/mk/gcc-config.mk

all: main.elf main.bin main.hex

include $(OPENWCH_DIR)/mk/genlink-rules.mk
include $(OPENWCH_DIR)/mk/gcc-rules.mk
```

`genlink-config.mk` reads `ld/devices.data` and, for the selected `DEVICE`,
sets `-march`/`-mabi`, adds `-D<FAMILY>`, picks
`lib/libopenwch_<family>.a`, and arranges for `generated.$(DEVICE).ld` to be
produced from `ld/linker.ld.S`.

A minimal application:

```c
#include <libopenwch/ch32v0/gpio.h>
#include <libopenwch/qingke/nvic.h>

void exti7_0_isr(void)
{
	gpio_toggle(GPIOA, GPIO1);
	exti_clear_flag(EXTI0);
}

int main(void)
{
	rcc_periph_clock_enable(RCC_GPIOA);
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_10MHZ,
	              GPIO_CNF_OUTPUT_PUSHPULL, GPIO1);
	for (;;) {
		gpio_toggle(GPIOA, GPIO1);
	}
}
```

## Starting a project

`template/` is an application skeleton, the counterpart of
[libopencm3-template](https://github.com/bonedaddy/libopencm3-template):

```sh
cp -r libopenwch/template ~/src/my-firmware
cd ~/src/my-firmware/examples/blink
make OPENWCH_DIR=~/src/libopenwch
make OPENWCH_DIR=~/src/libopenwch flash    # needs minichlink
```

Four working examples ship with it — `blink` and `uart_echo` for the CH32V003,
`ch582_blink` and `ch582_uart_echo` for the CH58x.

Each example is a self-contained directory with its own `Makefile`; `DEVICE`
selects the part and everything else (ISA, linker script, library) is derived
from it.  See `template/README.md` for the full variable reference.

## API conventions

* Functions: `periph_verb_object()` — `gpio_set_mode()`, `usart_set_baudrate()`,
  `rcc_periph_clock_enable()`.
* Constants: `PERIPH_REGISTER_BIT` — `USART_CTLR1_UE`, `RCC_CFGR0_PLLON`.
* Types: lowercase with `_t`.
* The first argument of a peripheral function is its base address, so the same
  function drives every instance (`usart_set_baudrate(USART1, 115200)`).
* Registers are reached through `MMIO32(addr)` and named as close to the WCH
  reference manual as possible.

WCH's own EVT library uses CamelCase (`GPIO_Init`, `GPIOA_ModeCfg`); libopenwch
deliberately does not.  See `project.md` §5.1 for a full migration table.

## Layout

```
Makefile            TARGETS-based recursive build
mk/                 reusable build modules (gcc-config, genlink-config, ...)
scripts/            genlink.py, irq2nvic_h, genlinktest.sh, checkpatch.pl
ld/                 devices.data + linker.ld.S + tests
include/libopenwch/
    qingke/             core layer (≈ libopencm3's cm3/)
    ch32v0/             CH32V00x family headers and irq.json
    ch5xx58x/           CH58x family headers and irq.json
    dispatch/           device -> family header dispatch
lib/
    Makefile.include    single-archive rules
    qingke/             core layer implementation
    ch32v0/             CH32V00x family build
    ch5xx58x/           CH58x family build
template/           application skeleton (rules/, examples/)
doc/  tests/  examples/
```

## License

Library code (`include/`, `lib/`, `ld/`, `mk/`) is
**LGPL-3.0-or-later** — see `LICENSE` (or the identical `COPYING.LGPL3`).  This
is the same split libopencm3 uses, so applications that merely link against the
library are unaffected.

`scripts/checkpatch.pl` is imported from the Linux kernel and is **GPL-2.0**
(see `COPYING.GPL2`).  It is a development tool and is not part of the library.

`COPYING.GPL3` is shipped because the GNU LGPL version 3 is defined as the GNU
GPL version 3 plus additional permissions.  See `NOTICE` for full provenance.

## References

* [libopencm3](https://github.com/libopencm3/libopencm3) — API style, build
  system, and the structure this project follows.
* [ch32fun](https://github.com/cnlohr/ch32fun) — the toolchain-compatibility
  strategy for `riscv64-unknown-elf`.
* WCH's official EVT packages for the CH32V003 and CH58x — used as the
  register-level reference (not copied).
