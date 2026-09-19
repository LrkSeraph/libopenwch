# libopenwch — 阶段性目标（phase.md）

> 总体设计见 `project.md`，当前完成情况见 `status.md`，入口索引见 `AGENTS.md`。
>
> 阶段划分原则：**每个阶段结束时仓库必须处于可构建、可验证的状态**。
> 阶段内任务用复选框跟踪；完成一个立即勾选，并同步更新 `status.md`。

---

## 阶段总览

| 阶段 | 名称 | 交付物 | 验收标准 | 依赖 |
|---|---|---|---|---|
| **P0** | 仓库骨架与构建系统 | `Makefile`、`mk/`、`scripts/`、`ld/`、`lib/Makefile.include` | `make genlinktests` 通过；空 `TARGETS` 下 `make` 成功；工具链探测正确报错/报版本 | RISC-V 工具链 |
| **P1** | `qingke/` 核心层 | `include/libopenwch/qingke/*`、`lib/qingke/*` | 能编译出 `.a`；向量表结构与 WCH 一致；`nvic_*` 可操作 PFIC | P0 |
| **P2** | CH32V003 外设驱动 | `include/libopenwch/ch32v003/*`、`lib/ch32v/003/*` | `make TARGETS=ch32v/003` 全绿；示例 `blink`/`usart_echo` 链接成功 | P1 |
| **P3** | CH582/CH58x 外设驱动 | `include/libopenwch/ch582/*`、`lib/ch5xx/58x/*` | `make TARGETS=ch5xx/58x` 全绿；示例链接成功 | P1 |
| **P4** | 示例、文档、CI、硬件验证 | `examples/`、`doc/`、`.github/workflows/ci.yml`、`NOTICE` | Doxygen 生成成功；CI 绿；硬件上跑通 blink + uart | P2, P3 |
| **P5** | 扩展族与 USB/BLE 接口 | `ch32x035`、`ch32v103`、`ch57x/59x`、`usb/` | 新增族构建全绿；`devices.data` 覆盖全部规划器件 | P4 |

---

## P0 — 仓库骨架与构建系统（libopencm3 对齐 + ch32fun 工具链适配）

**目标**：建立与 libopencm3 同构的构建骨架，并用 ch32fun 的方式让它认识
`riscv64-unknown-elf` 工具链。此阶段**不含任何外设驱动**。

### P0.1 仓库骨架

- [x] 创建目录：`mk/ scripts/ ld/tests/ include/libopenwch/ include/libopencmsis/ lib/ doc/ tests/ examples/`
- [x] `COPYING.LGPL3`、`COPYING.GPL3`（从 libopencm3 原样取用，保留版权头）
- [x] `HACKING`（继承 libopencm3，补充 RISC-V 章节）
- [x] `HACKING_COMMON_DOC`（改写成"新增 WCH 族"清单）
- [x] `.gitignore`（`.a`、`.o`、`.d`、`*.elf`、`*.bin`、`*.hex`、`generated.*.ld`、`.stamp_failure_*`、`doc/html/`）
- [x] `.clang-format`（参考 `ch32fun-master/.clang-format`，内核风格，4 空格 Tab→8）
- [x] `README.md`（项目简介 + 快速上手 + 一期范围）
- [ ] `git init` 并首次提交

### P0.2 构建系统核心（`mk/`）

- [x] `mk/README` —— 用法说明，与 libopencm3 的 `mk/README` 逐条对齐
- [x] `mk/gcc-config.mk`
  - [x] RISC-V 前缀探测链：`riscv64-unknown-elf` → `riscv64-none-elf` → `riscv32-unknown-elf` → `riscv-none-elf` → `riscv64-elf` → `riscv32-elf`
  - [x] `PREFIX` 可覆盖；默认 `riscv64-unknown-elf`
  - [x] Windows(`where`)/NetBSD(pkgsrc) 分支
  - [x] `CC/CXX/LD/AR/AS/OBJCOPY/OBJDUMP/GDB/SIZE/CPP` 全部用 `$(PREFIX)` 前缀
  - [x] `GCC_MAJOR` 探测 + `GCC_GE_13`
  - [x] `ZMMUL_OK` 探测（试编译 `-march=rv32ec_zmmul`）
  - [x] 工具链缺失时 `$(error ...)` 给出安装提示（**不**回退到 `arm-none-eabi`）
