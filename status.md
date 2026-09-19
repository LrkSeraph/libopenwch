# libopenwch — 项目状态（status.md）

> 总体设计见 `project.md`，阶段性目标见 `phase.md`，入口索引见 `AGENTS.md`。
>
> **维护约定**：本文件是**唯一**的进度事实来源。每完成一个任务就立即更新；
> 每次会话结束前必须刷新「下一步」与「阻塞项」。

---

## 快照

| 项 | 值 |
|---|---|
| 当前阶段 | **P3 完成；P4 软件部分完成，硬件在环受阻** |
| 阶段进度 | P0: 100% ｜ P1: 100% ｜ P2: 100% ｜ P3: 100% ｜ **P4: 75%**（缺硬件在环 + Doxygen 实测）｜ P5: 0% |
| 最近更新 | P4 软件部分：Doxygen 配置、CI 工作流（工具链×目标矩阵）、NOTICE。P3 完成：CH58x 12 个外设（202 个公开函数）、`tests/ch5xx58x/api_smoke.c`、`ch582_blink` 改为真正的 blink；累计 **518 个公开函数**，两族均 0 警告 0 重名 0 命名违规 |
| 构建状态 | ✅ `make` 全绿：`lib/libopenwch_ch32v0.a`、`lib/libopenwch_ch5xx58x.a` |
| 工具链状态 | ✅ `riscv64-unknown-elf-gcc` 15.3.0-24 |
| 仓库状态 | ✅ 7 个提交；构建后工作区依然干净 |

### 可复现的验证命令与结果

```console
$ make                       # 全绿
  BUILD   lib/ch32v0
  CC      vector_handlers.c ... AR libopenwch_ch32v0.a
  BUILD   lib/ch5xx58x
  CC      vector_handlers.c ... AR libopenwch_ch5xx58x.a

$ ls lib/*.a
lib/libopenwch_ch32v0.a   lib/libopenwch_ch5xx58x.a

$ make genlinktests
  TEST  OK  : ld/tests/ch32v002f4p6
  TEST  OK  : ld/tests/ch32v003f4p6
  TEST  OK  : ld/tests/ch32v303vct6
  TEST  OK  : ld/tests/ch571f
  TEST  OK  : ld/tests/ch582m
  TEST  OK  : ld/tests/ch584m

$ make list-targets
ch32v0 ch5xx58x

$ ./scripts/genlink.py ld/devices.data ch32v003f4p6 MARCH
rv32ec
```

链接冒烟测试（P0 的硬性验收，用一个真实 `.elf` 证明链接脚本、向量表布局和库三者的契约）：

```console
$ riscv64-unknown-elf-gcc -march=rv32ec_zicsr_zifencei -mabi=ilp32e \
      -DCH32V0 -DCH32V003F4P6 -D_ROM=16K -D_RAM=2K -D_ROM_OFF=0x00000000 \
      -D_RAM_OFF=0x20000000 -D_GP=1 -D_HICODE=0 -P -E ld/linker.ld.S -o generated.ld
$ riscv64-unknown-elf-gcc ... -Tgenerated.ld main.o -Llib -lopenwch_ch32v0 \
      -nostdlib -lgcc -o smoke.elf
LINK OK        # 链接脚本内置的 ASSERT 全部通过

$ riscv64-unknown-elf-nm -n smoke.elf | grep -E "_vector_base|vector_table"
00000004 D _vector_base      # .init 之后、4 字节对齐
00000044 D vector_table      # == _vector_base + 64   ✔
$ riscv64-unknown-elf-objdump -d -j .init smoke.elf
   0: 1540006f   j 154 <_reset_entry>     # 映像首字必须是可执行的跳转
$ riscv64-unknown-elf-objdump -s -j .vector smoke.elf
 0014 a9bdf9f3 ...                        # slot 4 == 0xF3F9BDA9 (WCH magic) ✔
```

