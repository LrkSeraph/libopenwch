# libopenwch

[English](README.md)

面向 **WCH RISC-V 单片机** 的开源外设驱动库，参照
[libopencm3](https://github.com/libopencm3/libopencm3) 设计：小写下划线 API、
按系列划分目录，以及根据型号自动推导 ISA 和链接脚本的 Makefile。

> **孵化中——尚未达到产品级，硬件验证仍不完整。** 公开 API 可以编译和链接，
> 寄存器描述遵循 WCH 手册。部分集成组件已在真实 CH32V003 上验证（SPI NOR
> CRC 示例和 `wchlink` GDB server 软件断点），但整个库尚未完成硬件验证。

| 系列 | 型号 | 内核 / ISA | 状态 |
|---|---|---|---|
| `ch32v0` | CH32V003/002/004/005/006/007 | QingKe V2，RV32EC | 316 个公开函数，15 个外设 |
| `ch5xx58x` | CH582/583/584/585 | QingKe V4，RV32IMAC | 230 个公开函数，12 个外设 + BLE |

仍需要真实硬件确认的开放问题：`DBGMCU_CR` 的位布局，以及 BLE 封装是否可用。

## 工具链

```sh
sudo apt-get install -y gcc-riscv64-unknown-elf
```

`mk/gcc-config.mk` 会依次探测 `riscv64-unknown-elf`、`riscv64-none-elf`、
`riscv32-unknown-elf`、`riscv-none-elf`、`riscv64-elf`、`riscv32-elf`，
也可以用 `make PREFIX=...` 覆盖。不会探测 `riscv64-linux-gnu-`。

库归档本身只需要 libgcc，但 GCC 13 会把 `openwch_reset_init()` 的
`.data`/`.bss` 初始化循环转换成 `memcpy`/`memset`。如果所选 multilib 没有
newlib，`mk/libc-config.mk` 会自动链接按族构建的 mini-libc；
`LIBOPENWCH_NOSTDLIB=1` 可强制启用。mini-libc 只提供 `mem*` 和 `str*`。

## 构建

```sh
make                     # 构建 TARGETS 中的所有系列
make TARGETS=ch32v0      # 只构建一个系列
make list-targets
make genlinktests        # 校验 ld/devices.data（不需要工具链）
make apitest             # 编译并链接每个公开函数
make stylecheck          # checkpatch.pl 第二意见
```

产物：`lib/libopenwch_<family>.a` 和
`lib/libopenwch_mini_libc_<family>.a`。格式化由 clang-format 负责，通过本地
pre-commit hook 执行，CI 不会代替格式化。

## 使用方式

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

`genlink-config.mk` 会选择 `-march`/`-mabi`、族归档和生成的链接脚本。不要
手工添加 `-march`/`-mabi`；混用 `ilp32` 和 `ilp32e` 会破坏 ABI。
`-nostartfiles` 是必需的，因为启动代码和向量表由 libopenwch 提供。

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

时钟树以数据形式描述（`rcc_hsi_configs[]`、`rcc_hse_configs[]`、
CH58x 上的 `clk_source_t`）；调用一次即可选择配置，并发布
`rcc_sysclk_frequency`、`rcc_ahb_frequency`、`rcc_apb1_frequency`、
`rcc_apb2_frequency`、`rcc_adc_frequency`。

完整应用建议从
[libopenwch-template](https://github.com/LrkSeraph/libopenwch-template) 和
[libopenwch-examples](https://github.com/LrkSeraph/libopenwch-examples) 开始。

## 蓝牙 LE（CH58x）

WCH 的闭源协议栈以 Apache-2.0 许可证内置在 `lib/ble/wch/` 下，并用一层薄
`ble_*` 封装 TMOS、GAP、peripheral role、GATT server 和启动流程。它是封装，
不是协议栈；central/observer/broadcaster、配对、OTA 和 mesh 均未覆盖。大约
需要 145 KB flash，并由应用声明堆空间。

## API 说明

* 函数命名为 `periph_verb_object()`；第一个参数是外设基地址。
* 常量为 `PERIPH_REGISTER_BIT`；类型为小写 `_t`；寄存器使用 `MMIO32()` 和
  WCH 手册名称。
* CH32V00x 的 `gpio_set_mode(port, mode, pins)` 接收 WCH 的不透明
  `GPIO_Mode_*` token，而不是 libopencm3 的 `(mode, cnf)` 对。

## 文档

```sh
make html      # 根站点 + 每个系列的 API 站点
```

输出：`doc/html/index.html`、`doc/<family>/html/index.html`；警告日志位于
`doc/*.log`。需要 Doxygen；详见 `doc/README.md`。

## 目录布局

```text
Makefile            TARGETS 递归构建
mk/                 应用也可复用的构建模块
scripts/            genlink.py、irq2nvic_h、checkpatch.pl
ld/                 devices.data、linker.ld.S、tests
include/libopenwch/ qingke/、ch32v0/、ch5xx58x/、dispatch/、ble/
lib/                按族源码、mini-libc、BLE
doc/                Doxygen 模板、说明页面、内置主题
tests/              按族的 API smoke test
```

## 许可证

`include/`、`lib/`、`ld/`、`mk/` 使用 LGPL-3.0-or-later；
`scripts/checkpatch.pl` 使用 GPL-2.0；`lib/ble/wch/` 使用 Apache-2.0。
见 `LICENSE` 和 `NOTICE`。

构建时参考了 libopencm3、ch32fun 和 WCH EVT 包。与南京沁恒微电子无隶属关系。