- [x] `mk/gcc-rules.mk` —— 基本沿用 libopencm3 的 `%.elf/%.bin/%.hex/%.srec/%.list/%.o`
- [x] `mk/genlink-config.mk`
  - [x] `MARCH/MABI` → `ARCH_FLAGS := -march=... -mabi=...`
  - [x] 移除 ARM 的 `-mcpu/-mthumb/-mfpu/-mfloat-abi` 分支
  - [x] `ZMMUL=1` 且 `ZMMUL_OK=1` 时追加 `_zmmul`
  - [x] 库自动选择：`libopenwch_$(genlink_family).a` → 回退 `libopenwch_$(genlink_subfamily).a`
  - [x] `CPPFLAGS`/`LDFLAGS` 的 `-I$(OPENWCH_DIR)/include`、`-L$(OPENWCH_DIR)/lib`
  - [ ] 兼容 `OPENCM3_DIR` 变量名（`OPENWCH_DIR ?= $(OPENCM3_DIR)`，降低迁移摩擦）
- [x] `mk/genlink-rules.mk` —— `generated.$(DEVICE).ld` 预处理规则（含 `_ENTRY/_GP/_HICODE` 定义）
- [x] `lib/Makefile.include` —— 单库归档规则；公共 `OBJS`（核心层）可配置

### P0.3 脚本（`scripts/`）

- [x] `scripts/genlink.py` 移植
  - [x] 新增 `MARCH`/`MABI` 查询（走 `info`，不进 `defs`）
  - [x] 新增 `ARCH`、`ZMMUL`、`ENTRY`、`GP`、`HICODE` 支持
  - [x] 保持 `FAMILY`/`SUBFAMILY`/`DEFS`/`CPPFLAGS` 接口不变
  - [x] 单测：对 `ch32v003f4p6`、`ch582m`、`ch583m` 能解析出正确族与内存参数
- [x] `scripts/irq2nvic_h` 移植（RISC-V 化）
  - [x] 生成 `include/libopenwch/<target>/nvic.h`：`#define <PERIPH>_IRQ <n>` + ISR 原型 + `OPENWCH_IRQ_COUNT`
  - [x] 生成 `lib/<target>/vector_nvic.c`：`OPENWCH_IRQ_HANDLERS` 裸 `.word` 列表宏
  - [x] 生成 `include/libopencmsis/<target>/irqhandlers.h`
  - [x] **修正路径推导**：支持嵌套 `lib/ch32v/003/vector_nvic.c`
  - [x] `--remove` 支持（对应根 Makefile 的 `%.cleanhdr`）
- [x] `scripts/genlinktest.sh` 移植（用 `riscv64-unknown-elf-gcc` 做链接冒烟）
- [ ] `scripts/gendoxylist`、`scripts/gendoxylayout.py` 移植
- [x] `scripts/checkpatch.pl` 从 libopencm3 取用

### P0.4 链接脚本（`ld/`）

- [x] `ld/devices.data`：新增 `MARCH/MABI/ZMMUL/ENTRY/GP/HICODE` 字段与文档头
  - [x] `ch32v003`（ROM=16K RAM=2K，`rv32ec/ilp32e`）
  - [x] `ch32v002/004/005/006/007`（`ZMMUL=1`）
  - [x] `ch582/583/584/585`（`rv32imac/ilp32`）
  - [x] 其余族条目占位（`ch32v103/203/303`、`ch32x035`、`ch32l103`、`ch571/573`、`ch591/592`）
  - [x] 真实封装名正则（`ch32v003f4p6`、`ch32v003j4m6`、`ch582m`、`ch583m` …）
- [x] `ld/linker.ld.S`：RISC-V 版通用模板
  - [x] `ENTRY(_start)`（可由 `_ENTRY` 覆盖）
  - [x] `ram`/`rom` 默认区 + 可选 `ram1..ram5`/`ccm`/`eep` 保留
  - [x] `.init` 段（WCH 向量表，`KEEP(SORT_NONE(.init))`）
  - [x] `.highcode` 段（`_HICODE` 时启用，VMA in RAM / LMA in ROM，导出 `_highcode_lma/_vma_start/_vma_end`）
  - [x] `__global_pointer$`（`_GP` 时启用）
  - [x] `_eusrstack` / `_susrstack`（WCH 硬件中断堆栈用）
  - [x] 保留 `_data/_edata/_ebss/_data_loadaddr/_stack/end`
  - [x] 删除 `.ARM.extab/.ARM.exidx`
