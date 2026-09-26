# libopenwch

[中文文档](README.zh-CN.md)

Open-source peripheral driver library for **WCH RISC-V microcontrollers**,
modeled on [libopencm3](https://github.com/libopencm3/libopencm3):
lowercase_snake_case API, family directories, and a Makefile that derives the
ISA and linker script from the part number.

> **Incubating — not production-ready, and hardware validation is partial.**
> The public API compiles and links, and the register descriptions follow WCH's
> manual.  Some integration components have been validated on real CH32V003
> hardware (the SPI NOR CRC example and the `wchlink` GDB server with software
> breakpoints); the library as a whole still has not.

| Family | Parts | Core / ISA | State |
|---|---|---|---|
| `ch32v0` | CH32V003/002/004/005/006/007 | QingKe V2, RV32EC | 316 public functions, 15 peripherals |
| `ch5xx58x` | CH582/583/584/585 | QingKe V4, RV32IMAC | 230 public functions, 12 peripherals + BLE |

Open questions that need a real part: the `DBGMCU_CR` bit layout and whether the
BLE wrapper works at all.

## Toolchain

```sh
sudo apt-get install -y gcc-riscv64-unknown-elf
```

`mk/gcc-config.mk` probes `riscv64-unknown-elf`, `riscv64-none-elf`,
`riscv32-unknown-elf`, `riscv-none-elf`, `riscv64-elf`, `riscv32-elf`, and can
be overridden with `make PREFIX=...`. `riscv64-linux-gnu-` is not probed.

The library archives need only libgcc, but GCC 13 turns `openwch_reset_init()`'s
`.data`/`.bss` loops into `memcpy`/`memset`. When the toolchain has no newlib for
the selected multilib, `mk/libc-config.mk` links the bundled per-family
mini-libc automatically; `LIBOPENWCH_NOSTDLIB=1` forces it. The mini-libc
provides `mem*` and `str*` only.

## Building

```sh
make                     # all families in TARGETS
make TARGETS=ch32v0      # one family
make list-targets
make genlinktests        # validate ld/devices.data (no toolchain needed)
make apitest             # compile and link every public function
make stylecheck          # checkpatch.pl second opinion
```

Artifacts: `lib/libopenwch_<family>.a` and
`lib/libopenwch_mini_libc_<family>.a`. Formatting is clang-format's job via the
local pre-commit hook, never CI.

## Using it

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

`genlink-config.mk` selects `-march`/`-mabi`, the family archive, and the
generated linker script. Never add `-march`/`-mabi` by hand; mixing `ilp32`
and `ilp32e` breaks the ABI. `-nostartfiles` is required because libopenwch
supplies `_start` and the vector table.

```c
#include <libopenwch/ch32v0/gpio.h>
#include <libopenwch/ch32v0/rcc.h>

int main(void)
{
rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_PLL_HSI_48MHZ]);

rcc_periph_clock_enable(RCC_GPIOA);
gpio_set_mode(GPIOA, GPIO_MODE_OUT_PP, GPIO1);
for (;;) {
gpio_toggle(GPIOA, GPIO1);
}
}
```

Clock trees are data (`rcc_hsi_configs[]`, `rcc_hse_configs[]`,
`clk_source_t` on CH58x); one call selects one and publishes
`rcc_sysclk_frequency`, `rcc_ahb_frequency`, `rcc_apb1_frequency`,
`rcc_apb2_frequency`, `rcc_adc_frequency`.

For working applications, use
[libopenwch-template](https://github.com/LrkSeraph/libopenwch-template) and
[libopenwch-examples](https://github.com/LrkSeraph/libopenwch-examples).

## Bluetooth LE (CH58x)

WCH's closed-source stack is vendored under `lib/ble/wch/` (Apache-2.0, not
LGPL) and wrapped with a thin `ble_*` layer for TMOS, GAP, the peripheral role,
GATT server, and startup. It is a wrapper, not a stack; central/observer/
broadcaster, bonding, OTA, and mesh are not covered. Needs about 145 KB flash
and an application-declared heap.

## API notes

* Functions `periph_verb_object()`; first argument is the peripheral base.
* Constants `PERIPH_REGISTER_BIT`; types lowercase `_t`; registers use
  `MMIO32()` and WCH manual names.
* CH32V00x `gpio_set_mode(port, mode, pins)` takes WCH's opaque `GPIO_Mode_*`
  token, not libopencm3's `(mode, cnf)` pair.

## Documentation

```sh
make html      # root site + one API site per family
```

Outputs: `doc/html/index.html`, `doc/<family>/html/index.html`; warning logs in
`doc/*.log`. Doxygen is required; see `doc/README.md`.

## Layout

```
Makefile            TARGETS-based recursive build
mk/                 reusable build modules for applications too
scripts/            genlink.py, irq2nvic_h, checkpatch.pl
ld/                 devices.data, linker.ld.S, tests
include/libopenwch/ qingke/, ch32v0/, ch5xx58x/, dispatch/, ble/
lib/                per-family sources, mini-libc, BLE
doc/                Doxygen template, prose pages, vendored theme
tests/              per-family API smoke tests
```

## Licence

`include/`, `lib/`, `ld/`, `mk/` are LGPL-3.0-or-later; `scripts/checkpatch.pl`
is GPL-2.0; `lib/ble/wch/` is Apache-2.0. See `LICENSE` and `NOTICE`.

Built with reference to libopencm3, ch32fun, and WCH EVT packages. Not
affiliated with Nanjing Qinheng Microelectronics.