CH582（RV32IMAC + `.highcode`）同样链接成功并生成 `.highcode` 输出段。

---

## 已完成

### 规划阶段

- [x] 通读四个参考工程（`libopencm3` / `ch32fun` / `ch32v003 EVT` / `ch583 EVT`），
      确认设计约束。详细结论见 `project.md` §2。
- [x] 确定关键技术约束
  - CH32V003 = `rv32ec/ilp32e`，CH58x = `rv32imac/ilp32` → **必须分库**
  - WCH PFIC 寄存器布局与 ARM NVIC 同构 → `nvic_*` API 可同名保留
  - **PFIC 索引 = 原始向量表 slot 号（外部中断从 16 开始）**。
    实证：`ch32fun` 用 `PFIC->IPRIOR[24]` 设置 CH32V003 的 DMA1Ch3 优先级，
    而 `DMA1_Channel3_IRQn == 24`
  - **`mtvec = base | 3`（WCH mode 3）** = 绝对地址向量模式，表项是**完整 32 位地址**
    （不是 `j` 指令）。CH32V003 与 CH583 的启动汇编都确认了这一点
  - CH58x 的 RWA 寄存器需 `0x57/0xA8` 签名解锁，窗口仅约 16 个系统时钟周期
  - **主line GCC 拒绝 `"WCH-Interrupt-fast"`**，会退化成普通 `ret`
- [x] 编写 `project.md` / `phase.md` / `status.md` / `AGENTS.md`

### P0 — 仓库骨架与构建系统（100%）

- [x] `.gitignore` / `.clang-format` / `HACKING` / `HACKING_COMMON_DOC` / `README.md`
- [x] `COPYING.LGPL3` / `COPYING.GPL3` / `scripts/checkpatch.pl`（取自 libopencm3）
- [x] `mk/gcc-config.mk` — RISC-V 前缀探测链、工具链能力探测、`ZMMUL_OK`
- [x] `mk/gcc-rules.mk` — `%.o`/`%.elf`/`%.bin`/`%.hex`/`%.srec`/`%.list`
- [x] `mk/genlink-config.mk` — `MARCH`/`ZEXT`/`MABI`/`ZMMUL` → `ARCH_FLAGS`，
      库自动选择 `libopenwch_<family>.a`
- [x] `mk/genlink-rules.mk` — `generated.$(DEVICE).ld` 生成
- [x] `mk/README`、`ld/README`
- [x] `scripts/genlink.py` — RISC-V 字段扩展（`MARCH`/`MABI`/`ZEXT`/`ZMMUL`/`ENTRY`）
- [x] `scripts/irq2nvic_h` — RISC-V 化，输出 `nvic.h` + `vector_handlers.c` +
      `vector_names.c` + `irqhandlers.h`；**修正了 libopencm3 的 `./` 路径 bug**
- [x] `scripts/genlinktest.sh` — 直接查询真实 `ld/devices.data`（不保留副本）
- [x] `ld/devices.data` — 7 个族组、19 个型号族，全部可解析
- [x] `ld/linker.ld.S` — RISC-V 通用模板，含 3 条构建期 `ASSERT`
- [x] `lib/Makefile.include` — 单库归档 + 工具链缺失硬报错
- [x] 根 `Makefile` — `TARGETS` 递归、失败聚合、`genlinktests`、`stylecheck`
- [x] `ld/tests/` 6 组用例，`make genlinktests` 全绿

### P1 — `qingke/` 核心层（100%）

- [x] `include/libopenwch/qingke/`：`common.h` `memorymap.h` `csr.h` `pfic.h`
      `nvic.h` `vector.h` `systick.h` `assert.h` `sync.h`
- [x] `lib/qingke/vector_chipset.S` — `.init` 入口跳转 + `.vector_exc`（slot 0..15）
- [x] `lib/qingke/vector.c` — `_reset_entry` / `openwch_reset_init` /
      `openwch_vector_set/get` / `openwch_irq_name` / 弱钩子
