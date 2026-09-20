# libopenwch — 项目规划（project.md）

> 本文件是 libopenwch 的**总体设计文档**。阶段性目标见 `phase.md`，当前完成情况见 `status.md`。
> 三个文件由 `AGENTS.md` 统一引用。

---

## 1. 项目定位

libopenwch 是一个面向 **南京沁恒（WCH）RISC-V MCU** 的开源外设驱动库。

目标：**在 API 风格、目录结构、构建系统、代码规范四个维度上对齐 libopencm3**，为非 ARM 的
WCH RISC-V 芯片提供一套「小、直、无 HAL 中间层」的驱动库。

一句话类比：

| libopencm3 | libopenwch |
|---|---|
| `arm-none-eabi-gcc` | `riscv64-unknown-elf-gcc` |
| `cm3/`（Cortex-M 核心层） | `qingke/`（沁恒 QingKe RISC-V 核心层） |
| `stm32/f1`、`efm32/tg` … | `ch32v003`、`ch582` … |
| `nvic_enable_irq()`（ARM NVIC） | `nvic_enable_irq()`（PFIC 兼容实现，同名 API） |
| `ld/devices.data` + `scripts/genlink.py` | 同一机制，扩展 RISC-V 字段 |
| 小写非驼峰：`gpio_set_mode()` | 小写非驼峰：`gpio_set_mode()` |

### 1.1 硬性需求（来自项目发起）

1. **遵循 libopencm3 一致的风格**，提供**非驼峰命名**（lowercase_snake_case）的 API 体系。
   - `gpio_set_mode()`、`rcc_clock_setup_hsi()`、`usart_set_baudrate()`、`tim_set_prescaler()`
   - 常量全大写前缀式：`GPIO_MODE_OUTPUT_50MHZ`、`USART_CR1_UE`
   - 明确**不使用** WCH EVT 的驼峰命名（`GPIO_Init`、`GPIOA_ModeCfg`、`RCC_GetClocksFreq`）。
2. **按 WCH MCU 体系划分分类**，粒度与 libopencm3 的 `stm32/f1` 相当。
   - `ch32v003`（以及同族 `ch32v002/004/005/006/007`）、`ch32v103`、`ch32v203`、`ch32v303` …
   - `ch582`、`ch583`、`ch584`、`ch585`、`ch571`、`ch573`、`ch591`、`ch592`
   - 目录/目标命名同时保留 `ch32vxx` / `ch5xx` 两级族名，例如 `ch32v/003`、`ch5xx/58x`。
3. **构建系统与 libopencm3 保持一致**：
   - 根 `Makefile` 的 `TARGETS` / `lib/` 递归 / `mk/` 可复用组件；
   - `ld/devices.data` + `scripts/genlink.py` 的"设备名 → 链接脚本"自动生成；
   - 用户侧 `OPENCM3_DIR` + `include $(OPENWCH_DIR)/mk/genlink-config.mk` 的用法保持一致。
4. **兼容 `gcc-riscv64-unknown-elf`**，并借鉴 **ch32fun** 的改动：
   - 工具链前缀自动探测（多候选 prefix 回退链）；
   - MCU → `-march`/`-mabi` 映射表；
   - 旧版本 GCC 不支持 `zmmul` 时自动降级；
   - 可选 `-msmall-data-limit` / `__global_pointer$` 支持。
5. **前期先实现 `ch32v003` 与 `ch582M`**（`ch582` 及 `ch583/584/585` 同族）。

### 1.2 非目标（明确排除）

- 不做 Arduino / 类 HAL 的面向对象封装。
- 不绑定 RTOS，不引入动态内存。
- 不复刻 WCH EVT 的 `StdPeriphDriver` API（只作为寄存器事实来源）。
- 不实现 BLE 协议栈 / USB 协议栈（USB *设备控制器*寄存器级驱动在后期阶段可选）。
  BLE 侧改为**链接 WCH 的闭源二进制**并在其上提供薄封装层，见 `lib/ble/README`。
- **本仓库不含任何 host 侧 USB 代码**，也不提供编程器实现，见 §1.3。
- `ch32v003` 的 `ch32fun` 式极简 runtime **不**直接采用；本库走 libopencm3 的
  "静态库 + 链接脚本生成 + 可选 newlib"路线。ch32fun 仅作为**工具链兼容方案的参考**。

### 1.3 伴随工具定位（WCH-LinkE 编程器）

本项目的定位分两层。**第一层是对外身份，第二层只在内部文档记录**：

1. **主定位（对外）**：与 libopencm3 对齐的 WCH RISC-V 外设驱动库。
   `README.md` 只讲这一层，其中不出现编程器工具的宣传。
2. **伴随定位（内部）**：为 **WCH-LinkE 编程器**提供烧录与调试能力的配套工具。

**边界（硬性约束）**：本仓库**不含任何 host 侧 USB 代码**，也**不再包含应用模板与示例**。
库的构建只需要一条 RISC-V 工具链，不引入 `libusb`/`pkg-config`/`udev`。
模板与示例已拆到两个独立仓库，其中的 `flash`/`monitor`/`unbrick` 只做**委派**——
把工作交给外部编程器工具，自身不实现协议。

**四个仓库的分工**：

| 仓库 | 角色 | 关系 |
|---|---|---|
| `libopenwch`（本仓库） | 驱动库 | — |
| `libopenwch-template` | 应用骨架 | 独立仓库；用户从它开始建项目。它以 **submodule** 挂载本库（`libopenwch/`）与工具（`tools/wchlink/`） |
| `libopenwch-examples` | 逐个外设的完整示例 | 独立仓库；同样以 **submodule** 挂载本库。它同时是本库 CI 的集成测试对象（CI 克隆它并用当前 checkout 构建全部示例） |
| `libopenwch-tools` | WCH-LinkE 烧录器（`wchlink`） | 独立仓库；不作为本库的 submodule，只由 template 以 **submodule** 挂在 `tools/wchlink/` |

模板与示例**不是**本库的 submodule：它们是用户可见的独立工程，按 libopencm3 /
libopencm3-template 的成例应当独立可用。工具被 template 以 submodule 引用：
模板的 `make flash PROGRAMMER=wchlink` 需要它的源码在树内。

**载体**：本仓库不挂任何 submodule，因此 `git clone` 与 CI 都不受影响，也不需要 libusb。
`PROGRAMMER=wchlink` 在 template 中默认指向 `tools/wchlink/build/wchlink`，
在 examples 中则只查 `PATH`（它不挂该 submodule）。

**为什么不直接集成**（评估结论）：

| 维度 | 库（本仓库） | WCH-LinkE 工具 |
|---|---|---|
| ISA / 工具链 | 仅 `riscv64-unknown-elf` | host `x86_64` gcc |
| 构建驱动 | `TARGETS` 按族递归 | 单个 host 可执行文件 |
| 产物 | `lib/libopenwch_<family>.a` | 可执行文件 |
| 依赖 | 仅一条 RISC-V 工具链 | `libusb-1.0`、`libudev`、`pthread` |
| 受众 | 固件作者 | 任何持有 WCH-LinkE 的人，不限本库用户 |

集成在技术上**可行**，但会把第二套工具链、一组 host 依赖和一类新产物塞进一个以
「`TARGETS` 递归 + 一条 RISC-V 工具链」为身份的构建里，并与「对齐 libopencm3」的定位冲突
——libopencm3 自身也不提供编程器。生态同样如此：`wlink`、`minichlink`、
`riscv-openocd-wch` 都是独立工程。因此选择**分离仓库 + submodule**，并保留现有的
「委派」边界。工具自身的规划见 `phase.md` 的 P6。

