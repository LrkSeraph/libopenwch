# libopenwch

An open-source peripheral driver library for **WCH (Nanjing Qinheng) RISC-V
microcontrollers**, written in the style of
[libopencm3](https://github.com/libopencm3/libopencm3): a small, direct
register-level API with **lowercase_snake_case** names, a family/subfamily
directory layout, and a `Makefile`-based build that generates the right linker
script from the part number.

## Status: incubating — not ready for production

libopenwch is **pre-1.0 and in active incubation**.  The public API is not frozen and
breaking changes land between commits.  Please read this section before depending on
anything here.

What that means concretely:

- **Only two families exist so far** — `ch32v0` and `ch5xx58x`.  The rest of the WCH
  line-up is planned, not written.
- **Nothing has been verified on silicon.**  No development board and no WCH-Link are
  available to this project, so hardware-in-the-loop testing is outstanding for every
  driver.  What *is* verified is that the entire public API compiles and links, that the
  archives carry the right ISA, and that generated code matches the reference manual's
  register descriptions.  That is not the same thing as having run it.
- **A known hardware-dependent question is open**: the `DBGMCU_CR` bit layout
  (CSR `0x7c0`), where WCH's EVT and ch32fun disagree.  A value was chosen, raw accessors
  are provided, and it needs a real part to settle.  See known issue K4 in `status.md`.
- **The Bluetooth LE layer links WCH's closed-source stack** and has never executed on
  a chip.

**Do not use this in production.**  `status.md` has the current progress and the open
blockers, `phase.md` the staged plan, and `project.md` the full design.

| Family | Parts | Core | State |
|---|---|---|---|
| `ch32v0` | CH32V003, CH32V002, CH32V004, CH32V005, CH32V006, CH32V007 | QingKe V2, RV32EC | driver API complete, **not run on hardware** — 316 public functions, 15 peripherals |
| `ch5xx58x` | CH582, CH583, CH584, CH585 | QingKe V4, RV32IMAC | driver API complete, **not run on hardware** — 230 public functions, 12 peripherals plus the Bluetooth LE layer |

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

### C library requirements

libopenwch calls no C library function directly, and its archives reference
only libgcc (`__mulsi3`, `__udivsi3`, `__udivdi3` and friends) — but one part
of it is not free of the C library in practice.  `openwch_reset_init()` copies
`.data` from flash to RAM and zeroes `.bss`, and whether the compiler turns
those loops into `memcpy()`/`memset()` calls depends on the compiler version:
GCC 13 emits the calls, GCC 15 inlines the loops.  A freestanding link
therefore has to supply `memcpy` and `memset` alongside libgcc.

This matters, because Debian's and Ubuntu's `gcc-riscv64-unknown-elf` ship
**no newlib at all** for the `rv32e` or `rv32imac` multilibs, so any link that
pulls in `-lc` or `-lgloss` fails outright.  The bundled mini-libc supplies
what is missing, and applications built from `template/` have two modes:

| mode | link | use when |
|---|---|---|
| default | `-lc -lgcc -lnosys` (newlib) | the toolchain has newlib |
| `LIBOPENWCH_NOSTDLIB=1` | `-nostdlib` + `libopenwch_mini_libc_<family>.a` + `-lgcc` | it does not |

```sh
make -C template/examples/blink LIBOPENWCH_NOSTDLIB=1
```

The mini-libc is a small freestanding set — `memcpy`, `memmove`, `memset`,
`memcmp`, `memchr`, `strlen`, `strnlen`, `strcmp`, `strncmp`, `strcpy`,
`strncpy`, `strchr` — built per family into its own archive, so that it carries
the right ISA and never shadows a real C library unless you ask for it.  It has
no `printf`, no `malloc` and no floating point.

The API smoke tests link the same way (`-nostdlib` plus the mini-libc), which
is what keeps this requirement honest: if the library ever grows a dependency
on some other part of the C library, `make apitest` stops linking.

## Building the library

```sh
git clone <this repo>
cd libopenwch

make                     # builds every family in TARGETS
make TARGETS=ch32v0      # one family
make list-targets        # show what can be built
make genlinktests        # validate ld/devices.data (no toolchain needed)
make stylecheck          # second opinion (scripts/checkpatch.pl)
make clean
```

Artifacts land in `lib/`:

```
lib/libopenwch_ch32v0.a
lib/libopenwch_ch5xx58x.a
lib/libopenwch_mini_libc_ch32v0.a
lib/libopenwch_mini_libc_ch5xx58x.a
```

### Formatting

Formatting is **clang-format**'s job, driven by `.clang-format`.  It runs from
`.git/hooks/pre-commit` — never in CI.  `git commit` reformats the C sources
you staged and re-stages them.  If clang-format is not installed the hook does
nothing and the commit proceeds, so a contributor without LLVM is not blocked.

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

## Bluetooth LE (CH58x)

The CH582/CH583 have a Bluetooth LE radio.  The radio is driven by **WCH's
closed-source stack**, which this project vendors under `lib/ble/wch/` and
wraps in a thin, lowercase_snake_case layer:

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

`template/examples/ch582_ble_advertise/` is that program complete: it
advertises as "libopenwch" and lights an LED when a central connects.

```sh
cd template/examples/ch582_ble_advertise
make            # LIBOPENWCH_BLE=1 and LIBOPENWCH_NOSTDLIB=1 are the defaults here
make flash
```

**The layer is a wrapper, not a stack.**  WCH's stack keeps its own
event-driven design: an application registers one TMOS task, receives
connection events as messages, and calls `ble_tmos_process()` forever.  The
layer renames and documents that API; it does not hide it.

**What it covers:** TMOS, GAP parameters, the peripheral role, the GATT
server, and start-up.  **What it does not:** the central/observer/broadcaster
roles, the bonding manager, OTA and mesh.  Those remain reachable through
WCH's own names in the vendor header, which is installed alongside the layer.
See `lib/ble/README`.

**Licensing and memory.**  `lib/ble/wch/` is WCH's, under **Apache-2.0**, not
the LGPL that covers the rest of this project — see `NOTICE`.  The stack needs
a heap the application declares (`BLE_HEAP_DEFINE`) and about 145 KB of flash.

## Starting a project

`template/` is an application skeleton, the counterpart of
[libopencm3-template](https://github.com/bonedaddy/libopencm3-template):

```sh
cp -r libopenwch/template ~/src/my-firmware
cd ~/src/my-firmware/examples/blink
make OPENWCH_DIR=~/src/libopenwch
make OPENWCH_DIR=~/src/libopenwch flash    # needs a WCH-Link programmer
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
.clang-format       the formatting authority (see .git/hooks/pre-commit)
mk/                 reusable build modules (gcc-config, genlink-config, ...)
scripts/            genlink.py, irq2nvic_h, genlinktest.sh, checkpatch.pl
ld/                 devices.data + linker.ld.S + tests
include/libopenwch/
    qingke/             core layer (≈ libopencm3's cm3/)
    ch32v0/             CH32V00x family headers and irq.json
    ch5xx58x/           CH58x family headers and irq.json
    dispatch/           device -> family header dispatch
    ble/                Bluetooth LE layer (ble/wch/ is WCH's, Apache-2.0)
lib/
    Makefile.include    single-archive rules
    qingke/             core layer implementation
    ch32v0/             CH32V00x family build
    ch5xx58x/           CH58x family build
    mini_libc/          freestanding string/memory routines
    ble/                Bluetooth LE layer; ble/wch/ is WCH's binary
template/           application skeleton (rules/, examples/)
doc/  tests/
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