- [x] `ld/README`、`ld/tests/*.data`（至少 `ch32v003f4p6`、`ch582m` 两组）

### P0.5 根 `Makefile`

- [x] `TARGETS ?= ch32v/003 ch5xx/58x`（一期默认）
- [x] `IRQ_DEFN_FILES` / `NVIC_H` / `VECTOR_NVIC_C` / `IRQHANDLERS_H` 的嵌套路径推导
- [x] `LIB_DIRS := $(wildcard $(addprefix lib/,$(TARGETS)))` + `$(LIB_DIRS): $(IRQ_GENERATED_FILES)`
- [x] `.stamp_failure_*` 失败聚合
- [x] `all/build/lib/clean/%.cleanhdr/stylecheck/styleclean/genlinktests/genlinktests.clean/list-targets/html/doc`
- [x] `make list-targets` 输出一期目标

### P0 实现记录（与初版规划的偏差）

实现过程中发现并修正了若干设计问题，已回写到 `project.md`，此处记录偏差：

1. **目标名 = 库目录名 = genlink FAMILY**。初版计划用嵌套 `ch32v/003`，实现时确定
   采用 **`ch32v0`、`ch5xx58x`、`ch5xx57x`、`ch5xx59x`、`ch32v0v4`、`ch32x0`、`ch32l1`**
   这样的单层名字。理由：
   - `genlink.py` 的 `FAMILY` 直接决定 `-lopenwch_<family>`，它必须是一个合法的库名；
   - 族名同时充当 `lib/<family>/` 目录名后，`TARGETS`、归档名、include 路径三者一致，
     不需要任何映射表；
   - libopencm3 的 `stm32/f1` 之所以能嵌套，是因为它的 `FAMILY` 是 `stm32f1`，
     而目录是 `stm32/f1`——同一份信息写两遍。单层名消除了这种重复。
   CH32V00x 各型号共享一个归档（它们 ISA 相同），符合"一库一族"的约束。

2. **`devices.data` 的模式必须"不能吞掉自己的组名"**。这是实现中踩到的坑：
   - 器件行必须写 `ch32v003?*`（`?*` 而非 `*`），否则 `ch32v003*` 会先匹配到组名
     `ch32v003`，遍历提前结束；
   - 组行必须写**精确**的 `ch32v0`，否则 `ch32v0*` 会把器件名当组名匹配。
   - `+` 续行必须排在 `END` 行**之前**，因为遍历一旦处理到 `END` 行就停止。

3. **向量表拆成两个输入段**。`.vector_exc`（slot 0..15，来自 `vector_chipset.S`）
   与 `.vector`（slot 16..，来自生成的 `vector_handlers.c`），由链接脚本显式拼接。
   仅靠同名段跨目标文件排序不可靠。链接脚本用 `ASSERT` 强制
   `vector_table - _vector_base == 64`，这个断言在开发中确实抓到过一次布局错误。

4. **`_reset_entry` 放在 C 里**。汇编只负责建立 `gp`/`sp` 并 `call _reset_entry`；
   其余初始化（`.data`/`.bss`/`mtvec`/INTSYSCR/构造函数）都是 C，可读性更好。

5. **必须显式声明 `zicsr`/`zifencei`**。RISC-V 20191213 规范起这两个扩展独立，
   GCC ≥ 12 不再隐含；`csrrw`/`mret`/`fence.i` 都会汇编失败。已在 `devices.data`
   中用 `MARCH=rv32ec ZEXT=_zicsr_zifencei`（以及 `rv32imac` 同理）表达。

6. **`mk/` 全程使用 `$(abspath ...)` 绝对路径**。相对 `../..` 在虚拟化/沙箱文件系统下
   不可靠（实测 `cd ..` 与 `realpath ..` 对该工作区返回错误结果）。这既影响编译
   （`VPATH`、`-I`），也影响归档输出路径（`SRCLIBDIR`）。

7. **`blocking_handler()` 定义在生成文件里**。GCC 要求 `alias` 目标与别名在**同一
   编译单元内定义**（仅声明不够），所以它由 `irq2nvic_h` 生成到
   `vector_handlers.c` 中，而不是放在 `lib/qingke/vector.c`。