---

## 2. 参考项目与各自的角色

工作区 `incubator_workspace/` 中四个参考工程：

| 目录 | 角色 | 具体借鉴内容 |
|---|---|---|
| `libopencm3/` | **主模板** | 目录结构、`TARGETS` 递归构建、`mk/*.mk`、`genlink`、`devices.data`、`irq.json`→`nvic.h` 生成器、Doxygen 规范、`HACKING` 代码规范、per-target `Makefile` 结构、`common/` 变体共享机制、LGPL 许可 |
| `ch32fun-master/` | **工具链适配参考** | `riscv64-unknown-elf` 前缀探测链、`-march`/`-mabi` 表、GCC13 `zmmul` 特判、`misc/libgcc.a` 兜底、RISC-V 启动汇编/链接脚本细节、`__global_pointer$` 与 `.highcode` 处理 |
| `ch32v003-main/` | **CH32V003 寄存器/时序事实来源** | `Peripheral/inc/ch32v00x*.h` 的寄存器布局与位定义、`Core/core_riscv.h` 的 PFIC/CSR 访问、`Startup/startup_ch32v00x.S` 的向量表与启动流程、`Ld/Link.ld` 的内存与段布局、`RCC/GPIO/TIM/...` 各 EXMA 的寄存器操作序列 |
| `ch583-main/` | **CH582/583 寄存器/时序事实来源** | `StdPeriphDriver/inc/CH583SFR.h`（`R8_/R16_/R32_/RB_` 原始寄存器命名）、`CH58x_clk.c`/`CH58x_sys.c` 时钟树、RWA 安全访问（`0x57/0xA8` 签名）协议、`Link.ld`/`startup_CH583.S` |

**重要的架构事实（决定设计）**

- WCH 的 `ch32v003` 使用 **RV32EC（`-march=rv32ec -mabi=ilp32e`）**；
  `ch58x` 使用 **RV32IMAC（`-march=rv32imac -mabi=ilp32`）**。两者**不能**放进同一个
  静态库，必须按族/子族拆库——这与 libopencm3 的 `libopencm3_stm32f1.a` 分库方式一致。
- WCH 无 ARM NVIC，但有 **PFIC（Programmable Fast Interrupt Controller）**，
  其寄存器块位于 `0xE000E000`，且**寄存器布局刻意模仿 NVIC**：
  `IENR[8]` ≈ `NVIC_ISER`、`IRER[8]` ≈ `NVIC_ICER`、`IPRIOR[256]` ≈ `NVIC_IPR`。
  因此 libopencm3 的 `nvic_enable_irq()/nvic_disable_irq()/nvic_set_priority()` **可以同名保留**。
- WCH 的中断入口是 RISC-V 裸向量表（**不是** ARM 的 `initial_sp_value + reset + exceptions` 结构），
  表项是 `.word handler`，第 0 项是一条 `j handle_reset` 指令（`mtvec` 的 MODE=1 向量模式，
  且 WCH 用 mode `3` 表示"向量表项是跳转指令"）。
- `ch58x` 的多数外设寄存器（时钟、电源、GPIO、Flash 控制器）是 **RWA（Read-Write-Auto-safe）**，
  写入前必须先解锁：向 `R8_SAFE_ACCESS_SIG` 依次写 `0x57` 再写 `0xA8`。
  `ch32v003` 无此机制。这一点必须由驱动层封装，不能泄漏给用户。

---

## 3. 目录结构（project layout）

与 libopencm3 保持 1:1 的心智模型。**族（family）→ 子族（subfamily）→ 器件（device）** 三级。