- [x] `lib/qingke/nvic.c` — PFIC → `nvic_*`（含嵌套、阈值、快速中断、软复位）
- [x] `lib/qingke/assert.c`、`lib/qingke/systick.c`（含 `qingke_delay_us/ms`）
- [x] 两个族都能编译、归档、链接；向量表断言通过
- [x] `make stylecheck` 可运行（结果仅作参考，见「已知问题」）

### P2 — CH32V00x 外设驱动（进行中，15%）

- [x] `include/libopenwch/ch32v0/memorymap.h`（总线 + 全部 `*_BASE` + `ESIG`/`OB`/`VENDOR_CFG0`）
- [x] `include/libopenwch/ch32v0/common/gpio_common_v1.h` + `lib/ch32v0/common/gpio_common_v1.c`
  - `gpio_set_mode()`（不透明 nibble）、`gpio_set/clear/toggle/get`、
    `gpio_port_read/write`、`gpio_port_config_lock`（LCKR 序列）、
    `gpio_primary_remap`/`gpio_secondary_remap`、`gpio_exti_select_source`
- [x] `include/libopenwch/ch32v0/rcc.h` + `lib/ch32v0/common/rcc_common_v1.c`
  - `rcc_clock_setup_hsi_48mhz()`（含读 `CFG0_PLL_TRIM` 工厂 trim）、
    `rcc_clock_setup_hse_48mhz()`、`rcc_clock_setup_sysclk()`、
    `rcc_get_clocks_freq()`、外设时钟/复位（带总线的标识符编码）、
    分频器、CSS、复位标志
- [x] 设备薄壳 `lib/ch32v0/{gpio,rcc}.c`
- [x] 在 `lib/ch32v0/Makefile` 的 `OBJS` 中启用
- [x] 用真实程序验证：编译 + 链接 + 反汇编确认写入的 nibble 与 WCH EVT 一致

**待办**：`usart`、`tim`、`spi`、`i2c`、`adc`、`dma`、`exti`、`flash`、
`iwdg`/`wwdg`、`pwr`、`opa`、`dbgmcu`；库内 `examples/`；`tests/` 自动化。

### P2.6 — 用户应用模板（`template/`）

- [x] `template/rules/toolchain.mk` —— RISC-V 前缀探测链、`PREFIX` 覆盖、
      工具链缺失硬报错、minichlink 的 `monitor`/`unbrick`
- [x] `template/rules/rules.mk` —— `PROJECT`/`DEVICE` 驱动；`-nostartfiles`；
      `.elf/.bin/.hex/.map/.list`；`flash`/`size`/`clean`；
      `LIBOPENCHW_NOSTDLIB` 选项
- [x] `template/examples/blink`（CH32V003，1124 B text）与
      `template/examples/ch582_blink`（CH58x 核心层，376 B text）
- [x] `template/README.md`（完整变量参考）、`.gitignore`、`.vscode/{c_cpp_properties,settings,tasks,launch}.json`
- [x] 验证 `DEVICE=` 切换会正确改变 ISA 与链接脚本

**该模板暴露并已修正 3 个库侧缺陷**（详见 `phase.md`「P2 实现记录（模板 / 应用侧）」）：

1. `CC ?= $(PREFIX)-gcc` 永不生效（GNU make 预定义 `CC=cc`）→ 曾静默使用宿主编译器
2. `ZMMUL_OK` 只在 `gcc-config.mk` 探测，应用侧从不 include 它 → `_zmmul` 永不追加
3. `libopenwch_ch32v0.a` 曾用 `_zmmul` 编译 → 链接出的 CH32V003 映像声明了该型号没有的扩展；
   现已改为纯 `rv32ec`（核心与外设中无任何 mul/div，已用 objdump 验证）

### P2 — CH32V00x 外设驱动（完成，100%）

