# libopenwch

[中文](README.zh-CN.md)

Open-source peripheral driver library for **WCH RISC-V MCUs**, modeled on
[libopencm3](https://github.com/libopencm3/libopencm3): snake_case API, per-family
directories, Makefile-derived ISA/linker script.

> **Incubating; hardware validation is partial.** API links and register
> descriptions follow WCH manuals. SPI NOR CRC example and `wchlink` GDB
> software breakpoints have been validated on CH32V003; most of the library has not.

| Family | Parts | ISA | State |
|---|---|---|---|
| `ch32v0` | CH32V003/002/004/005/006/007 | RV32EC | stable API surface; hardware partial |
| `ch5xx58x` | CH582/583/584/585 | RV32IMAC | drivers + BLE wrapper; hardware pending |

Open hardware questions: CH58x USB API, CH58x debug-interface flash, BLE wrapper.

## Toolchain and build

```sh
# one free toolchain
sudo apt-get install -y gcc-riscv64-unknown-elf
# or override: make PREFIX=riscv64-unknown-elf-

make                     # all TARGETS
make TARGETS=ch32v0      # one family
make list-targets
make genlinktests        # device database, no toolchain
make apitest             # link every public function
make stylecheck          # checkpatch.pl opinion
```

`mk/gcc-config.mk` probes the common RISC-V bare-metal prefixes. Archives need
libgcc only; GCC 13's `.data`/`.bss` init may pull `memcpy`/`memset`, so
`mk/libc-config.mk` links the bundled mini-libc when no newlib is available.
`LIBOPENWCH_NOSTDLIB=1` forces it. Output:
`lib/libopenwch_<family>.a`, `lib/libopenwch_mini_libc_<family>.a`.

## Using it

```make
OPENWCH_DIR ?= /path/to/libopenwch
DEVICE      ?= ch32v003f4p6
CFLAGS      += -Os -g -Wall -std=c99
OBJS        += main.o

include $(OPENWCH_DIR)/mk/genlink-config.mk
include $(OPENWCH_DIR)/mk/gcc-config.mk
all: main.elf main.bin main.hex
include $(OPENWCH_DIR)/mk/genlink-rules.mk
include $(OPENWCH_DIR)/mk/gcc-rules.mk
```

Never add `-march`/`-mabi` manually; mixing ilp32/ilp32e breaks ABI.
`-nostartfiles` is required because libopenwch owns `_start` and the vectors.

```c
#include <libopenwch/ch32v0/rcc.h>
#include <libopenwch/ch32v0/gpio.h>

int main(void) {
rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_PLL_HSI_48MHZ]);
rcc_periph_clock_enable(RCC_GPIOA);
gpio_set_mode(GPIOA, GPIO_MODE_OUT_PP, GPIO1);
for (;;)
gpio_toggle(GPIOA, GPIO1);
}
```

Clock trees are data: `rcc_hsi_configs[]` / `rcc_hse_configs[]` on
CH32V0, and `rcc_sysclk_configs[]` on CH58x.  The same call style
works on both families:

```c
rcc_clock_setup_pll(&rcc_sysclk_configs[RCC_SYSCLK_PLL_60MHZ]);
```

One call publishes `rcc_sysclk_frequency`, and the SysTick helper can
then be configured in one more call:

```c
qingke_systick_setup_hclk(rcc_sysclk_frequency);
```

## BLE (CH58x)

WCH's Apache-2.0 stack is vendored in `lib/ble/wch/` and wrapped by a thin
`ble_*` layer for TMOS, GAP, peripheral role, GATT server and startup.  The
common advertising-data fields have a typed builder in `ble/adv.h`
(`ble_adv_data_add_flags()`, `ble_adv_data_add_name()`, ...) so applications do
not have to carry hand-written AD byte arrays.  It is a wrapper, not a full
stack: central/observer, pairing, OTA and mesh are not covered.  Needs about
145 KB flash and an application-declared heap.

## API notes

* `periph_verb_object()`, first argument is the peripheral base.
* Constants `PERIPH_REGISTER_BIT`; types `_t`; MMIO via `MMIO32()` and WCH names.
* CH32V00x `gpio_set_mode(port, mode, pins)` takes WCH's opaque `GPIO_Mode_*`
  token, not `(mode, cnf)`.

## Docs and layout

```sh
make html
# doc/html/index.html, doc/<family>/html/index.html
```

```text
Makefile mk/ scripts/ ld/       build, generators, linker, device data
include/libopenwch/             qingke/, ch32v0/, ch5xx58x/, dispatch/, ble/
lib/                            per-family sources, mini-libc, BLE
doc/                            Doxygen pages/templates/theme
tests/                          per-family API smoke tests
```

## Licence

`include/`, `lib/`, `ld/`, `mk/`: LGPL-3.0-or-later. `scripts/checkpatch.pl`:
GPL-2.0. `lib/ble/wch/`: Apache-2.0. Built with reference to libopencm3,
ch32fun and WCH EVT packages; not affiliated with WCH.