```
libopenwch/
├── Makefile                      # 根：TARGETS 列表 + 递归构建 + 代码生成
├── README.md
├── COPYING.LGPL3                 # 库代码 LGPL-3.0-or-later（与 libopencm3 一致）
├── COPYING.GPL3                  # 生成脚本等工具 GPL-3.0-or-later
├── HACKING                       # 代码规范（直接继承 libopencm3 的 HACKING）
├── HACKING_COMMON_DOC            # 新族接入清单（继承并改写）
├── .gitignore
├── .clang-format                 # 参考 ch32fun 的 .clang-format，内核风格
│
├── mk/                           # 可复用构建组件（应用侧 include）
│   ├── README                    # 用法说明（与 libopencm3 对齐）
│   ├── gcc-config.mk             # RISC-V 工具链前缀自动探测
│   ├── gcc-rules.mk              # %.elf / %.bin / %.hex / %.o 通用规则
│   ├── genlink-config.mk         # 设备名 → CPU/MARCH/MABI/ROM/RAM → ARCH_FLAGS
│   ├── genlink-rules.mk          # generated.$(DEVICE).ld 生成规则
│   ├── openwch-config.mk         # 库名/库路径/头文件路径自动装配（可选合并进 genlink-config.mk）
│   └── openwch-rules.mk          # 用户工程可选的 convenience target（flash/size/...），无硬件依赖
│
├── scripts/
│   ├── genlink.py                # 移植自 libopencm3，新增 MARCH/MABI/ARCH/ZMMUL 处理
│   ├── irq2nvic_h                # 移植自 libopencm3，生成 RISC-V 风格向量表
│   ├── genlinktest.sh            # 移植：链接脚本冒烟测试
│   ├── gendoxylist                # 移植：Doxygen 源文件清单
│   ├── gendoxylayout.py           # 移植：Doxygen layout
│   └── checkpatch.pl              # 直接继承 libopencm3 的 Linux 风格检查
│
├── ld/
│   ├── devices.data              # WCH 器件树（本项目的核心"单一事实来源"）
│   ├── linker.ld.S               # 通用链接脚本模板（RISC-V 版，含 .init/.highcode/gp）
│   ├── README
│   └── tests/                    # genlink 冒烟测试数据（.data）
│
├── include/
│   ├── libopenwch/
│   │   ├── qingke/               # ← 对应 libopencm3 的 cm3/（RISC-V 核心层）
│   │   │   ├── common.h          # MMIO8/16/32、BIT0..、BEGIN_DECLS、stdint
│   │   │   ├── memorymap.h       # CORE_PERIPH_BASE=0xE0000000 等
│   │   │   ├── csr.h             # mstatus/mtvec/mepc/mcause、__enable_irq 等
│   │   │   ├── pfic.h            # PFIC 寄存器块（= NVIC 寄存器布局）
│   │   │   ├── nvic.h            # 在 PFIC 之上提供 nvic_* API（同名于 libopencm3）
│   │   │   ├── systick.h         # 0xE000F000 SysTick（WCH 专有，非 RISC-V 标准）
│   │   │   ├── vector.h          # vector_table_t（RISC-V 变体）
│   │   │   ├── assert.h
│   │   │   ├── sync.h
│   │   │   ├── doc-qingke.h      # Doxygen 分组
│   │   │   └── doc-csr.h
│   │   ├── ch32v.h               # 族通用层（V 系列共用：PFIC 时序、Flash、ESIG…）
│   │   ├── ch5xx.h               # 族通用层（5xx/BLE 系列共用：RWA 安全访问、时钟…）
│   │   │
│   │   ├── ch32v0/               # 族目录：CH32V002/003/004/005/006/007（RV32EC）
│   │   │   ├── common/
│   │   │   │   ├── gpio_common_v1.h
│   │   │   │   ├── rcc_common_v1.h
│   │   │   │   ├── usart_common_v1.h
│   │   │   │   ├── tim_common_v1.h
│   │   │   │   ├── spi_common_v1.h
│   │   │   │   ├── i2c_common_v1.h
│   │   │   │   ├── adc_common_v1.h
│   │   │   │   ├── dma_common_v1.h
│   │   │   │   ├── exti_common_v1.h
│   │   │   │   ├── iwdg_common_v1.h
│   │   │   │   ├── wwdg_common_v1.h
│   │   │   │   ├── pwr_common_v1.h
│   │   │   │   ├── flash_common_v1.h
│   │   │   │   ├── opa_common_v1.h
│   │   │   │   └── syscfg_common_v1.h   # AFIO/EXTI 复用
│   │   │   └── doc-ch32v0.h
│   │   ├── ch32v003/             # 器件目录（薄包装）
│   │   │   ├── gpio.h  rcc.h  usart.h  tim.h  spi.h  i2c.h  adc.h
│   │   │   ├── dma.h  exti.h  iwdg.h  wwdg.h  pwr.h  flash.h  opa.h  dbgmcu.h
│   │   │   ├── memorymap.h       # 0x40000000/0x40010000/0x40020000 总线与 *_BASE
│   │   │   ├── irq.json          # 中断号表（由 irq2nvic_h 生成 nvic.h）
│   │   │   ├── nvic.h            # 生成物
│   │   │   └── doc-ch32v003.h
│   │   ├── ch32v103/ …           # 规划中的其他 V 系列（仅占位）
│   │   ├── ch5xx/                # 族目录：CH57x/58x/59x（RV32IMAC）
│   │   │   ├── common/
│   │   │   │   ├── rwa_common_v1.h      # 安全访问解锁协议
│   │   │   │   ├── clk_common_v1.h      # SetSysClock 等价物
│   │   │   │   ├── gpio_common_v1.h     # 位域式 GPIO（R32_PA_DIR/OUT/PIN）
│   │   │   │   ├── uart_common_v1.h
│   │   │   │   ├── spi_common_v1.h
│   │   │   │   ├── i2c_common_v1.h
│   │   │   │   ├── tim_common_v1.h
│   │   │   │   ├── pwm_common_v1.h
│   │   │   │   ├── adc_common_v1.h
│   │   │   │   ├── flash_common_v1.h
│   │   │   │   ├── pwr_common_v1.h
│   │   │   │   └── sys_common_v1.h
│   │   │   └── doc-ch5xx.h
│   │   ├── ch582/                # 器件目录（薄包装）
│   │   │   ├── gpio.h  rcc.h  clk.h  uart.h  spi.h  i2c.h  tim.h  pwm.h
│   │   │   ├── adc.h  flash.h  pwr.h  sys.h
│   │   │   ├── memorymap.h
│   │   │   ├── irq.json  nvic.h
│   │   │   └── doc-ch582.h
│   │   ├── ch583/ ch584/ ch585/ …  # 规划中的同族（仅占位）
│   │   ├── usb/                  # 复用 libopencm3 的 usb 栈（后期，ARM 无关部分）
│   │   ├── docmain.dox
│   │   ├── license.dox
│   │   └── doc-peripheral-apis.h
│   └── libopencmsis/             # CMSIS 兼容层（后期；WCH EVT 代码迁移用）
│
├── lib/
│   ├── Makefile.include          # 单库归档规则（LIBNAME + OBJS → libopenwch_*.a）
│   ├── qingke/                   # 核心层实现（每库都编译）
│   │   ├── vector.c              # 向量表 + reset_handler + trap dispatch
│   │   ├── vector_chipset.c      # 芯片相关早期初始化
│   │   ├── vector_nvic.c         # 由 irq.json 生成的弱 IRQ handler 表
│   │   ├── nvic.c                # PFIC → nvic_* API
│   │   ├── assert.c  sync.c  csr.c  systick.c  dwt.c
│   ├── common/                   # 与架构无关的公共实现
│   │   └── (预留)
│   ├── ch32v/                    # 族聚合层
│   │   ├── Makefile.include      # 族级 TGT_CFLAGS（-march=rv32ec…）
│   │   ├── common/               # 族级共享 C 实现（V 系列共通时序）
│   │   └── 003/                  # ← 目标目录（TARGETS 中写 ch32v/003）
│   │       ├── Makefile          # LIBNAME=libopenwch_ch32v003, OBJS, VPATH
│   │       ├── gpio.c  rcc.c  usart.c  tim.c  spi.c  i2c.c  adc.c
│   │       ├── dma.c  exti.c  iwdg.c  wwdg.c  pwr.c  flash.c  opa.c  syscfg.c
│   │       └── (VPATH 引 ../common)
│   ├── ch5xx/
│   │   ├── Makefile.include
│   │   ├── common/
│   │   └── 58x/
│   │       ├── Makefile          # LIBNAME=libopenwch_ch582
│   │       └── gpio.c rwa.c clk.c sys.c pwr.c flash.c uart.c spi.c i2c.c tim.c pwm.c adc.c
│   ├── ethernet/                 # 规划中
│   └── usb/                      # 规划中
│
├── doc/
│   ├── Makefile                  # Doxygen（移植，TARGETS 换成 WCH）
│   ├── templates/{Doxyfile_Device,DoxygenLayout_Root.xml,DoxygenLayout_Device.xml}
│   └── source/ …
│
├── tests/
│   ├── rules.mk                  # 通用应用侧 Makefile 片段
│   ├── shared/
│   ├── ch32v003-generic/         # 板级/最小系统冒烟工程（blink/uart echo）
│   └── ch582-generic/
│
├── examples/                     # 已迁出：见下方说明
│
├── project.md  phase.md  status.md  AGENTS.md
└── .github/workflows/ci.yml      # 多工具链矩阵 + 示例集成测试
```

上图中的 `examples/` 只保留位置说明：面向用户的示例已迁到独立仓库
**`libopenwch-examples`**（`examples/<名字>/`，每个含 `main.c` + `Makefile`），
应用骨架在 **`libopenwch-template`**（`Makefile` + `src/`）。两者都以 submodule
引用本库，本仓库不再持有它们。CI 的 `examples` job 会克隆 examples 仓库并用
**当前 checkout** 构建全部示例，见 `.github/workflows/ci.yml`。

### 3.1 分类映射表（WCH 体系 ↔ 本库目标名 ↔ 库文件名）

| WCH 系列 | 核心 ISA | `TARGETS` = 库目录 = `FAMILY` | 静态库 |
|---|---|---|---|
| CH32V003 / 002 / 004 / 005 / 006 / 007 | RV32EC（002+ 带 zmmul） | `ch32v0` | `lib/libopenwch_ch32v0.a` |
| CH32V103 / 203 / 303 | RV32IMAC | `ch32v0v4` | `lib/libopenwch_ch32v0v4.a` |
| CH32X035 | RV32IMAC | `ch32x0` | `lib/libopenwch_ch32x0.a` |
| CH32L103 | RV32IMAC | `ch32l1` | `lib/libopenwch_ch32l1.a` |
| **CH582 / CH583 / CH584 / CH585** | RV32IMAC | **`ch5xx58x`** | `lib/libopenwch_ch5xx58x.a` |
| CH570 / 571 / 572 / 573 | RV32IMAC | `ch5xx57x` | `lib/libopenwch_ch5xx57x.a` |
| CH591 / CH592 | RV32IMAC | `ch5xx59x` | `lib/libopenwch_ch5xx59x.a` |