8. **`OPENWCH_INTERRUPT` 默认使用可移植的 `__attribute__((interrupt()))`**。
   实测本机 `riscv64-unknown-elf-gcc 15.3.0` **拒绝** `"WCH-Interrupt-fast"`
   （会退化成普通 `ret`，即根本不是中断处理函数）。WCH 的快速中断属性是
   opt-in：定义 `OPENWCH_USE_WCH_INTERRUPT` 并使用 WCH 工具链时才启用。

### P2 实现记录（CH32V00x）

9. **CH32V00x 的 GPIO 引脚配置 nibble 不是 (MODE, CNF) 位域打包**。
   这是本阶段最重要的发现。WCH 手册与 EVT 给出的
   `GPIO_Mode_*` 值是：

   | 名称 | 值 |
   |---|---|
   | `GPIO_Mode_AIN` | `0x00` |
   | `GPIO_Mode_IN_FLOATING` | `0x04` |
   | `GPIO_Mode_IPD` | `0x28` |
   | `GPIO_Mode_IPU` | `0x48` |
   | `GPIO_Mode_Out_OD` | `0x14` |
   | `GPIO_Mode_Out_PP` | `0x10` |
   | `GPIO_Mode_AF_OD` | `0x1c` |
   | `GPIO_Mode_AF_PP` | `0x18` |

   穷举所有可能的两个 2-bit 字段位置（MODE 在 s_m、CNF 在 s_c）**不存在**
   能复现上表的解。也就是说手册正文 "MODE[1:0] / CNF[3:2]" 的字段描述
   与实际寄存器编码不一致。

   因此 libopenwch **把 nibble 当作不透明寄存器值**处理，直接采用上表的
   数值（与 EVT、`ch32fun` 一致）。这样从 EVT 或手册抄来的值可以原样传给
   `gpio_set_mode()`，不会因为重新解释位域而出错。

   代价是 `gpio_set_mode()` 的签名与 libopencm3 的
   `gpio_set_mode(port, mode, cnf, pins)` 不同——这是**有意的不兼容**，
   已在头文件中显著说明。拉/下拉方向仍由 OUTDR 选择，因此
   `GPIO_MODE_IPD` 写 0、`GPIO_MODE_IPU` 写 1。

### P2 实现记录（模板 / 应用侧）

模板的引入暴露了库构建系统中的两个真实缺陷，已修正：

10. **`CC ?= $(PREFIX)-gcc` 不生效**。GNU make 预定义了 `CC = cc`（还有
    `CXX = g++`），因此 `?=` 永远不会赋值，构建会**静默地使用宿主编译器**。
    模板的 `toolchain.mk` 改用 `:=` 显式赋值。库侧的 `mk/gcc-config.mk`
    用的是 `?=`，但在 `PREFIX` 已知的前提下无害；应用侧必须用 `:=`。

11. **`ZMMUL_OK` 只在 `mk/gcc-config.mk` 中探测**，而模板只
    `include genlink-config.mk`，导致 `ZMMUL_OK` 为空、`_zmmul` **永远不被
    追加**——CH32V002/004/005/006/007 会静默退化为软乘法。已把探测移入
    `mk/genlink-config.mk`（该模块本就消费这个值），使模块自洽；调用方仍可
    预先设置 `ZMMUL_OK` 以避免重复探测。

12. **`libopenwch_ch32v0.a` 曾用 `_zmmul` 编译**，导致链接后的 CH32V003
    映像声明了一个该型号并未宣称的扩展。族归档现已改为**纯 `rv32ec`**：
    - 核心层与已实现外设中**没有任何 mul/div 指令**（objdump 验证）；
    - QingKe V2 无硬件除法，`/`/`%` 本来就总是调用 libgcc；
    - 乘法仅在**应用**请求时使用硬件，而针对 002/004/005/006/007 构应用会
      自动从 `ld/devices.data` 得到 `-march=rv32ec_zmmul_zicsr_zifencei`。
    这样单个族归档对全部型号都是正确的。

### P0 验收

- [x] `make TARGETS=""` 成功（空构建不报错）
- [x] `make genlinktests` 全部 `TEST OK`
- [x] `./scripts/genlink.py ld/devices.data ch32v003f4p6 MARCH` 输出 `rv32ec`
- [x] `PREFIX=arm-none-eabi- make` 给出明确错误（不回退）
- [x] `make stylecheck` 无报错（此时文件少）

---