一期列出的 15 个外设全部实现并归档，`lib/libopenwch_ch32v0.a` 含 **316 个公开函数**：

- [x] `gpio`（含 AFIO remap / EXTI 源选择）、`rcc`、`usart`
- [x] `tim`、`spi`、`i2c`、`adc`、`dma`、`exti`
- [x] `flash`（标准 + fast 路径）、`iwdg`、`wwdg`、`pwr`、`opa`、`dbgmcu`
- [x] `systick`（P1 已含）
- [x] `lib/ch32v0/Makefile` 的 `OBJS` 全部启用（37 个目标）

**新增测试设施**：

- [x] `tests/ch32v0/api_smoke.c` + `tests/ch32v0/Makefile` —— 调用每个公开函数，
      以 `-Werror` 与库相同的警告集编译**并链接**；镜像内含 280 个外设函数
- [x] 根 `Makefile` 的 `make apitest` / `apitest.clean`
- [x] `api_smoke` 的 `main()` 只取函数地址而不调用（避免真跑破坏芯片）

**一致性核验**（一次性脚本，见 `phase.md`「P2 完成记录」）：

- 37 个目标全部唯一解析，无 VPATH 遮蔽、无空文件
- 归档无重复全局符号
- 全部公开函数名符合 `^[a-z][a-z0-9_]*$`

### P3 — CH58x 外设驱动（完成，100%）

- [x] `memorymap.h`（SFR 平坦窗口）、`rwa`（安全访问）、`clk`、`gpio`
- [x] `sys`、`pwr`、`flash`、`uart`（4 实例）、`spi`（2 实例）、`i2c`
- [x] `tmr`（4 实例）、`pwm`（PWMX）、`adc`（含 touchkey）
- [x] `lib/ch5xx58x/Makefile` 的 `OBJS` 全部启用（29 个目标）
- [x] `tests/ch5xx58x/api_smoke.c`（172 个外设函数链接进镜像）
- [x] `template/examples/ch582_blink` 改为真正的 blink

CH58x 共 **202 个公开函数**。两族合计 **518 个**。

**诚实的缺口**：`flash` 的擦除/编程需要 WCH 的二进制 `libISP583.a`（ROM 例程）
或外部烧写器，libopenwch 不链接它，因此这两个函数是明确返回
`FLASH_STATUS_UNSUPPORTED` 的 stub，并在头文件说明。可读部分
（`flash_read`/`flash_rom_read`/`flash_get_unique_id`/`flash_get_chip_id`/
`flash_set_latency`）是真实实现。

### 相对初版规划的设计修正

实现中发现并修正的问题已详细记录在 `phase.md` 的「P0 实现记录」一节，摘要：

1. 目标名/库目录名/genlink FAMILY **统一为 `ch32v0`、`ch5xx58x`** 等单层名
2. `devices.data` 的器件模式必须用 `?*`、组行必须精确、`+` 续行必须在 `END` 之前
3. 向量表拆成 `.vector_exc` + `.vector` 两个输入段，由链接脚本显式拼接
4. `_reset_entry` 主体用 C 实现，汇编只建立 `gp`/`sp`
5. **必须显式声明 `zicsr`/`zifencei`**（GCC ≥ 12 不再隐含）
6. `mk/` 全程使用 `$(abspath)` 绝对路径（沙箱/虚拟化文件系统下 `..` 不可靠）
7. `blocking_handler()` 由生成器定义在 `vector_handlers.c` 内（GCC alias 约束）
8. `OPENWCH_INTERRUPT` 默认用可移植的 `__attribute__((interrupt()))`
9. **CH32V00x 的 GPIO nibble 不是 (MODE, CNF) 位域打包**：穷举所有字段位置都
   无法复现 WCH 手册的 `GPIO_Mode_*` 值，因此是把 nibble 当作**不透明寄存器值**
   处理。`gpio_set_mode(port, nibble, pins)` 与 libopencm3 的三参数签名
   **有意不兼容**，已在头文件与 `phase.md` 说明