> 一期启用 `ch32v0` 与 `ch5xx58x`；其余族已在 `ld/devices.data` 中定义，但尚未创建
> 对应的 `lib/` 目录，因此不进入 `TARGETS`。
>
> **为什么用单层名字而不是 `ch32v/003`**：`genlink.py` 的 `FAMILY` 直接决定
> `-lopenwch_<family>`，必须是一个合法库名；让 `TARGETS`、`lib/<family>/` 目录名、
> 归档名三者取同一个值，就不需要任何映射表。libopencm3 的 `stm32/f1` 之所以能嵌套，
> 是因为它的 `FAMILY` 是 `stm32f1` 而目录是 `stm32/f1`——同一份信息写了两遍。
> CH32V00x 各型号 ISA 相同，共享一个归档，符合「一库一族」约束。

---

## 4. 核心层设计（`qingke/` ↔ libopencm3 的 `cm3/`）

这是本项目**与 libopencm3 差异最大**的一层，必须单独设计。

### 4.1 向量表

libopencm3（ARM）：

```c
typedef struct {
    unsigned int *initial_sp_value;
    vector_table_entry_t reset, nmi, hard_fault, ... systick;
    vector_table_entry_t irq[NVIC_IRQ_COUNT];
} vector_table_t;
```

libopenwch（RISC-V / WCH）：向量表是**裸字数组**，表项为跳转指令或函数地址。
WCH 约定：`mtvec = _start | 3`（MODE=3，向量表每项是一条 `j` 指令）。

```c
/* include/libopenwch/qingke/vector.h */
typedef void (*vector_table_entry_t)(void);

/* WCH/RISC-V: 第 0 项必须是 j handle_reset（由启动汇编提供），其余为 handler 地址。
   表项以 .word 形式存放在 .init 段，需 4 字节对齐（norvc + align 2）。 */
typedef struct {
    uint32_t reset_jump;                       /* j _reset_handler */
    uint32_t reserved_x004;                    /* 0 */
    vector_table_entry_t nmi;                  /* 2  */
    vector_table_entry_t hard_fault;           /* 3  */
    uint32_t reserved_x010[8];                 /* 4..11 */
    vector_table_entry_t systick;              /* 12 */
    uint32_t reserved_x034;                    /* 13 */
    vector_table_entry_t sw;                   /* 14 */
    uint32_t reserved_x03c;                    /* 15 */
    vector_table_entry_t irq[OPENWCH_IRQ_COUNT]; /* 16.. */
} vector_table_t;
```

启动流程（等价于 libopencm3 的 `reset_handler`，但用汇编 + C 混合）：

1. `.init` 段头部：`j handle_reset`；后面紧跟向量表数据。
2. `handle_reset`（汇编，位于 `lib/qingke/vector_chipset.c` 的裸 asm 块或独立 `.S`）：
   - `la gp, __global_pointer$`（`norelax`）
   - `la sp, _eusrstack`
   - 拷贝 `.highcode`（VMA in RAM / LMA in FLASH）
   - 拷贝 `.data`，清零 `.bss`
   - `csrw mstatus, 0x1880`（MPP=M，MIE=0）
   - `csrw 0x804, 0x3`（WCH 扩展：使能中断嵌套 + 硬件堆栈）
   - `csrw mtvec, (_start | 3)`
   - `jal SystemInit`（弱符号，用户可覆盖；本库提供默认实现调用 `rcc_clock_setup_*`）
   - `la t0, main`；`csrw mepc, t0`；`mret`
3. `SystemInit()` 默认实现：`rcc_clock_setup_hsi(...)`，然后可选调用 `__libc_init_array()`
   （由 `FUNCONF_SUPPORT_CONSTRUCTORS` 等价开关 `OPENWCH_SUPPORT_CONSTRUCTORS` 控制）。

### 4.2 异常/中断分发

- RISC-V 陷阱入口由 `mtvec` 指向向量表；WCH 的 PFIC 在中断时自动把中断号
  映射到向量表偏移，**无需软件再读 `mcause` 分派外部中断**。
- 但 **异常**（`mcause` MSB=0）必须由软件处理：`Ecall_M_Mode_Handler`、`NMI_Handler`、
  `HardFault_Handler` 等，由 `irq2nvic_h` 生成弱定义，用户可覆盖。
- 默认弱 handler 实现为 `DefaultIRQHandler()`：置 `OPENWCH_ASSERT`/死循环，
  在 `OPENWCH_ASSERT_DEBUG` 下打印 `mcause`/`mepc`。

### 4.3 `nvic_*` API（同名保留）

PFIC 寄存器布局与 NVIC 同构，实现直接映射：

```c
/* lib/qingke/nvic.c */
void nvic_enable_irq(uint8_t irqn)  { PFIC_IENR(irqn >> 5) = 1U << (irqn & 0x1f); }
void nvic_disable_irq(uint8_t irqn) { PFIC_IRER(irqn >> 5) = 1U << (irqn & 0x1f); }
void nvic_set_priority(uint8_t irqn, uint8_t prio) { PFIC_IPRIOR(irqn) = prio; }
uint8_t nvic_get_priority(uint8_t irqn) { return PFIC_IPRIOR(irqn); }
/* WCH 专有（libopencm3 有 nvic_set_priority_grouping 的等价物） */
void nvic_set_priority_grouping(uint32_t group);   /* NVIC_PriorityGroup_0/1 */
void nvic_enable_irq_nesting(bool enable);          /* CSR 0x804 bit1 */
```

保留 `nvic_*` 名称是**有意**的：让从 libopencm3 迁移的用户代码零改动，
文档中会注明 "PFIC, not ARM NVIC"。

### 4.4 全局中断开关与临界区

```c
#define OPENWCH_IRQ_ENABLE()   csrs(mstatus, 0x88)
#define OPENWCH_IRQ_DISABLE()  csrc(mstatus, 0x88); fence.i
```

提供 `qingke_irq_enable()/qingke_irq_disable()/qingke_irq_save()/qingke_irq_restore()`，
对应 libopencm3 `cm3/sync.h` 的语义（`cm3_irq_save` → `qingke_irq_save`）。

### 4.5 与 libopencm3 逐项对照（核心层）

| libopencm3 (`cm3/`) | libopenwch (`qingke/`) | 说明 |
|---|---|---|
| `MMIO8/16/32/64` | 同名 | 直接沿用 |
| `BIT0..BIT31` | 同名 | 直接沿用 |
| `SCB_*`、`cortex.h`、`mpu.h`、`fpb.h`、`itm.h`、`tpiu.h`、`dwt.h` | **删除** | ARM 专有 |
| `systick.h`（ARM SysTick） | `systick.h`（WCH `0xE000F000`，寄存器不同） | 保留 API 名 `systick_set_reload` 等 |
| `nvic.h`（ARM NVIC） | `nvic.h`（PFIC 同构） | API 同名 |
| `vector.h`（`initial_sp_value` 结构） | `vector.h`（裸字数组） | 结构不同，`irq[]` 成员名保留 |
| `scb.h` 的 `scb_reset_system()` | `qingke_system_reset()` | 走 WCH `R32_RST`/`PFIC` 软复位 |
| `cm3_assert` / `cm3_assert_not_reached` | `openwch_assert` / `openwch_assert_not_reached` | 语义一致 |

---

## 5. 外设 API 设计规范

### 5.1 命名

