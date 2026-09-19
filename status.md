# libopenwch — 项目状态（status.md）

> 总体设计见 `project.md`，阶段性目标见 `phase.md`，入口索引见 `AGENTS.md`。
>
> **维护约定**：本文件是**唯一**的进度事实来源。每完成一个任务就立即更新；
> 每次会话结束前必须刷新「下一步」与「阻塞项」。

---

## 快照

| 项 | 值 |
|---|---|
| 当前阶段 | **P1 完成，进入 P2（CH32V00x 外设驱动）** |
| 阶段进度 | P0: 100% ｜ **P1: 100%** ｜ P2: 0% ｜ P3: 0% ｜ P4: 0% ｜ P5: 0% |
| 最近更新 | 完成 P0 构建系统 + P1 `qingke/` 核心层；两个族均可编译出静态库并通过链接冒烟测试 |
| 构建状态 | ✅ `make` 全绿：`lib/libopenwch_ch32v0.a`、`lib/libopenwch_ch5xx58x.a` |
| 工具链状态 | ✅ `riscv64-unknown-elf-gcc` 15.3.0-24 |
| 仓库状态 | 📄 源码树完整，`git init` 已执行（尚未提交） |

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

---

## 未开始（P2 待办摘要）

- ⬜ `include/libopenwch/ch32v0/common/*.h`（`v1` 变体头文件集，15 个外设）
- ⬜ `include/libopenwch/ch32v0/memorymap.h` + 各外设设备头
- ⬜ `include/libopenwch/ch32v0/doc-ch32v0.h`
- ⬜ `lib/ch32v0/` 下 15 个外设的 `.c` 与 `common/*.c`
- ⬜ `lib/ch32v0/Makefile` 中已注释的 `OBJS` 逐条启用
- ⬜ `examples/ch32v003/{blink,usart_echo}`
- ⬜ `tests/ch32v003-generic` 冒烟工程

CH58x 侧（P3）同样未开始：`rwa.c` `clk.c` `sys.c` `gpio.c` `uart.c` 等。

---

## 阻塞项 / 待决

**当前无阻塞项。**

| # | 项 | 类型 | 影响 | 处理 |
|---|---|---|---|---|
| B1 | 无硬件（WCH-Link + CH32V003/CH582 板） | 环境 | P4 硬件在环无法在本机完成 | 不阻塞 P0–P3 |
| B2 | 本工作区的虚拟化文件系统对子目录路径的 `..` 解析不正确 | 环境 | 已规避 | 构建系统全部改用 `$(abspath)` 绝对路径 |
| B3 | 工具链 multilib 列表不含 `rv32ec` 字样，但 `rv32e/ilp32e/libgcc.a` 存在且可用 | 环境 | 无 | 已在 `mk/gcc-config.mk` 做能力探测 |

### 已知问题（不阻塞）

| # | 问题 | 说明 |
|---|---|---|
| K1 | `make stylecheck` 对 `volatile` MMIO、`do {} while(1)` 断言、复杂宏报 warning/error | 这些是 `checkpatch.pl` 对内核风格之外的合理用法的固有误报；libopencm3 自身的 `cm3/common.h` 同样报 3 errors / 7 warnings。定位为**参考工具**而非门禁，已在 `HACKING` 说明 |
| K2 | `.vector` 段设为只读数据（`rodata` PHDR），ISR 目标仍在 `.text` | 与 WCH EVT 一致；两者都在 flash。运行期改表（`OPENWCH_VECTOR_SET`）只在表位于 RAM 时生效，已在 `vector.h` 注明 |
| K3 | P0 的链接脚本冒烟测试尚未纳入 `make` 目标 | 目前是人工执行的验证步骤。P4 计划加入 `tests/` 作为自动化用例 |

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
6. **P3** 同法推进 CH58x（先 `rwa.c`，再 `clk.c`，其余依赖它）。
7. **P4** 把链接冒烟测试固化为 `tests/` 用例；补 Doxygen、CI、`NOTICE`。

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

---

## 变更日志

| 日期/轮次 | 变更 |
|---|---|
| 初始轮 | 创建 `project.md`、`phase.md`、`status.md`、`AGENTS.md`；完成四工程勘察与设计决策 |
| 第 2 轮 | 用户确认 Q1–Q4；实测工具链能力；完成 **P0**（骨架 + `mk/` + `scripts/` + `ld/` + 根 Makefile）与 **P1**（`qingke/` 核心层）；两族归档构建成功并通过链接冒烟测试；修正 8 项设计问题（见 `phase.md`） |