10. **`mk/gcc-config.mk` 的工具变量用了 `?=`，等于从未赋值**。GNU make 预定义
    `CC=cc`、`LD=ld`、`AS=as` 等内建变量，`?=` 不会覆盖它们，因此
    `CC`/`LD`/`AS` 一直解析为**宿主工具**。已改为 `:=`（命令行 `make CC=...`
    仍然优先）。这与第 2 轮在应用模板中发现的是同一类缺陷，当时只修了模板侧，
    库侧漏掉了
11. **`DBGMCU` 不是内存映射外设**。WCH EVT 用 `csrr/csrw 0x7c0` 访问调试控制
    寄存器，芯片版本/型号在 `0x1ffff7c4`；CH32V003 的 SVD 里根本没有 DBGMCU
    外设节点。原先按 `DBGMCU_BASE=0xe0042000` 的内存映射实现是错的，已重写。
    位定义存在**未解决的分歧**（见「已知问题」K4）
12. **CH58x 的 RWA helper 必须 `always_inline`**。若编译成真实函数，16 个系统时钟的
    解锁窗口要跨过 `ret`、返回和调用方的地址计算才轮到寄存器写入，不保证完成。
    反汇编确认 `clk_set_sys_clock()` 内联了 8 个窗口、没有外部 helper 调用
13. **`memorymap.h` 的 CH58x 身份/复位寄存器偏移写错了**：`R8_CHIP_ID` 应为
    `SYS_BASE+0x41`（原写 `+0x46`，那是 `R8_RST_WDOG_CTRL`），
    `R8_GLOB_RESET_KEEP` 应为 `+0x47`（原写 `+0x44`）。两个独立的实现代理同时报出
    了这个问题。已按 `CH583SFR.h` 修正并补齐 `R8_RESET_STATUS`/`R8_GLOB_CFG_INFO`/
    `R8_WDOG_COUNT`/`R8_SLP_*`/`R8_CK32K_CONFIG`/`R8_BAT_DET_*`。
    教训：这类偏移写错不会编译报错，只会静默读到旁边的寄存器
14. **AFIO remap 位定义取自 SVD 而非 EVT 的打包 token**。EVT 把"值/位号/半字选择/
    类别"打包进一个 32 位 token 再运行时解包，极易出错；libopenwch 改为直接暴露
    SVD 中的字段，并为 USART1/I2C1/TIM1/TIM2 提供 2-bit 字段级 helper。
    这一改动同时修正了原先写错的 `AFIO_PCFR1_PA1_PA2_REMAP`（应为 bit 15，不是 12）

---

## 未开始（P4 剩余 / P5 待办）

P2 已全部完成，见上一节。CH58x 侧（P3）尚未开始：

P3 已全部完成，见上一节。P4 待办：

- ⬜ Doxygen：把 `doc/Makefile` 接上真的 doxygen，生成两族文档
- ⬜ CI：GitHub Actions 工具链矩阵 + 目标矩阵
- ⬜ `NOTICE`（参考来源与许可说明）
- ⬜ `tests/` 里的链接冒烟测试（目前链接脚本测试在 `ld/tests/`，API 测试在 `tests/`）
- ⬜ 硬件在环验证（**需用户提供 WCH-Link + 板子**）

---

## P4 — 示例、文档、CI 与硬件验证（软件部分完成）

- [x] `doc/Doxyfile.in` + `doc/Makefile`（单模板按族替换；缺 doxygen 时优雅跳过）
- [x] `.github/workflows/ci.yml`：工具链矩阵 × 目标矩阵 + "两族同时 make" 作业；
      CI 的每条命令都已在本机逐条验证
- [x] `NOTICE`：来源与许可（libopencm3 派生 / WCH EVT 仅参考 / ch32fun 影响工具链策略 /
      本项目原创），并说明未使用 ch32fun 的 `misc/libgcc.a`