| 类别 | 规则 | 示例 |
|---|---|---|
| 函数 | `periph_verb_noun()`，小写下划线 | `gpio_set_mode()`, `gpio_set()`, `gpio_clear()`, `gpio_toggle()`, `rcc_clock_setup_hsi()`, `usart_set_baudrate()`, `usart_enable()`, `tim_set_prescaler()`, `spi_init_master()`, `adc_set_sample_time()` |
| 寄存器 | 全大写 `PERIPH_REG` | `GPIO_CFGLR`, `RCC_CFGR0`, `USART_CTLR1` |
| 位域 | 全大写 `PERIPH_REG_BIT` | `USART_CTLR1_UE`, `RCC_CFGR0_PLLSRC` |
| 类型 | 小写 `_t` | `gpio_mode_t`, `usart_parity_t` |
| 配置枚举 | 全大写 `PERIPH_...` | `GPIO_MODE_OUTPUT_PP`, `USART_BAUDRATE_115200` |
| 端口基址 | `GPIOA`、`GPIOB`、`GPIOC`、`GPIOD`（与 libopencm3 一致，指针常量） | |
| 引脚号 | `GPIO0..GPIO7`（与 libopencm3 的 `GPIO5` 风格一致） | |

**迁移对照（WCH EVT → libopenwch）**

| WCH EVT（驼峰） | libopenwch（非驼峰） |
|---|---|
| `GPIO_Init(GPIOA, &GPIO_InitStructure)` | `gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50MHZ, GPIO_CNF_OUTPUT_PP, GPIO0 \| GPIO1)` |
| `GPIO_SetBits(GPIOA, GPIO_Pin_1)` | `gpio_set(GPIOA, GPIO1)` |
| `GPIO_ResetBits(GPIOA, GPIO_Pin_1)` | `gpio_clear(GPIOA, GPIO1)` |
| `GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1)` | `gpio_get(GPIOA, GPIO1)` |
| `GPIO_WriteBit(...)` | `gpio_set()` / `gpio_clear()` |
| `GPIO_PinRemapConfig(GPIO_FullRemap_USART1, ENABLE)` | `gpio_primary_remap()` 或 `usart_set_remap(USART1, USART_REMAP_FULL)` |
| `GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1)` | `exti_select_source(EXTI1, GPIOA)` |
| `RCC_GetClocksFreq(&clk)` | `rcc_get_clocks_freq(&clk)`（保留，语义一致） |
| `RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE)` | `rcc_periph_clock_enable(RCC_GPIOA)` |
| `USART_Init(USART1, &USART_InitStructure)` | `usart_set_baudrate()`, `usart_set_databits()`, `usart_set_stopbits()`, `usart_set_mode()`, `usart_set_parity()`, `usart_set_flow_control()`, `usart_enable()` |
| `USART_SendData(USART1, c)` | `usart_send(USART1, c)` |
| `TIM_Cmd(TIM1, ENABLE)` | `tim_enable(TIM1)` |
| `GPIOA_ModeCfg(GPIO_Pin_1, GPIO_ModeOut_PP_5mA)` (CH58x) | `gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_PP, GPIO_DRIVE_5MA, GPIO1)` |

### 5.2 每个外设的实现分层

完全复刻 libopencm3 的"公共变体"机制：

```
include/libopenwch/ch32v0/common/gpio_common_v1.h   ← 寄存器宏 + 位定义 + 声明
include/libopenwch/ch32v003/gpio.h                  ← 器件差异（可用端口、AF 映射）+ include common
lib/ch32v/common/gpio_common_v1.c                   ← 实现（可被多个器件复用）
lib/ch32v/003/gpio.c                                ← 器件薄壳（通常只 include common .c 或做特化）
```

命名规则：`<periph>_common_<variant>.{c,h}`。变体号 `v1` 表示"第一代寄存器布局"。
`ch32v003` 与 `ch32v002/004/005/006/007` 共享 `v1`；`ch582/583/584/585` 共享 `v1`。

### 5.3 断言与错误处理

```c
/* include/libopenwch/qingke/assert.h */
#define openwch_assert(cond) \
    do { if (!(cond)) { openwch_assert_failed(__FILE__, __LINE__, #cond); } } while (0)
```

由 `-DOPENWCH_ASSERT_DEBUG` 控制是否启用；失败时（可选）打印并通过
`qingke_system_reset()` 复位。与 libopencm3 的 `cm3_assert` 语义一致。

### 5.4 一期必须交付的 API 面（`ch32v003`）

| 外设 | 函数 |
|---|---|
| `gpio` | `set_mode` `set` `clear` `toggle` `get` `port_set` `port_clear` `port_get` `lock_config` `set_eventout` `primary_remap` `secondary_remap` |
| `rcc` | `clock_setup_hsi` `clock_setup_hse` `osc_on` `osc_off` `wait_for_osc` `pll_on` `pll_off` `periph_clock_enable` `periph_clock_disable` `get_clocks_freq` `ahb_set_prescaler` `apb1_set_prescaler` `apb2_set_prescaler` `set_sysclk_source` |
| `usart` | `set_baudrate` `set_databits` `set_stopbits` `set_parity` `set_mode` `set_flow_control` `enable` `disable` `send` `recv` `enable_rx_dma` … |
| `spi` | `init_master` `init_slave` `enable` `disable` `send` `recv` `set_baudrate_prescaler` `set_clock_polarity` `set_clock_phase` `set_dff` `set_bit_order` `enable_software_slave_management` `set_nss` |
| `i2c` | `init_master` `init_slave` `enable` `disable` `send_start` `send_stop` `send_data` `read_data` `get_flag` `clear_flag` `set_own_address` `set_clock_frequency` |
| `tim` | `set_mode` `enable` `disable` `set_prescaler` `set_period` `set_counter` `get_counter` `set_oc_value` `set_oc_polarity` `set_oc_mode` `enable_oc_output` `set_input_filter` `set_input_prescaler` `enable_preload` `set_alignment` `set_direction` `enable_irq` |
| `adc` | `enable` `disable` `set_channel` `set_sample_time` `start_conversion` `get_eoc` `read_regular` `enable_continuous` `enable_scan` `enable_dma` `set_external_trigger` |
| `dma` | `channel_set_config` `channel_set_priority` `channel_set_memory_address` `channel_set_peripheral_address` `channel_set_number_data` `channel_enable` `channel_disable` `channel_enable_interrupt` `channel_clear_interrupt_flag` `enable_mem2mem` |
| `exti` | `select_source` `set_trigger` `enable_irq` `disable_irq` `get_flag` `clear_flag` |
| `syscfg`(AFIO) | `gpio_remap` 相关、`exti_select_source` |
| `pwr` | `set_voltage_scaling` `enable_low_power` … |
| `flash` | `unlock` `lock` `wait_for_last_operation` `erase_page` `program_word` `program_halfword` `get_status` `clear_status` |
| `iwdg`/`wwdg` | `set_prescaler` `set_reload` `start`/`enable` `get_flag` `clear_flag` |
| `opa`(运放) | `enable` `disable` `set_mode` … |
| `systick` | `set_reload` `set_clock_source` `enable_counter` `disable_counter` `enable_interrupt` `get_value` |
| `qingke` | `irq_enable` `irq_disable` `irq_save` `irq_restore` `system_reset` `delay_us` `delay_ms` |

`ch582` 额外/差异：`rwa`（安全访问解锁）、`clk`（`SetSysClock` 等价物）、
位域式 GPIO、`pwm`（多通道 PWMX）、`uart`（4 实例）、`tim`（timer0-3 分别独立寄存器）、
`sys`（`SYS_ResetExecute`、`SYS_GetSysTickCnt`）、`sys` 中断保存/恢复。

---

## 6. 构建系统设计

### 6.1 与 libopencm3 保持一致的部分（原样保留）