## P1 — `qingke/` 核心层（对应 libopencm3 的 `cm3/`）

**目标**：让 RISC-V 芯片能启动、能进中断、能被 `nvic_*` API 控制。

### P1.1 头文件

- [x] `include/libopenwch/qingke/common.h` —— `MMIO8/16/32/64`、`BIT0..BIT31`、`BEGIN_DECLS/END_DECLS`、stdint 引入
- [x] `include/libopenwch/qingke/memorymap.h` —— `CORE_PERIPH_BASE`、`PFIC_BASE=0xE000E000`、`SYSTICK_BASE=0xE000F000`
- [x] `include/libopenwch/qingke/csr.h` —— `csr_read/csr_write/csr_set/csr_clear` 宏 + `mstatus/mtvec/mepc/mcause/mie/mip`
- [x] `include/libopenwch/qingke/pfic.h` —— `PFIC_Type` 结构（ISR/IPR/ITHRESDR/CFGR/GISR/VTFIDR/VTFADDR/IENR/IRER/IPSR/IPRR/IACTR/IPRIOR/SCTLR）
- [x] `include/libopenwch/qingke/nvic.h` —— `nvic_enable_irq/disable_irq/set_priority/get_priority/set_priority_grouping`、`NVIC_IRQ_COUNT`
- [x] `include/libopenwch/qingke/vector.h` —— `vector_table_t`（RISC-V 变体）+ `extern vector_table`
- [x] `include/libopencwch/qingke/systick.h` —— WCH SysTick（`CTLR/SR/CNT/CMP`）API
- [x] `include/libopenwch/qingke/assert.h` —— `openwch_assert` / `openwch_assert_not_reached`
- [x] `include/libopencwch/qingke/sync.h` —— `qingke_irq_enable/disable/save/restore`
- [ ] `include/libopenwch/qingke/doc-qingke.h` —— Doxygen 分组

### P1.2 实现

- [x] `lib/qingke/vector_chipset.c` —— `handle_reset` 汇编（gp/sp/highcode/data/bss/mstatus/csr0x804/mtvec/SystemInit/mret）+ `_start` 向量表头
- [x] `lib/qingke/vector.c` —— `vector_table_t vector_table`（`.init` 段）、弱 `SystemInit`、`__libc_init_array`（可选）、`DefaultIRQHandler`
- [x] `lib/qingke/vector_nvic.c` —— 由 `irq2nvic_h` 生成（编译期存在）
- [x] `lib/qingke/nvic.c` —— PFIC 映射实现
- [x] `lib/qingke/assert.c` —— 断言失败处理（可选打印 `mcause/mepc` 后复位）
- [x] `lib/qingke/systick.c`
- [x] `lib/qingke/sync.c`（临界区）
- [x] `lib/qingke/csr.c` —— `__get_MSTATUS` 等外部符号（若用 C 实现）

### P1.3 启动链路验证

- [x] 用 `-march=rv32ec -mabi=ilp32e` 编译 `lib/qingke/*.c` 成功
- [x] `riscv64-unknown-elf-objdump -d`：确认 `.init` 第一项是 `j handle_reset`
- [x] `nm`：`vector_table`、`handle_reset`、`nvic_enable_irq` 符号存在
- [x] 用 `qemu-riscv32` 或纯链接测试确认 `SystemInit`→`main` 链路可达（无外设时可 stub `rcc`）

### P1 验收

- [x] `lib/libopencwch_ch32v003.a`（仅核心层）生成成功
- [x] 归档中无未定义符号（除 libgcc/libc 可选项）
- [ ] `make -C doc html` 中 `qingke` 分组可生成

---

## P2 — CH32V003 外设驱动（一期重点一）

**目标**：`make TARGETS=ch32v/003` 全绿，并能链接出可运行固件。

### P2.1 器件基础

- [x] `include/libopenwch/ch32v003/memorymap.h`（`PERIPH_BASE=0x40000000`、`APB1/APB2/AHB`、所有 `*_BASE`、`ESIG/OB/INFO/VENDOR_CFG0`）
- [ ] `include/libopenwch/ch32v003/irq.json`（`nmi/hardfault/systick/sw/wwdg/pvd/flash/rcc/exti7_0/awu/dma1_channel1..7/adc1/i2c1_ev/i2c1_er/usart1/spi1/tim1_brk/tim1_up/tim1_trg_com/tim1_cc/tim2`）
- [x] `include/libopenwch/ch32v0/common/*.h`（`v1` 变体头文件集）
- [ ] `include/libopenwch/ch32v0/doc-ch32v0.h`、`ch32v003/doc-ch32v003.h`
- [ ] `lib/ch32v/Makefile.include`（族级 `TGT_CFLAGS`、`VPATH`）

