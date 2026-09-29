# libopenwch

[English](README.md)

面向 **WCH RISC-V 单片机** 的开源外设驱动库，参照
[libopencm3](https://github.com/libopencm3/libopencm3) 设计：小写下划线 API、
按系列划分目录、Makefile 自动推导 ISA 与链接脚本。

> **孵化中，硬件验证不完整。** 公开 API 可编译链接，寄存器描述遵循 WCH 手册。
> SPI NOR CRC 示例和 `wchlink` GDB 软件断点已在 CH32V003 上验证，其余大量外设未硬件验证。

| 系列 | 型号 | ISA | 状态 |
|---|---|---|---|
| `ch32v0` | CH32V003/002/004/005/006/007 | RV32EC | API 基本稳定；硬件部分验证 |
| `ch5xx58x` | CH582/583/584/585 | RV32IMAC | 驱动 + BLE 封装；硬件待验 |

待硬件确认：CH58x USB API、CH58x 调试接口烧录、BLE 封装。

## 工具链与构建

```sh
sudo apt-get install -y gcc-riscv64-unknown-elf
make                     # 全部 TARGETS
make TARGETS=ch32v0      # 单系列
make list-targets
make genlinktests        # 设备数据库，无需工具链
make apitest             # 链接全部公开函数
make stylecheck
```

`mk/gcc-config.mk` 探测常见裸机 RISC-V 前缀，可用 `make PREFIX=...` 覆盖。
归档只需要 libgcc；GCC 13 的 `.data`/`.bss` 初始化可能引用
`memcpy`/`memset`，无 newlib 时 `mk/libc-config.mk` 自动链接内置 mini-libc，
`LIBOPENWCH_NOSTDLIB=1` 可强制。产物：`lib/libopenwch_<family>.a`、
`lib/libopenwch_mini_libc_<family>.a`。

## 使用方式

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

不要手工添加 `-march`/`-mabi`；混用 ilp32/ilp32e 会破坏 ABI。
`-nostartfiles` 必需，启动代码与向量表由本库提供。

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

时钟树是数据（`rcc_hsi_configs[]`、`rcc_hse_configs[]`、CH58x
`clk_source_t[]`），一次调用发布 `rcc_*_frequency`。

## BLE（CH58x）

WCH 的 Apache-2.0 协议栈内置在 `lib/ble/wch/`，用薄 `ble_*` 层封装 TMOS、
GAP、peripheral、GATT server 和启动。它只是封装：central/observer、配对、
OTA、mesh 未覆盖；约需 145 KB flash，堆由应用声明。

## API 说明

* 函数 `periph_verb_object()`，首参外设基址。
* 常量 `PERIPH_REGISTER_BIT`，类型 `_t`，寄存器用 `MMIO32()` 和 WCH 名称。
* CH32V00x `gpio_set_mode(port, mode, pins)` 收 WCH 不透明 `GPIO_Mode_*`，
  不是 libopencm3 的 `(mode, cnf)`。

## 文档与目录

```sh
make html
# doc/html/index.html、doc/<family>/html/index.html
```

```text
Makefile mk/ scripts/ ld/       构建、生成器、链接脚本、设备数据
include/libopenwch/             qingke/、ch32v0/、ch5xx58x/、dispatch/、ble/
lib/                            按族源码、mini-libc、BLE
doc/                            Doxygen 页面/模板/主题
tests/                          按族 API smoke test
```

## 许可证

`include/`、`lib/`、`ld/`、`mk/`：LGPL-3.0-or-later；
`scripts/checkpatch.pl`：GPL-2.0；`lib/ble/wch/`：Apache-2.0。
参考 libopencm3、ch32fun 与 WCH EVT；与 WCH 无隶属关系。