- 根 `Makefile` 的 `TARGETS`、`LIB_DIRS = $(wildcard $(addprefix lib/,$(TARGETS)))`、
  `$(LIB_DIRS): $(IRQ_GENERATED_FILES)` 的依赖、`.stamp_failure_*` 失败聚合、
  `make V=1` 的静默/详细模式。
- `lib/Makefile.include` 的单库归档规则。
- `mk/gcc-rules.mk` 的 `%.elf` / `%.bin` / `%.hex` / `%.o` 规则。
- `mk/genlink-config.mk` / `mk/genlink-rules.mk` 的职责划分。
- `ld/devices.data` + `scripts/genlink.py` 的 `FAMILY`/`SUBFAMILY`/`DEFS` 查询接口。
- 应用侧用法（与 libopencm3 逐字对齐）：

```make
OPENWCH_DIR ?= /path/to/libopenwch
DEVICE      ?= ch32v003f4p6
include $(OPENWCH_DIR)/mk/genlink-config.mk
include $(OPENWCH_DIR)/mk/gcc-rules.mk
include $(OPENWCH_DIR)/mk/genlink-rules.mk

CFLAGS  += -Os -g -Wall -Wextra -std=c99
OBJS    += main.o
LDSCRIPT = generated.$(DEVICE).ld

all: $(OBJS) main.elf main.bin main.hex
```

### 6.2 必须改造的部分（ch32fun 式改动）

#### (a) 工具链前缀自动探测 —— `mk/gcc-config.mk`

```make
PREFIX ?= $(shell \
    for p in riscv64-unknown-elf riscv64-none-elf riscv32-unknown-elf \
             riscv-none-elf riscv64-elf riscv32-elf; do \
        command -v $$p-gcc >/dev/null 2>&1 && { echo $$p; exit 0; }; \
    done; echo riscv64-unknown-elf)
```

- 保留 `PREFIX` 可覆盖（`make PREFIX=/opt/toolchain/bin/riscv64-unknown-elf-`）。
- Windows/NetBSD 分支参考 ch32fun 的 `where`/pkgsrc 处理。
- 默认值写死为 `riscv64-unknown-elf`（需求指定的目标工具链）。

#### (b) 工具链能力探测 + `zmmul` 降级

```make
GCC_MAJOR  := $(shell $(CC) -dumpversion | cut -d. -f1)
GCC_GE_13  := $(shell [ $(GCC_MAJOR) -ge 13 ] && echo 1 || echo 0)
ZMMUL_OK   := ...   # 探测 -march=rv32ec_zmmul 是否被接受
```

`ch32v002/004/005/006/007` 需要 `zmmul`（乘法扩展）。若工具链 < GCC 13 或不接受
`_zmmul`，则退化为 `rv32ec`（软乘）——与 ch32fun 行为一致。

#### (c) 设备树新增 RISC-V 字段 —— `ld/devices.data`

libopencm3 的字段只有 `ROM/RAM/ROM_OFF/RAM_OFF/CPU/FPU`。新增：

| 字段 | 含义 | 例 |
|---|---|---|
| `MARCH` | `-march` 值 | `rv32ec` |
| `MABI` | `-mabi` 值 | `ilp32e` |
| `ZMMUL` | 是否需要 `_zmmul` | `1` |
| `ENTRY` | 入口符号（生成到 ld 的 `ENTRY()`） | `_start` |
| `GP` | 是否需要 `__global_pointer$` | `1` |
| `HICODE` | 是否需要 `.highcode` 段 | `1` |

`devices.data` 片段示例：

```
ch32v003?*   ch32v003  ROM=16K RAM=2K ZMMUL=0
ch32v002?*   ch32v002  ROM=16K RAM=4K ZMMUL=1
ch32v00x     ch32v     MARCH=rv32ec MABI=ilp32e ROM_OFF=0x00000000 RAM_OFF=0x20000000 ENTRY=_start GP=1 HICODE=1
ch32v003     ch32v     MARCH=rv32ec MABI=ilp32e
ch32v       END       ARCH=riscv

ch582?*      ch58x     ROM=448K RAM=32K
ch58x        ch5xx     MARCH=rv32imac MABI=ilp32 ROM_OFF=0x00000000 RAM_OFF=0x20000000 ENTRY=_start GP=1 HICODE=1
ch5xx        END       ARCH=riscv
```

> 具体正则与 `RAM_OFF`（`ch57x` 部分型号为 `0x20003800`）在实现阶段用真实封装名验证。

#### (d) `genlink.py` / `genlink-config.mk` 的 RISC-V 化

```make
genlink_march := $(shell $(OPENWCH_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) MARCH)
genlink_mabi  := $(shell $(OPENWCH_DIR)/scripts/genlink.py $(DEVICES_DATA) $(DEVICE) MABI)
ARCH_FLAGS    := -march=$(genlink_march) -mabi=$(genlink_mabi)
```

- 用 `MARCH/MABI` 取代 ARM 的 `-mcpu=` + `-mthumb` + `-mfpu=`。
- `genlink.py` 新增 `MARCH`/`MABI`/`ARCH`/`ZMMUL` 等 key 的查询分支（`ARCH` 加入 `info`，
  不加入 `defs`，与 `CPU`/`FPU` 同处理）。
- `genlink-rules.mk` 中 `$(CPP) $(ARCH_FLAGS) ... -P -E $< -o $@` 保持不变
  （RISC-V 的 `-march`/`-mabi` 对预处理无害）。

#### (e) 通用链接脚本 —— `ld/linker.ld.S`

以 libopencm3 的 `ld/linker.ld.S` 为骨架，改造为 RISC-V：

```ld
ENTRY(_start)                       /* ← 可被 _ENTRY 宏覆盖 */

MEMORY {
    ram (rwx) : ORIGIN = _RAM_OFF, LENGTH = _RAM
#if defined(_ROM)
    rom (rx)  : ORIGIN = _ROM_OFF, LENGTH = _ROM
#endif
    ...
}

SECTIONS {
    .init : { KEEP(*(SORT_NONE(.init))) . = ALIGN(4); } >rom   /* WCH 向量表 */
    .text : { *(.text*) *(.rodata*) . = ALIGN(4); } >rom
    .preinit_array / .init_array / .fini_array ... >rom

#if defined(_HICODE)
    .highcode : { *( .highcode* ) } >ram AT >rom
    _highcode_lma = LOADADDR(.highcode);
    _highcode_vma_start = ADDR(.highcode);
    _highcode_vma_end   = ADDR(.highcode) + SIZEOF(.highcode);
#endif

    .data : { _data = .; *(.data*) *(.ramtext*) ... _edata = .; } >ram AT >rom
    _data_loadaddr = LOADADDR(.data);
    .bss  : { _sbss = .; *(.bss*) *(COMMON) _ebss = .; } >ram

#if defined(_GP)
    PROVIDE(__global_pointer$ = MIN(_sdata + 0x800, MAX(_data + 0x800, _ebss - 0x800)));
#endif
    PROVIDE(_eusrstack = ORIGIN(ram) + LENGTH(ram));
    end = .; PROVIDE(_end = .);
}
```

- 保留 libopencm3 的 `_ROM`/`_RAM`/`_ROM_OFF`/`_RAM_OFF` 命名与 `_data`/`_edata`/`_ebss`/
  `_data_loadaddr`/`_stack` 符号，保证 genlink 机制不变。
- 删除 `.ARM.extab`/`.ARM.exidx`，新增 `.init`、`.highcode`、`__global_pointer$`、`_eusrstack`、
  `_susrstack`。

#### (f) 中断头文件生成器 —— `scripts/irq2nvic_h`