### P2.2 驱动实现（按依赖顺序）

- [x] `rcc`（时钟树：HSI/HSE/PLL、`rcc_clock_setup_hsi_48m`、`rcc_periph_clock_enable`、`rcc_get_clocks_freq`）
- [x] `gpio` —— `gpio_set_mode(port, nibble, pins)`、`gpio_set/clear/toggle/get`、
      `gpio_port_read/write`、`gpio_port_config_lock`、`gpio_primary_remap`、
      `gpio_secondary_remap`、`gpio_exti_select_source`
      （注意：`gpio_set_mode()` 取**单个不透明 nibble**，不是 libopencm3 的
      `(mode, cnf)` 两个参数、也不是 CH58x 的 `(mode, drive)`——原因见下）
- [x] `syscfg`(AFIO)（`exti_select_source`、remap 寄存器）
- [x] `exti`（触发沿、中断使能、标志）
- [x] `usart`（波特率、数据位、停止位、校验、模式、流控、收发、DMA 挂钩）
- [x] `tim`（PWM 输出比较、输入捕获、预分频、自动重装、对齐、中断）
- [x] `spi`（主/从、时钟极性/相位、NSS、DMA）
- [x] `i2c`（主/从、地址、时钟频率、START/STOP、ACK）
- [x] `adc`（通道、采样时间、连续/扫描、外部触发、DMA、内部通道）
- [x] `dma`（通道配置、优先级、地址/长度、中断、mem2mem）
- [x] `flash`（解锁、页擦除、字/半字编程、状态）
- [x] `iwdg`/`wwdg`
- [x] `pwr`（低压/待机/停止、AWU）
- [x] `opa`（运放）
- [x] `dbgmcu`（调试挂起位）
- [x] `systick`（`systick_set_reload`、`systick_set_frequency`、延时）
- [ ] `mini-libc`（可选，`lib/common/mini_libc.c`：`memcpy/memset/strlen` + `_write`/`putchar` 弱符号；仅当用户 `-nostdlib`）

### P2.3 器件外壳

- [x] `lib/ch32v/003/Makefile`（`LIBNAME=libopenwch_ch32v003`、`OBJS`、`VPATH`、`SRCLIBDIR=../../..`）
- [ ] `lib/ch32v/003/<periph>.c` 薄壳（include `../common/<periph>_common_v1.c` 或器件特化）

### P2.4 示例与验证

- [x] `examples/ch32v003/blink/`（`main.c` + `Makefile` + `.clang-format`）
- [x] `examples/ch32v003/usart_echo/`
- [ ] `examples/ch32v003/tim_pwm/`（可选）
- [ ] `examples/ch32v003/adc_poll/`（可选）
- [ ] `make -C examples/ch32v003/blink OPENWCH_DIR=../../.. DEVICE=ch32v003f4p6` 产出 `.elf/.bin/.hex`
- [ ] `riscv64-unknown-elf-size` 报告合理（blink < 2 KiB）

### P2 验收

- [ ] `make TARGETS=ch32v/003` 全绿
- [ ] `lib/libopenwch_ch32v003.a` 包含全部一期外设的 `.o`
- [ ] `make genlinktests` 含 `ch32v003f4p6` 用例且通过
- [ ] `make stylecheck`（对该族文件）通过
- [ ] API 命名审查：全部非驼峰，且与 `project.md` §5.1 对照表一致

---

## P3 — CH582 / CH58x 外设驱动（一期重点二）

**目标**：`make TARGETS=ch5xx/58x` 全绿。

### P3.1 器件基础

- [ ] `include/libopenwch/ch582/memorymap.h`（`R32_*` 地址 → `*_BASE`；RWA 寄存器标注）
- [ ] `include/libopenwch/ch582/irq.json`
- [ ] `include/libopenwch/ch5xx/common/*.h`（`v1` 变体：`rwa`、`clk`、`gpio`、`uart`、`spi`、`i2c`、`tim`、`pwm`、`adc`、`flash`、`pwr`、`sys`）
- [ ] `include/libopenwch/ch5xx/doc-ch5xx.h`、`ch582/doc-ch582.h`
- [ ] `lib/ch5xx/Makefile.include`

