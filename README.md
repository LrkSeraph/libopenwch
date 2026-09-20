# libopenwch

An open-source peripheral driver library for **WCH (Nanjing Qinheng) RISC-V
microcontrollers**, written in the style of
[libopencm3](https://github.com/libopencm3/libopencm3): a small, direct
register-level API with **lowercase_snake_case** names, a family/subfamily
directory layout, and a `Makefile`-based build that generates the right linker
script from the part number.

> **Incubating — not ready for production.**  Pre-1.0, API not frozen, and
> **nothing has run on silicon**: this project has no development board and no
> WCH-Link.  What *is* verified is that the whole public API compiles and links,
> that the archives carry the right ISA, and that the register descriptions
> match WCH's reference manual.  That is not the same as having run it.

| Family | Parts | Core | State |
|---|---|---|---|
| `ch32v0` | CH32V003/002/004/005/006/007 | QingKe V2, RV32EC | 316 public functions, 15 peripherals |
| `ch5xx58x` | CH582/583/584/585 | QingKe V4, RV32IMAC | 230 public functions, 12 peripherals + Bluetooth LE |

Two open questions need a real part to settle: the `DBGMCU_CR` bit layout
(CSR `0x7c0`), where WCH's EVT and ch32fun disagree, and whether the BLE layer
works at all — it links WCH's closed-source stack and has never executed.

## Toolchain

```sh
sudo apt-get install -y gcc-riscv64-unknown-elf
```

`mk/gcc-config.mk` probes `riscv64-unknown-elf`, `riscv64-none-elf`,
`riscv32-unknown-elf`, `riscv-none-elf`, `riscv64-elf`, `riscv32-elf`, in that
order; override with `make PREFIX=/opt/xpack/bin/riscv-none-elf`.
`riscv64-linux-gnu-` is deliberately not probed: its crt and libc conventions
break bare-metal builds.

**libc.**  The library calls no C library function directly and its archives
reference only libgcc — but `openwch_reset_init()`'s `.data`/`.bss` loops become
`memcpy`/`memset` calls under GCC 13 (GCC 15 inlines them), so a freestanding
link has to supply both.  Debian and Ubuntu ship **no newlib** for the `rv32e`
or `rv32imac` multilibs, so an application has two modes:

| mode | link | use when |
|---|---|---|
| default | `-lc -lgcc -lnosys` (newlib) | the toolchain has newlib |
| `LIBOPENWCH_NOSTDLIB=1` | `-nostdlib` + `libopenwch_mini_libc_<family>.a` + `-lgcc` | it does not |

The mini-libc is a per-family archive of `memcpy`, `memmove`, `memset`,
`memcmp`, `memchr`, `strlen`, `strnlen`, `strcmp`, `strncmp`, `strcpy`,
`strncpy`, `strchr` — no `printf`, no `malloc`, no floating point.  The API
smoke tests link the same way, which is what keeps this honest: if the library
ever grows another libc dependency, `make apitest` stops linking.

## Building

```sh
make                     # every family in TARGETS
make TARGETS=ch32v0      # one family
make list-targets
make genlinktests        # validate ld/devices.data (no toolchain needed)
make apitest             # compile and link every public function
make stylecheck          # second opinion (scripts/checkpatch.pl)
```

Artifacts land in `lib/`: `libopenwch_<family>.a` and
`libopenwch_mini_libc_<family>.a`.

**Formatting** is clang-format's job, driven by `.clang-format` and run from
`.git/hooks/pre-commit` — never in CI.  Committing reformats the staged C
sources and re-stages them; without clang-format installed the hook does
nothing and the commit proceeds.

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

`genlink-config.mk` reads `ld/devices.data` and, for the selected `DEVICE`,
sets `-march`/`-mabi`, adds `-D<FAMILY>`, picks the family archive, and arranges
for `generated.$(DEVICE).ld` to be produced from `ld/linker.ld.S`.  Do not add
`-march`/`-mabi` by hand: mixing `ilp32` and `ilp32e` objects silently breaks
the ABI.  `-nostartfiles` is required — the vector table and reset path come
from this library, not from the toolchain's crt0.

```c
#include <libopenwch/ch32v0/gpio.h>
#include <libopenwch/qingke/nvic.h>

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

**Start a new project from
[libopenwch-template](https://github.com/LrkSeraph/libopenwch-template)**, not
from here: a `Makefile`, a `src/` and this library wired in as a submodule, and
nothing else.  **Read worked code in
[libopenwch-examples](https://github.com/LrkSeraph/libopenwch-examples)**:
`blink`, `uart_echo`, `ch582_blink`, `ch582_uart_echo` and
`ch582_ble_advertise`, which are also what this library's CI builds against
itself.

## Bluetooth LE (CH58x)

The radio is driven by **WCH's closed-source stack**, vendored under
`lib/ble/wch/` (**Apache-2.0**, not the LGPL that covers the rest — see
`NOTICE`) and wrapped in a thin lowercase_snake_case layer covering TMOS, GAP
parameters, the peripheral role, the GATT server and start-up:

```c
BLE_HEAP_DEFINE(ble_heap, BLE_HEAP_SIZE_DEFAULT);

ble_config_default(&config);
config.heap = ble_heap;
config.heap_size = sizeof(ble_heap);
config.mac = device_mac;          /* you supply the address */

if (ble_init(&config) != BLE_INIT_OK) {
	for (;;) {
	}
}

ble_gap_role_peripheral_init();
app_task = ble_tmos_task_register(app_event_handler);
ble_gap_role_set_param(BLE_GAP_ROLE_PARAM_ADVERT_DATA, sizeof(data), data);
ble_gap_role_peripheral_start_device(app_task, 0, &role_cbs);

for (;;) {
	ble_tmos_process();
}
```

It is a **wrapper, not a stack**: WCH's event-driven design is preserved (one
TMOS task, connection events as messages, `ble_tmos_process()` forever).  The
central/observer/broadcaster roles, bonding, OTA and mesh are not covered;
those names remain reachable in the vendor header.  See `lib/ble/README`.
The stack needs about 145 KB of flash and a heap the application declares.

## API conventions

* `periph_verb_object()` — `gpio_set_mode()`, `usart_set_baudrate()`.
* Constants `PERIPH_REGISTER_BIT` (`USART_CTLR1_UE`), types lowercase `_t`.
* The first argument is the peripheral base address, so one function drives
  every instance: `usart_set_baudrate(USART1, 115200)`.
* Registers go through `MMIO32(addr)` and are named as close to the WCH manual
  as the style allows.

WCH's EVT library uses CamelCase (`GPIO_Init`, `GPIOA_ModeCfg`); this library
deliberately does not.  One intentional deviation from libopencm3:
`gpio_set_mode(port, nibble, pins)` on the CH32V00x takes a single opaque
CNF/MODE nibble rather than a `(mode, cnf)` pair, because that nibble is not a
bitfield decomposition.

## Layout

```
Makefile            TARGETS-based recursive build
.clang-format       the formatting authority (.git/hooks/pre-commit)
mk/                 reusable build modules (also all an application needs)
scripts/            genlink.py, irq2nvic_h, genlinktest.sh, checkpatch.pl
ld/                 devices.data + linker.ld.S + tests
include/libopenwch/ qingke/ (core)  ch32v0/  ch5xx58x/  dispatch/  ble/
lib/                Makefile.include + per-family sources + mini_libc/ + ble/
doc/  tests/        Doxygen; per-family API smoke tests
```

## Licence and references

`include/`, `lib/`, `ld/`, `mk/` are **LGPL-3.0-or-later** (`LICENSE`), the same
split libopencm3 uses, so applications that merely link the library are
unaffected.  `scripts/checkpatch.pl` is **GPL-2.0** (`COPYING.GPL2`), imported
from the Linux kernel.  `lib/ble/wch/` is WCH's, **Apache-2.0**.  Full
provenance is in `NOTICE`.

Built on [libopencm3](https://github.com/libopencm3/libopencm3) (API style,
build system, structure) and [ch32fun](https://github.com/cnlohr/ch32fun)
(toolchain strategy).  WCH's EVT packages for the CH32V003 and CH58x were used
as the register-level reference, not copied.  Nothing here is affiliated with or
endorsed by Nanjing Qinheng Microelectronics.