libopencm3 版生成三个文件：

| 输出 | libopencm3 | libopenwch |
|---|---|---|
| `include/libopenwch/<target>/nvic.h` | IRQ 号枚举 + ISR 原型 | 保留（RISC-V 无 NVIC_IRQn 枚举，改为 `#define <PERIPH>_IRQ <n>` + 原型） |
| `lib/<target>/vector_nvic.c` | `IRQ_HANDLERS` 宏（`{ .irq = { ALIAS } }`） | 改为 `OPENWCH_IRQ_HANDLERS` 宏（裸 `.word` 列表），供 `vector.c` 的 `irq[]` 使用 |
| `include/libopencmsis/<target>/irqhandlers.h` | CMSIS `IRQ_HANDLERS` | 保留（后期 CMSIS 兼容层用） |

生成规则（根 `Makefile`）：

```make
IRQ_DEFN_FILES   := $(foreach TARGET,$(TARGETS),$(wildcard include/libopenwch/$(TARGET)/irq.json))
NVIC_H           := $(IRQ_DEFN_FILES:%/irq.json=%/nvic.h)
VECTOR_NVIC_C    := $(IRQ_DEFN_FILES:include/libopenwch/%/irq.json=lib/%/vector_nvic.c)
IRQHANDLERS_H    := $(IRQ_DEFN_FILES:include/libopenwch/%/irq.json=include/libopencmsis/%/irqhandlers.h)
IRQ_GENERATED_FILES = $(NVIC_H) $(VECTOR_NVIC_C) $(IRQHANDLERS_H)

include/libopenwch/%/nvic.h lib/%/vector_nvic.c include/libopencmsis/%/irqhandlers.h: include/libopenwch/%/irq.json ./scripts/irq2nvic_h
	@printf "  GENHDR  $*\n";
	$(Q)./scripts/irq2nvic_h ./$<;
```

> 注意：libopencm3 的 `irq2nvic_h` 里 `../` 偏移是针对非嵌套路径写死的，
> 移植时必须改成参数化/按 `%` 路径推导，否则 `lib/ch32v/003/vector_nvic.c` 路径会错。

#### (g) 嵌套目标目录的处理

根 `Makefile` 的 `LIB_DIRS` 逻辑**无需改动**（`$(addprefix lib/,$(TARGETS))` 天然支持
`lib/ch32v/003`）。需要额外处理：

- `.stamp_failure_$(subst /,_,$@)` 已正确处理斜杠。
- `lib/<family>/Makefile.include` 为族级公共 `TGT_CFLAGS` + `OBJS` 提供 include 点。
- 子目录 `lib/ch32v/003/Makefile` 设 `LIBNAME = libopenwch_ch32v003`，归档产物写到
  `$(SRCLIBDIR)/$(LIBNAME).a`，即 `lib/libopenwch_ch32v003.a`。
- `SRCLIBDIR ?= ../../..`（三层深度）——与 libopencm3 的 `../..`（两层）不同，需注意。

### 6.3 构建流程（一期）

```sh
# 1) 交叉工具链（Debian/Kali 系）
sudo apt-get install -y gcc-riscv64-unknown-elf

# 2) 构建库
cd libopenwch
make TARGETS="ch32v/003 ch5xx/58x" PREFIX=riscv64-unknown-elf-
# 或直接：
make                     # 使用根 Makefile 中的默认 TARGETS

# 3) 产物
ls lib/libopenwch_ch32v003.a lib/libopenwch_ch582.a
make genlinktests        # 链接脚本生成冒烟测试

# 4) 应用侧示例
cd examples/ch32v003/blink
make OPENWCH_DIR=../../.. DEVICE=ch32v003f4p6
```

### 6.4 工具链兼容性风险与对策

| 风险 | 对策 |
|---|---|
| 系统未安装 RISC-V 工具链 | `mk/gcc-config.mk` 探测失败时给出**明确错误信息**并提示安装命令；不静默回退到 `arm-none-eabi` |
| GCC < 13 不支持 `-march=rv32ec_zmmul` | 探测 + 降级为 `rv32ec`（软乘），与 ch32fun 一致 |
| GCC 13+ 的 `rv32ec` 默认行为变化 | 显式传 `-march`/`-mabi`，不依赖默认 |
| 缺少 newlib（`--without-newlib` 的裸工具链） | 库本身**不依赖 newlib**（只用 `stdint.h`/`stdbool.h`/`stddef.h`，freestanding 头）；示例工程用 `-nostdlib` + 自带 mini-libc（借鉴 ch32fun.c，放在 `lib/common/mini-libc.c`，可选） |
| `riscv64-linux-gnu-gcc` 被误用 | **不**自动回退到它（与 ch32fun 的明确决策一致），只接受 `*-unknown-elf` / `*-none-elf` / `*-elf` 系列 |
| 多工具链矩阵（unknown-elf / none-elf / xpack） | `.github/workflows/ci.yml` 中做矩阵构建 |

---

## 7. 一期交付范围（MVP）

**必须完成**

1. 仓库骨架 + `mk/` + `scripts/` + `ld/`（可构建、可生成链接脚本、可递归建库）。
2. `qingke/` 核心层：`common.h`、`csr.h`、`pfic.h`、`nvic.h`、`vector.h`、`assert.h`、
   `sync.h` + `vector.c`（启动/向量表/分发）、`nvic.c`、`assert.c`、`systick.c`。
3. `lib/ch32v/003/` + `include/libopenwch/ch32v003/`：
   `gpio`、`rcc`、`usart`、`tim`、`spi`、`i2c`、`adc`、`dma`、`exti`、`syscfg`、`flash`、
   `iwdg`、`wwdg`、`pwr`、`opa`、`systick`。
4. `lib/ch5xx/58x/` + `include/libopenwch/ch582/`：
   `rwa`、`clk`、`gpio`、`uart`、`spi`、`i2c`、`tim`、`pwm`、`adc`、`flash`、`pwr`、`sys`。
5. `examples/ch32v003/blink`、`examples/ch32v003/usart_echo`、
   `examples/ch582/blink`、`examples/ch582/uart_echo` —— 全部能 `make` 出 `.elf`/`.bin`/`.hex`。
6. `make genlinktests` 通过（覆盖 `ch32v003f4p6`、`ch582m` 等器件名）。
7. Doxygen 可生成（`make -C doc html` 至少对 ch32v003 成功）。
8. `project.md` / `phase.md` / `status.md` / `AGENTS.md` 四份文档，且 `status.md` 与实际进度同步。

**一期不做**

- `libopencmsis/`（只放生成器输出占位）
- `usb/`、`ethernet/`、BLE 协议栈
- `ch32v103/203/303`、`ch571/573`、`ch591/592` 的实现（仅目录占位 + `devices.data` 条目）
- CI 全矩阵（先本地脚本）

---

## 8. 许可与合规

- **库代码**（`include/`、`lib/`、`ld/`、`mk/`）：`LGPL-3.0-or-later`
  （`LICENSE` / `COPYING.LGPL3`），与 libopencm3 一致——允许商业闭源链接。
- **`scripts/checkpatch.pl`**：`GPL-2.0`（`COPYING.GPL2`），取自 Linux 内核，
  文件头即声明 GPL-2.0，无法改许可。它是开发工具，不属于库。
- **`lib/ble/wch/`**：**Apache-2.0**（`lib/ble/wch/LICENSE`），是 WCH 的闭源
  BLE 栈与其头文件原样 vendor 进来的，**不属于 LGPL**；再分发者必须随附其许可与
  `NOTICE` 中的说明。