### P3.2 驱动实现

- [ ] `rwa`（安全访问解锁/加锁，`rwa_unlock()`/`rwa_lock()`；所有 RWA 写操作的统一入口）
- [ ] `clk`（`clk_set_sys_clock(CLK_SOURCE_PLL_60MHz)` 等价物、HSE/HSI/32K、分频）
- [ ] `sys`（`sys_reset`、`sys_get_systick_cnt`、中断保存/恢复、`sys_safe_access`）
- [ ] `gpio`（位域式：`gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_PP, GPIO_DRIVE_5MA, GPIO1)`、`gpio_set/clear/toggle/get`、`gpio_set_irq_mode`、`gpio_pin_remap`、`gpio_group_cfg`）
- [ ] `uart`（4 实例，波特率由 `clk` 频率推导）
- [ ] `spi`（SPI0/SPI1 实例）
- [ ] `i2c`
- [ ] `tim`（timer0..3 独立寄存器；PWM 输出、捕获、中断）——对应 EVT 的 `CH58x_timer*.c`
- [ ] `pwm`（PWMX 多通道，若与 `tim` 分开实现）
- [ ] `adc`
- [ ] `flash`（ROM/RAM 划分、EEPROM 模拟，若适用）
- [ ] `pwr`（睡眠/唤醒、RTC 域）
- [ ] `systick`

### P3.3 器件外壳与示例

- [ ] `lib/ch5xx/58x/Makefile`（`LIBNAME=libopenwch_ch582`、`SRCLIBDIR=../../..`）
- [ ] `lib/ch5xx/58x/<periph>.c`
- [ ] `examples/ch582/blink/`、`examples/ch582/uart_echo/`
- [ ] 确认 `-march=rv32imac -mabi=ilp32`、`.highcode` 段、`0x20003800` RAM 偏移（`ch571/573` 才用，58x 不用）等差异

### P2.6 用户应用模板（`template/`）

- [x] `template/rules/toolchain.mk` —— RISC-V 前缀探测、`PREFIX` 覆盖、
      工具链缺失硬报错、`monitor`/`unbrick` 目标
- [x] `template/rules/rules.mk` —— `PROJECT`/`DEVICE` 驱动；`-nostartfiles`
      （必须，否则与工具链 crt0 冲突）；`${PROJECT}.{elf,bin,hex,map,list}`；
      `flash`/`size`/`monitor`/`unbrick`；`LIBOPENWCH_NOSTDLIB` 选项
- [x] `template/examples/blink` —— CH32V003 可编译可链接（1124 B）
- [x] `template/examples/ch582_blink` —— CH58x 核心层 bring-up（SysTick 1 ms）
- [x] `template/README.md`、`template/.gitignore`、`template/.vscode/*`
- [x] 验证 `DEVICE=` 切换会正确改变 ISA（V003 无 zmmul，V002/V004 有）

### P2 完成记录（CH32V00x 全部一期外设）

一期列出的 15 个外设全部实现并归档，`lib/libopenwch_ch32v0.a` 共 **316 个公开函数**：

| 外设 | 函数数 | 说明 |
|---|---|---|
| adc | 34 | 含 CH32 特有的校准电压（CALVOL）与外部触发延时（DLYR） |
| timer | 32 | TIM1/TIM2，通道表驱动的 CCxS/CCER 字段推导 |
| spi | 30 | 含 CRC 与双向模式 |
| i2c | 30 | 时钟频率由 `rcc_get_clocks_freq()` 推导，标准/快速模式 |
| dma | 29 | 7 通道，INTFR/INTFCR 的 4 bit/通道全局标志 |
| usart | 25 | 12.4 定点波特率分频器，含溢出保护 |
| rcc | 24 | HSI/HSE→48 MHz、工厂 PLL trim、外设时钟/复位、分频器 |
| flash | 20 | 标准路径 + buffered fast 编程路径 |
| nvic | 15 | PFIC 后端 |
| gpio | 15 | 含 AFIO remap 与 EXTI 源选择 |
| systick | 9 | |
| pwr | 9 | PVD、AWU、standby |
| exti | 8 | |
| wwdg / iwdg | 7 / 7 | |
| dbgmcu | 6 | |
| opa | 4 | 走 EXTEN 寄存器 |