- [ ] ⛔ 硬件在环验证：需 WCH-Link + CH32V003/CH582 板子（**外部条件**）
- [ ] ⛔ Doxygen 实际生成：本机无 `doxygen` 可执行文件

## 阻塞项 / 待决

**当前无阻塞项。**

| # | 项 | 类型 | 影响 | 处理 |
|---|---|---|---|---|
| B1 | 无硬件（WCH-Link + CH32V003/CH582 板） | 环境 | P4 硬件在环无法在本机完成 | 不阻塞 P0–P3 |
| B2 | 本工作区的虚拟化文件系统对子目录路径的 `..` 解析不正确 | 环境 | 已规避 | 构建系统全部改用 `$(abspath)` 绝对路径 |
| B3 | 工具链 multilib 列表不含 `rv32ec` 字样，但 `rv32e/ilp32e/libgcc.a` 存在且可用 | 环境 | 无 | 已在 `mk/gcc-config.mk` 做能力探测 |
| B4 | **无硬件**：无法做硬件在环验证 | 外部条件 | P4 最后两项之一无法完成 | 软件侧已用「全 API 编译+链接 + objdump 核对寄存器时序」替代；需用户提供 WCH-Link + 板子 |
| B5 | 本机未安装 `doxygen` | 环境 | Doxygen 生成未实测 | `doc/Makefile` 缺失时优雅跳过；配置已写好，安装后即可验证 |

### 已知问题（不阻塞）

| # | 问题 | 说明 |
|---|---|---|
| K1 | `make stylecheck` 对 `volatile` MMIO、`do {} while(1)` 断言、复杂宏报 warning/error | 这些是 `checkpatch.pl` 对内核风格之外的合理用法的固有误报；libopencm3 自身的 `cm3/common.h` 同样报 3 errors / 7 warnings。定位为**参考工具**而非门禁，已在 `HACKING` 说明 |
| K2 | `.vector` 段设为只读数据（`rodata` PHDR），ISR 目标仍在 `.text` | 与 WCH EVT 一致；两者都在 flash。运行期改表（`OPENWCH_VECTOR_SET`）只在表位于 RAM 时生效，已在 `vector.h` 注明 |
| K3 | ~~P0 的链接脚本冒烟测试尚未纳入 `make` 目标~~ | ✅ 已解决：`make apitest` + `make genlinktests` |
| K4 | **`DBGMCU_CR`（CSR 0x7c0）的位定义来源冲突，未经硬件验证** | WCH EVT 用 `IWDG_STOP=0x1 / WWDG=0x2 / TIM1=0x10 / TIM2=0x20`；`ch32fun` 的 `ch32v003hw.h` 用 `IWDG_STOP=0x100 / TIM1_STOP=0x1000`，但那些常量是**死代码**（从内存映射的 V20x/V30x 复制、从未被引用）。已采用 EVT 的值并在头文件注明；同时提供 `dbgmcu_get_control()`/`dbgmcu_set_control()` 裸 CSR 访问，用户可自行决定。**需要在真实硬件上确认** |

---

## 下一步（Next Actions，按序）

1. **P2.1** `include/libopenwch/ch32v0/memorymap.h`：`PERIPH_BASE`、`APB1/APB2/AHB`、
   全部 `*_BASE`、`ESIG`/`OB`/`INFO`/`VENDOR_CFG0`。
2. **P2.2** `include/libopenwch/ch32v0/common/gpio_common_v1.h` + `lib/ch32v0/gpio_common_v1.c`：
   `gpio_set_mode()`（CNF/MODE nibble，`GPIOA/C/D` 8 位端口，无 CFGHR）、
   `gpio_set/clear/toggle/get`（`BSHR`/`BCR`/`INDR`）、`gpio_port_*`、
   `gpio_lock_config`、`gpio_primary_remap`、`gpio_set_eventout`。