- **文档与治理文件**（`project.md`、`phase.md`、`status.md`、`AGENTS.md`、
  `README.md`、`NOTICE`）：CC0-1.0 / 公有领域。
- WCH EVT 代码（`ch32v003-main`、`ch583-main`）**只作为寄存器定义与操作时序的事实参考**，
  **不复制其代码**，避免引入 WCH 的许可不确定性。寄存器地址/位定义属于接口事实，
  按 HACKING 的要求"尽量贴近厂商手册命名"重新表述为 libopencm3 风格宏。
- ch32fun 采用 MIT/自定义宽松许可，其 `misc/libgcc.a` 等可再分发内容若被引入，
  需在 `NOTICE`/`README` 中标注来源与许可（本库**不使用**该 `libgcc.a`，
  改用自建 mini-libc 并探测工具链自带的 `rv32e/ilp32e` libgcc）。
- **伴随工具（WCH-LinkE 编程器，§1.3）**：独立仓库 `libopenwch-tools`，自带
  `LICENSE` 与 `NOTICE`。它只由 `libopenwch-template` 通过 submodule 引用，
  本仓库不分发其代码，它的许可也不改变本仓库的许可。
  工具实现走 **clean-room**：minichlink/wlink/openocd-wch 只作为**协议事实**参考，
  不复制其源码——与对待 WCH EVT 的做法一致。
- **示例（`libopenwch-examples`）与模板（`libopenwch-template`）**：同样独立仓库、
  LGPL-3.0-or-later、各自带 `LICENSE`/`NOTICE`。两者都以 submodule 引用本库，
  因此本库的许可与它们的许可互不影响。

---

## 9. 关键设计决策记录（ADR 摘要）

| # | 决策 | 理由 |
|---|---|---|
| D1 | 目标目录用嵌套路径 `ch32v/003`、`ch5xx/58x` | 与 WCH 族体系对齐；根 Makefile 的 `addprefix` 天然支持 |
| D2 | 按族拆静态库（`libopenwch_ch32v003.a`、`libopenwch_ch582.a`） | `rv32ec` 与 `rv32imac` 的 `.o` 不能混装在一个归档里；与 libopencm3 分库一致 |
| D3 | 保留 `nvic_*` API 名，内部映射到 PFIC | PFIC 寄存器布局与 NVIC 同构；libopencm3 用户零迁移成本 |
| D4 | 向量表用 `vector_table_t` 裸字数组结构，保留 `irq[]` 成员名 | 与 libopencm3 的 `vector.h` 心智一致，但反映 RISC-V 事实 |
| D5 | `irq2nvic_h` 生成物文件名不变（`nvic.h`/`vector_nvic.c`） | 降低与 libopencm3 的认知差异；文档注明"PFIC/NVIC" |
| D6 | `devices.data` 增加 `MARCH/MABI/ZMMUL/ENTRY/GP/HICODE` 字段 | 用同一套 genlink 机制表达 RISC-V 事实 |
| D7 | 库不依赖 newlib；示例可选自带 mini-libc | 兼容 `--without-newlib` 的裸工具链；ch32fun 的 `-nostdlib` 路线 |
| D8 | 不复制 WCH EVT 代码，只重写 | 许可洁净；API 风格必须是非驼峰，EVT 代码无法直接复用 |
| D9 | `ch32v0/common/` 的 `v1` 变体同时服务 002/003/004/005/006/007 | 复刻 libopencm3 的 `common/` 变体共享机制，避免七份重复代码 |
| D10 | `SystemInit()` 作为弱符号，默认实现调用 `rcc_clock_setup_hsi()` | 保持 libopencm3 的"用户在 main 之前/之中显式配时钟"哲学，同时兼容 WCH 启动流程 |

---

## 10. 后续扩展路线

1. **第二梯队芯片**：`ch32x035`（X 系列，RV32IMAC，无 PLL）、`ch32l103`、`ch32v103`。
2. **V20x/V30x**：引入 `-march=rv32imafc -mabi=ilp32f` 的 FPU 变体、`ch32v307` 的
   `TARGET_MCU_MEMORY_SPLIT` 式 RAM/Flash 划分。
3. **CH5xx 全族**：`ch571/573`（RAM 偏移 `0x20003800`）、`ch591/592`（`TARGET_MCU_LD==9`）。
4. **USB 设备控制器**：移植 libopencm3 的 `lib/usb/`，把 `usb_dwc_common`/`st_usbfs_*`
   抽象出 RISC-V 可用的 part（WCH 的 USB2.0 设备控制器与 `st_usbfs_v2` 相似度较高）。
5. **BLE**：✅ 已完成外设角色的薄封装层（TMOS / GAP / GAPRole / GATT server），
   链接 WCH 的闭源 `LIBCH58xBLE.a`；central、observer、broadcaster、配对、OTA、mesh
   仍待做。详见 `lib/ble/README` 与 §1.2。
6. **`libopencmsis/`**：让 WCH EVT 的 `StdPeriphDriver` 可编译在 libopenwch 之上
   （`NVIC_EnableIRQ` 等 CMSIS 名 → libopenwch 实现），作为迁移桥梁。
7. **文档站**：Doxygen + GitHub Pages，按族生成。
8. **示例与模板**：✅ 已拆为独立仓库——`libopenwch-examples`（逐个外设的完整示例，
   同时是本库 CI 的集成测试对象）与 `libopenwch-template`（应用骨架，本库与工具均以
   submodule 引入）。后续随新族/新外设扩充示例即可。
9. **伴随工具 `libopenwch-tools`（WCH-LinkE 编程器，§1.3）**：独立仓库，由 template
   以 submodule 引入。定位为**烧录工具**，分四个里程碑：
   1. 仓库骨架、host 构建（libusb）、USB 设备发现、`info` 子命令、芯片表、CLI；
   2. 停机/复位、调试寄存器读写、内存读回（`read`）；
   3. Flash 擦/写/校验、`reset`/`unbrick`（`flash`）；
   4. 单线调试终端。
   调试器（GDB stub）**不在**首版范围。该工具是清除 §11 中「硬件在环」阻塞的前提，
   但它自身也需要 WCH-LinkE 才能验证。

---

## 11. 验证策略

| 层次 | 手段 | 一期是否可执行 |
|---|---|---|
| 构建正确性 | `make TARGETS=... ` 全绿，`.a` 产物存在 | ✅ 需要 RISC-V 工具链 |
| 链接脚本生成 | `make genlinktests`（`ld/tests/*.data`） | ✅ |
| 归档内容 | `riscv64-unknown-elf-nm` 检查符号存在且无未定义 | ✅ |
| 代码规范 | `make stylecheck`（`scripts/checkpatch.pl`） | ✅ |
| 目标码检查 | `riscv64-unknown-elf-objdump -d` 确认 `-march` 生效（压缩指令/无浮点） | ✅ |
| 应用侧集成 | `examples` CI job：克隆 `libopenwch-examples`，用当前 checkout 构建全部示例 | ✅ 需 RISC-V 工具链 |
| 硬件在环 | WCH-Link + 编程器工具（`minichlink` 或 §1.3 的 `libopenwch-tools`）闪写 CH32V003/CH582 并跑 blink/uart | ⚠️ 需硬件 |
| 编程器工具（§1.3） | 在**独立仓库**中自测：host 构建 + 无设备时的诊断路径 | ⚠️ 构建可验，运行需 WCH-LinkE |
| QEMU | `qemu-riscv32` 可跑纯计算部分（无外设）；对寄存器级驱动意义有限 | ⚠️ 有限 |
