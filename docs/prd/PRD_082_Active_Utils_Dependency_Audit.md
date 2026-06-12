# PRD-082 - Active Utils Dependency Audit

## 背景

PRD-081 已经把 CMake 引用口径拆成 raw / declared / active，并确认当前主 GUI 目标仍有约 1690 个 active CMake refs，其中 `utils` 约 844 个。

但仅知道 active 目录还不够。下一步如果要继续降低编译面，必须先知道每个 active utils 目录是否被其它模块 include，否则容易误删或误过滤。

## 目标

1. 在 `source-tree-audit.ps1` 报告中新增 active utils 依赖证据。
2. 对每个 active utils 目录统计：
   - 目录内源码文件数量。
   - active CMake refs 数量。
   - 被目录外文件 include 的次数。
   - 目录外 include 来源模块数量。
3. 标记低风险候选：
   - active CMake refs 存在；
   - external include count 为 0；
   - 目录名看起来不是 canonical 基础能力。
4. 输出可复查的路径和计数，为后续 CMake 拆分或冻结 PRD 提供依据。

## 非目标

1. 不删除源码。
2. 不修改 `CMakeLists.txt`。
3. 不改变构建目标。
4. 不根据审计结果直接移除任何目录。
5. 不启动 BATCH 并行。

## canonical utils 目录

下列目录默认视为当前产品基础能力，不作为低风险候选：

- `checksum`
- `converter`
- `crypto`
- `data`
- `export`
- `log`
- `packet`
- `perf`
- `pipeline`
- `settings`
- `timestamp`

其它目录也不能仅凭名字删除，仍需结合 include 证据、CMake 引用和构建验证处理。

## 验收标准

1. `source-tree-latest.md` 包含 `Active Utils External Include Evidence` 表。
2. 表中至少包含 Path、Files、Active CMake refs、External include refs、External modules。
3. 报告包含 `Low-Risk Active Utils Split Candidates` 表。
4. 候选表只作为后续 PRD 输入，不允许当前阶段修改 CMake。
5. `tools/doctor.ps1` 继续通过。

## 失败条件

1. 审计脚本误写 `src/` 或 `CMakeLists.txt`。
2. include 统计把同目录内部 include 当成外部依赖。
3. 输出报告丢失 active CMake refs、Serial Station 缺口或旧 UART 证据。
4. 候选表被描述为可以直接删除。