**新增测试**：`tests/ch32v0/api_smoke.c` —— 调用**每一个**公开函数，以
`-Wall -Wextra -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes
-Wundef -Wshadow -Werror` 编译并链接，链接后镜像含 280 个外设函数。
`main()` 只取 `api_smoke` 的地址（阻止 `--gc-sections` 丢弃）而**不调用**它，
因为真跑会把所有外设重配并擦除 flash。用 `make apitest` 运行。

**一致性检查**（每次构建都会做的话更好，目前是一次性脚本）：

- 37 个 `OBJS` 目标全部唯一解析到存在的源文件，无 VPATH 遮蔽，无空文件
- 归档中无重复的全局符号
- 全部公开函数名符合 `^[a-z][a-z0-9_]*$`（仅 `_start`/`_reset_entry` 例外，
  它们是运行时符号）

### P3 验收

- [ ] `make TARGETS=ch5xx/58x` 全绿
- [ ] `lib/libopenwch_ch582.a` 生成，符号完整
- [ ] 示例 `.elf/.bin/.hex` 生成
- [ ] `make TARGETS="ch32v/003 ch5xx/58x"` 同时构建成功（两族 ISA 不冲突）
- [ ] RWA 协议在文档中明确，且**没有**在任何非解锁路径写 RWA 寄存器

---

## P4 — 示例、文档、CI 与硬件验证

- [ ] `doc/Makefile`、`doc/templates/*`、`doc/source/` 移植；`make -C doc html` 对两族成功
- [ ] `README.md` 完整化（安装工具链、构建、用自己的板子）
- [ ] `tests/ch32v003-generic/`、`tests/ch582-generic/` 最小系统冒烟工程
- [ ] `.github/workflows/ci.yml`：工具链矩阵（`riscv64-unknown-elf` / `riscv-none-elf` / xpack）× 目标矩阵（`ch32v/003`、`ch5xx/58x`）
- [ ] `NOTICE`（参考来源与许可说明）
- [ ] `CHANGELOG.md`
- [ ] 硬件在环：用 WCH-Link + `minichlink`（参考 `ch32fun-master/minichlink`）闪写
  - [ ] CH32V003 blink 实机通过
  - [ ] CH32V003 uart echo 实机通过
  - [ ] CH582 blink 实机通过
  - [ ] CH582 uart echo 实机通过
- [ ] `status.md` 全面更新，`phase.md` 勾选完毕

---

## P5 — 扩展族与 USB/BLE 接口（后续）

- [ ] `ch32x035`（X 系列；无 PLL，`-DCH32X03x`）
- [ ] `ch32v103`（RV32IMAC，`TARGET_MCU_LD==1` 式内存划分）
- [ ] `ch32v203/208`（`CH32V20x_D6/D8/D8W` 变体，EXT Flash 分段）
- [ ] `ch32v303/305/307`（`-march=rv32imafc -mabi=ilp32f`、`TARGET_MCU_MEMORY_SPLIT`）
- [ ] `ch32l103`
- [ ] `ch571/573`（RAM `0x20003800`、`_sdma_safe` 段）
- [ ] `ch591/592`
- [ ] `ch32h41x`（双核 V3F/V5F、ITCM/DTCM）—— 长期
- [ ] `lib/usb/`：USB 设备控制器主机/设备模式（WCH USB2.0 FS）
- [ ] BLE 的 HCI/寄存器级接口（不含协议栈）
- [ ] `libopencmsis/` 完整化，提供 WCH EVT 迁移桥

---

## 里程碑检查点（Reviewer Checklist）

每个阶段结束前**必须**逐条确认：

1. [ ] `git status` 干净，提交信息符合约定（`<area>: <imperative summary>`）
2. [ ] `make clean && make TARGETS="ch32v/003 ch5xx/58x"` 从零成功
3. [ ] `make genlinktests` 全绿
4. [ ] `make stylecheck` 无输出
5. [ ] 新增 API 全部为非驼峰，且已在 `project.md` §5.1 对照表中登记
6. [ ] 新增寄存器宏与 WCH 手册命名一致（`HACKING` 要求）
7. [ ] `phase.md` 勾选更新
8. [ ] `status.md` 更新（完成项、阻塞项、下一步）
9. [ ] `AGENTS.md` 中若工具链/流程变化则同步更新