3. **P2.3** `rcc`（HSI 24 MHz + PLL×2 = 48 MHz，读 `CFG0_PLL_TRIM`）、
   然后 `usart`、`tim`、`spi`、`i2c`、`adc`、`dma`、`exti`、`flash`、`iwdg`/`wwdg`、
   `pwr`、`opa`、`syscfg`、`dbgmcu`。
4. **P2.4** 每加一个外设就在 `lib/ch32v0/Makefile` 的 `OBJS` 中启用对应行，并跑一次 `make`。
5. **P2.5** `examples/ch32v003/blink` 与 `usart_echo`，用 `OPENWCH_DIR` + `DEVICE` 构建。
6. **P2.6** 模板骨架已完成（`template/`，两个示例可构建）。新增外设时在
   `template/examples/` 下补对应示例即可。
7. **P3** 同法推进 CH58x（先 `rwa.c`，再 `clk.c`，其余依赖它）；届时
   `template/examples/ch582_blink` 可换成真正的 blink。
8. **P4** 把链接冒烟测试固化为 `tests/` 用例；补 Doxygen、CI、`NOTICE`。

---

## 验证记录（本机）

| 检查 | 命令 | 结果 |
|---|---|---|
| 工具链版本 | `riscv64-unknown-elf-gcc --version` | ✅ 15.3.0-24 |
| `rv32ec_zmmul` 是否被接受 | `-march=rv32ec_zmmul -fsyntax-only` | ✅ 接受（`ZMMUL_OK=yes`） |
| `rv32e/ilp32e` 的 libgcc | `-print-libgcc-file-name` | ✅ `.../15/rv32e/ilp32e/libgcc.a` |
| 全量构建 | `make` | ✅ 2 个归档，无警告 |
| 链接脚本用例 | `make genlinktests` | ✅ 6/6 `TEST OK` |
| 型号解析 | `genlink.py ... {FAMILY,SUBFAMILY,MARCH,MABI,ZMMUL,ENTRY,GP,HICODE}` | ✅ 19 个型号全部完整（见 `phase.md`「P0 实现记录」） |
| 空目标构建 | `make TARGETS=` | ✅ 成功 |
| 工具链缺失报错 | `make PREFIX=nonexistent-tc-` | ✅ 明确报错并给出安装提示 |
| CH32V003 链接 | `-march=rv32ec_zicsr_zifencei -mabi=ilp32e -Tgenerated.ld` | ✅ `LINK OK`，3 条 ASSERT 通过 |
| CH32V003 向量表 | `nm`/`objdump` | ✅ `_vector_base=4`、`vector_table=0x44`（+64）、slot4=`0xF3F9BDA9` |
| CH32V003 入口 | `objdump -d -j .init` | ✅ `j _reset_entry` |
| CH582 链接 | `-march=rv32imac_zicsr_zifencei -mabi=ilp32` | ✅ `LINK OK`，`.highcode` 段存在 |
| 归档 ISA 标签 | `readelf -A` | ✅ `rv32e...zicsr...zifencei` / `rv32i_m_a_c...` |
| 公开 API 命名审查 | `grep` 核心层函数名 | ✅ 全部 lowercase_snake_case，无驼峰 |
| `make stylecheck` | `scripts/checkpatch.pl` | ⚠️ 仅 `volatile`/断言宏误报（见 K1） |
| 模板 blink 构建 | `make -C template/examples/blink` | ✅ 1124 B text / 156 B data |
| 模板 CH58x 构建 | `make -C template/examples/ch582_blink` | ✅ 376 B text |
| 模板 `DEVICE=` 切换 | `make DEVICE=ch32v003f4p6 / ch32v002f4p6` | ✅ ISA 分别不含/含 `zmmul` |
| 同族换型号 | `make -C template/examples/ch582_blink DEVICE=ch584m` | ✅ 构建成功 |
| 全 API 编译+链接 | `make apitest` | ✅ `-Werror` 通过，镜像含 280 个外设函数 |
| public 函数总数 | `nm --defined-only lib/libopenwch_ch32v0.a` | 316 个 |
| OBJS 解析 | 脚本核验 37 个目标 | ✅ 无缺失、无歧义、无 VPATH 遮蔽 |
| 重复全局符号 | `nm` + `uniq -d` | ✅ 无 |
| `mk/gcc-config.mk` 工具解析 | `make -f probe.mk` | ✅ `riscv64-unknown-elf-gcc`（修复前是 `cc`） |
| DBGMCU 指令 | `objdump -d dbgmcu_common_v1.o` | ✅ `csrr/csrw 0x7c0`、`lw 0x1ffff7c4` |
| RWA 解锁序列 | `objdump -d clk.o` | ✅ 8 个内联 `0x57`/`0xA8` 窗口，无外部 helper |
| CH58x 全 API 编译+链接 | `make apitest` | ✅ 172 个外设函数链接进镜像 |
| 两族函数总数 | `nm --defined-only lib/*.a` | ✅ ch32v0=316，ch5xx58x=202，合计 518 |
| 两族归档审计 | 重名符号 / 命名违规 | ✅ 均为 0 |
| CH582 真 blink | `make -C template/examples/ch582_blink` | ✅ 2808 B |

