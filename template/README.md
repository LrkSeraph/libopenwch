# libopenwch application template

A starting point for a firmware project built on
[libopenwch](../README.md).  Copy this directory, point it at a libopenwch
checkout, and start writing `main.c`.

This is the RISC-V/WCH counterpart of
[libopencm3-template](https://github.com/bonedaddy/libopencm3-template): the
same `rules.mk` idea, the same `PROJECT`/`DEVICE`/`OPENWCH_DIR` variables, and
a `make flash` that works without vendor tooling.

## Quick start

```sh
# 1. Get libopenwch (either as a sibling checkout or a submodule)
git clone <libopenwch-url> ~/src/libopenwch

# 2. Copy the template next to it
cp -r ~/src/libopenwch/template ~/src/my-firmware
cd ~/src/my-firmware

# 3. Build an example
cd examples/blink
make OPENWCH_DIR=~/src/libopenwch

# 4. Flash it (needs minichlink on PATH, see below)
make OPENWCH_DIR=~/src/libopenwch flash
```

Two examples are included and both build today:

| Example | Device | Family | What it does |
|---|---|---|---|
| `examples/blink` | `ch32v003f4p6` | ch32v0 | 48 MHz from the internal RC oscillator, toggles PD1 |
| `examples/uart_echo` | `ch32v003f4p6` | ch32v0 | USART1 echo at 115200 on PD5/PD6 |
| `examples/ch582_blink` | `ch582m` | ch5xx58x | 32 MHz crystal + PLL to 60 MHz, toggles PB4 |
| `examples/ch582_uart_echo` | `ch582m` | ch5xx58x | UART1 echo at 115200 on PA8/PA9 |

Each example targets one family, because the library archive is per family:
`blink` uses the CH32V00x `rcc`/`gpio` drivers and therefore only links against
`libopenwch_ch32v0.a`.  Building it with `DEVICE=ch582m` is expected to fail at
link time with undefined `rcc_*`/`gpio_*` references.  Within a family, though,
any sibling part works — `examples/ch582_blink` builds unchanged for
`ch582m`, `ch583m`, `ch584m` and `ch585m`, and `examples/blink` for every
CH32V00x part.

## Starting your own project

```sh
cp -r examples/blink examples/my_app
$EDITOR examples/my_app/main.c
cd examples/my_app && make flash
```

`main.c` only has to provide `int main(void)`.  Everything else — the reset
path, `.data`/`.bss` initialisation, the vector table and the entry point —
comes from libopenwch's QingKe core layer.

## Layout

```
template/
├── README.md
├── .gitignore
├── .vscode/                 editor configuration (IntelliSense, debug, tasks)
├── rules/
│   ├── toolchain.mk         toolchain discovery, minichlink targets
│   └── rules.mk             compile/link/flash rules
└── examples/
    ├── blink/               CH32V003
    └── ch582_blink/         CH58x core-layer bring-up
```

There is deliberately no top-level `Makefile`: a firmware project is one
directory with one `Makefile`, and each example is independent.  A top-level
`make` can still drive them all:

```sh
# build every example
for d in examples/*/; do make -C "$d" || exit 1; done
```

## The variables that matter

| Variable | Default | Meaning |
|---|---|---|
| `PROJECT` | — | basename of the output files (`blink` → `blink.elf`, `blink.bin`, `blink.hex`) |
| `DEVICE` | example specific | the part number, e.g. `ch32v003f4p6`.  Drives `-march`/`-mabi`, the linker script and which library is linked |
| `OPENWCH_DIR` | sibling of the template | path to the libopenwch checkout |
| `TEMPLATE_DIR` | `../..` from the example | path to this template directory |
| `PREFIX` | auto-detected | toolchain prefix without the trailing `-`, e.g. `riscv64-unknown-elf` |
| `CFILES` | `main.c` | C sources, basenames only |
| `AFILES` | — | assembly sources, basenames only |
| `CXXFILES` | — | C++ sources, basenames only |
| `BUILD_DIR` | `bin` | object output directory |
| `OPT` | `-Os` | optimisation level |
| `CSTD` | `-std=c99` | C standard |
| `INCLUDES` | — | extra `-I` paths |
| `DEFS` | — | extra `-D` flags |
| `CFLAGS` | — | extra compiler flags |
| `LDFLAGS` | — | extra linker flags |
| `LDLIBS` | — | extra libraries |
| `MINICHLINK` | `minichlink` | programmer binary |
| `WRITE_SECTION` | `flash` | minichlink write region |
| `LIBOPENWCH_NOSTDLIB` | — | set to `1` to link with `-nostdlib` instead of newlib |

A minimal `Makefile` for a new project is therefore just:

```make
PROJECT     = my_app
DEVICE      = ch32v003f4p6
TEMPLATE_DIR ?= $(abspath ../..)
OPENWCH_DIR  ?= $(abspath $(TEMPLATE_DIR)/..)
CFILES      = main.c
include $(TEMPLATE_DIR)/rules/rules.mk
```

## Toolchain

```sh
sudo apt-get install -y gcc-riscv64-unknown-elf
```

`rules/toolchain.mk` probes `riscv64-unknown-elf`, `riscv64-none-elf`,
`riscv32-unknown-elf`, `riscv-none-elf`, `riscv64-elf` and `riscv32-elf`, in
that order.  Override with:

```sh
make PREFIX=/opt/xpack-riscv-none-elf-gcc/bin/riscv-none-elf
```

`riscv64-linux-gnu-` is not probed: its crt and libc conventions break
bare-metal builds.

## Flashing

[minichlink](https://github.com/cnlohr/ch32fun) drives the WCH-Link and the
built-in USB ISP bootloader, needs no vendor driver, and works on Linux,
Windows and macOS.

```sh
make flash                      # write the internal flash image
make monitor                    # printf over the single-wire debug channel
make unbrick                    # recover a part that stopped answering
```

Point `MINICHLINK` at the binary if it is not on `PATH`:

```sh
make flash MINICHLINK=~/src/ch32fun/minichlink/minichlink
```

## Output files

| File | Use |
|---|---|
| `<project>.elf` | debugging, `objdump`, `gdb` |
| `<project>.bin` | `make flash`, and the vendor flash tools |
| `<project>.hex` | the WCH official flash utility |
| `<project>.map` | link map |
| `<project>.list` | disassembly with source (`make <project>.list`) |
| `generated.<device>.ld` | the linker script produced from `ld/devices.data` |

## Notes

* **`-nostartfiles` is mandatory.**  libopenwch supplies its own `_start` and
  vector table, so the toolchain's crt0 must not be linked in.  The rule is
  already in `rules/rules.mk`.
* **Do not add `-march`/`-mabi` by hand.**  They come from `DEVICE` through
  `ld/devices.data`, so the application is always built for the same ISA as the
  library it links against.  Mixing `ilp32` and `ilp32e` objects silently
  corrupts the ABI.
* **`gpio_set_mode()` takes one opaque nibble**, not libopencm3's
  `(mode, cnf)` pair.  See
  `include/libopenwch/ch32v0/common/gpio_common_v1.h` for why.
* The CH58x peripheral drivers are the P3 milestone; only the core layer is
  available for that family today.
