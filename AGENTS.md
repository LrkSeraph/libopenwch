# AGENTS.md — libopenwch

本文件是本仓库的**入口索引**，面向在此仓库工作的 AI Agent 与人类贡献者。

---

## 1. 必读文档（按顺序）

| 文档 | 作用 | 何时读 |
|---|---|---|
| **`project.md`** | **总体设计**：项目定位、需求、参考工程角色、目录结构、核心层设计（`qingke/` ↔ libopencm3 的 `cm3/`）、API 命名规范与 WCH EVT 迁移对照表、构建系统改造点（ch32fun 式工具链适配）、ADR、扩展路线、验证策略 | **开工前必读全文** |
| **`phase.md`** | **阶段性目标**：P0–P5 六阶段的交付物、任务复选框、每阶段验收标准、里程碑 Reviewer Checklist | 每次认领任务前读对应阶段 |
| **`status.md`** | **当前完成情况**：阶段进度快照、已完成/未开始清单、**阻塞项与待决问题**、下一步动作、验证记录、变更日志 | **每次会话开始时读**，**每次会话结束前更新** |
| `README.md` | 面向用户的快速上手（工具链安装、构建、示例） | 用户文档变更时 |

> **一句话**：`project.md` 说"要做成什么样"，`phase.md` 说"分几步做、每步怎么验收"，
> `status.md` 说"现在做到哪了"。

---

## 2. 项目一句话描述

**libopenwch** = 面向 WCH（沁恒）RISC-V MCU 的驱动库，**在 API 风格、目录结构、构建系统、
代码规范四个维度对齐 libopencm3**，并为 `riscv64-unknown-elf` 工具链做 ch32fun 式适配。
一期实现 **CH32V003** 与 **CH582/CH583**。

---

## 3. 关键约束（违反即为 bug）

1. **非驼峰命名**。所有公开函数 `periph_verb_noun()`，如 `gpio_set_mode()`、`usart_set_baudrate()`。
   **禁止**出现 `GPIO_Init()`、`GPIOA_ModeCfg()`、`RCC_GetClocksFreq()` 这类 WCH EVT 驼峰名。
   常量全大写 `PERIPH_REG_BIT`。类型小写 `_t`。
2. **不复制 WCH EVT 代码**。`ch32v003-main/`、`ch583-main/` 只作为寄存器地址/位定义/操作时序的
   *事实参考*，实现必须重写。许可洁净是硬要求（见 `project.md` §8）。
3. **构建系统与 libopencm3 同构**。`TARGETS`、`lib/` 递归、`mk/` 组件、`ld/devices.data` +
   `scripts/genlink.py`、`irq.json` → `nvic.h` 生成，这些机制不得另起炉灶。
4. **一库一族**。`rv32ec`（CH32V003）与 `rv32imac`（CH58x）的 `.o` **不得**混入同一归档。
   库名 `lib/libopenwch_<family>.a`。
5. **RWA 寄存器必须先解锁**。CH58x 的任何 RWA 寄存器写入必须经 `rwa_unlock()`/安全访问封装，
   不得裸写。
6. **寄存器宏命名贴近 WCH 手册**（`HACKING` 要求），例如 `USART_CTLR1_UE`、`GPIO_CFGLR`。
7. **提交前跑 `make stylecheck`**（`scripts/checkpatch.pl`，Linux 内核风格）。

---

## 4. 参考工程位置与用途

工作区根目录 `/home/lrk/Projects/incubator_workspace/`：

| 路径 | 用途 | 读取方式 |
|---|---|---|
| `libopencm3/` | **主模板**：结构、构建、规范、文档机制 | 可读取、可移植脚本与许可文件（LGPL/GPL 兼容），**不要**整体复制 ARM 专有代码 |
| `ch32fun-master/` | **工具链适配参考**：`riscv64-unknown-elf` 探测、`-march` 表、`zmmul` 特判、启动/链接细节 | 可参考；MIT 许可 |
| `ch32v003-main/` | CH32V003 寄存器/时序**事实来源**（WCH EVT） | **只读参考**，不得复制代码 |
| `ch583-main/` | CH582/583 寄存器/时序**事实来源**（WCH EVT） | **只读参考**，不得复制代码 |

---

## 5. 常用命令

```sh
# 工具链（若未安装；需用户授权）
sudo apt-get install -y gcc-riscv64-unknown-elf

# 构建一期全部库
cd libopenwch && make

# 只构建某一族
make TARGETS=ch32v/003
make TARGETS=ch5xx/58x
make TARGETS="ch32v/003 ch5xx/58x"

# 指定工具链前缀
make PREFIX=riscv64-unknown-elf-

# 链接脚本生成冒烟测试
make genlinktests

# 代码规范
make stylecheck
make styleclean

# 查看可用目标
make list-targets

# 清理
make clean

# 文档
make html        # 或 make -C doc html

# 示例
cd examples/ch32v003/blink && make OPENWCH_DIR=../../.. DEVICE=ch32v003f4p6
```

---

## 6. Agent 工作流约定

1. **开始任何工作前**：读 `status.md` 了解当前阶段与阻塞项；读 `phase.md` 中对应阶段的任务清单。
2. **认领任务**：只做 `phase.md` 中当前阶段的任务，不跳阶段。若必须跳，先在 `status.md`
   记录理由。
3. **每完成一个任务**：立刻在 `phase.md` 勾选，并更新 `status.md` 的进度与「已完成」清单。
4. **遇到阻塞**：写入 `status.md` 的「阻塞项」表，不要静默跳过。
5. **新增公开 API**：必须在 `project.md` §5.1 的「WCH EVT → libopenwch 对照表」中登记命名。
6. **改动构建系统**：同步更新 `project.md` §6、`phase.md` P0 对应任务、`mk/README`。
7. **每次会话结束**：刷新 `status.md` 的「下一步」与「变更日志」。
8. **验证优先**：任何"完成"的声明都必须附可复现的命令与输出（见 `status.md` 的「验证记录」表）。
   **不要**在未编译/未运行的情况下声称代码可用。

---

## 7. 目录速查

```
libopenwch/
├── project.md  phase.md  status.md  AGENTS.md    ← 四份治理文档
├── Makefile                                       ← 根构建（TARGETS 递归）
├── mk/          gcc-config / gcc-rules / genlink-config / genlink-rules
├── scripts/     genlink.py / irq2nvic_h / genlinktest.sh / checkpatch.pl
├── ld/          devices.data / linker.ld.S / tests/
├── include/libopenwch/
│   ├── qingke/      ← 核心层（≈ libopencm3 的 cm3/）
│   ├── ch32v0/common/   ch32v003/   ← V 系列（RV32EC）
│   └── ch5xx/common/    ch582/      ← 5xx/BLE 系列（RV32IMAC）
├── lib/
│   ├── Makefile.include
│   ├── qingke/     ← vector.c / nvic.c / assert.c / systick.c ...
│   ├── ch32v/      Makefile.include + common/ + 003/
│   └── ch5xx/      Makefile.include + common/ + 58x/
├── doc/  tests/  examples/
```

---

## 8. 状态图例

`status.md` 与 `phase.md` 中使用：

- ✅ 完成
- 🔄 进行中
- ⬜ 未开始
- ⚠️ 部分完成 / 有风险
- ⛔ 阻塞
- 📄 仅文档
