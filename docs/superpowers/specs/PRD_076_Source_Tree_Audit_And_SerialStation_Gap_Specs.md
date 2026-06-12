# Specs - PRD-076 Source Tree Audit And SerialStation Gap

## 目标

新增一个只读审计入口，把“文件太多、重复轮子、UART 配置缺口”转成可复查的 Markdown 报告。

## 非目标

1. 不删除任何源码。
2. 不移动任何源码。
3. 不修改 CMake。
4. 不实现新的 UART UI。
5. 不启动并行子 Agent 写代码。

## 约束

1. 必须遵守 `CLAUDE.md`、`01-project-overview.md`、`02-workflow.md`、`03-architecture.md`、`04-coding-standard.md`、`07-directory-structure.md` 和 `docs/serial_station_architecture.md`。
2. 审计脚本必须只读源码和构建配置。
3. 输出报告只能写入 `docs/reviews/simplify/`。
4. 不允许创建第二构建目录。
5. 不允许把 build 产物纳入报告输入。

## 产物

1. `docs/prd/PRD_076_Source_Tree_Slimming_And_SerialStation_Minimal_Uart.md`
2. `docs/superpowers/specs/PRD_076_Source_Tree_Audit_And_SerialStation_Gap_Specs.md`
3. `tools/source-tree-audit.ps1`
4. `docs/reviews/simplify/source-tree-latest.md`

## 验证命令

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\source-tree-audit.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\source-tree-audit.ps1 -OutFile .\docs\reviews\simplify\source-tree-latest.md
```

## 失败条件

1. 脚本误写 `src/`、`CMakeLists.txt` 或资源目录。
2. 输出路径可逃逸到 `docs/reviews/simplify/` 之外。
3. 报告不能说明旧 UART 配置存在证据和新 Serial Station 缺口。
4. 报告只给结论，没有可复查的文件路径或计数。

## 后续 BATCH 入口

本 Specs 只允许单 Agent 串行完成。后续如进入清理，可以基于报告拆成 5-30 个子任务，再按 `docs/superpowers/BATCH_PROTOCOL.md` 申请 3 个或 6 个子 Agent 并行。