---

## 变更日志

| 日期/轮次 | 变更 |
|---|---|
| 初始轮 | 创建 `project.md`、`phase.md`、`status.md`、`AGENTS.md`；完成四工程勘察与设计决策 |
| 第 2 轮 | 用户确认 Q1–Q4；实测工具链能力；完成 **P0**（骨架 + `mk/` + `scripts/` + `ld/` + 根 Makefile）与 **P1**（`qingke/` 核心层）；两族归档构建成功并通过链接冒烟测试；修正 8 项设计问题（见 `phase.md`） |
| 第 2 轮末 | 按用户要求提交已验证基线：`9a8ed34` "Initial libopenwch: libopencm3-style build system and QingKe core layer"（56 文件）。生成物已由 `.gitignore` 排除，提交后 `make clean && make && make genlinktests` 仍全绿且工作区干净 |
| 第 3 轮 | P2.1–P2.2：`ch32v0/memorymap.h` + `gpio` + `rcc`，提交 `8026add`。发现 GPIO nibble 不是 `(MODE,CNF)` 位域（穷举无解），改为不透明值，`gpio_set_mode()` 签名有意偏离 libopencm3 |
| 第 4 轮 | P2.6：新增 `template/` 应用骨架（rules/ + 两个示例 + VSCode 配置），提交 `7b0df2f`。模板暴露并修正 3 个库侧缺陷（`CC ?=` 失效、`ZMMUL_OK` 未在应用侧探测、族归档 ISA 标签过宽） |
| 第 5 轮 | **P2 完成**：CH32V00x 15 个外设、316 个公开函数；新增 `tests/ch32v0/api_smoke.c` 与 `make apitest`。独立复核 6 个并行实现代理的产出，发现并修正 2 处实质错误：GPIO nibble 位域假设（穷举证明无解→改为不透明值）、DBGMCU 误按内存映射实现（实为 CSR 0x7c0）；另修正 `mk/gcc-config.mk` 的工具变量 `?=` 缺陷与写错的 AFIO remap 位（bit 15 而非 12） |
| 第 6 轮 | **P3 完成**：CH58x 12 个外设（202 个公开函数）、`tests/ch5xx58x/api_smoke.c`、`ch582_blink` 改为真正的 blink。修正 `memorymap.h` 的 CH58x 身份/复位寄存器偏移（`R8_CHIP_ID`/`R8_GLOB_RESET_KEEP` 写错，由两个独立代理同时报出）与 `RWA` helper 必须 `always_inline` 的时序要求。两族合计 518 个公开函数，全部 0 警告、0 重名、0 命名违规 |
