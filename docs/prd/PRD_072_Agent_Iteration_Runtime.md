# PRD-072 Agent Iteration Runtime

## 1. 目标

建立一套可执行的 Agent 迭代机制，把用户提出的 Specs、GO、BATCH、LOOP 固化为仓库内可审查、可验证、可暂停的流程。

本 PRD 只定义执行机制和工具骨架，不新增 EmbedDebug 产品功能，不修改 C++ 业务代码。

## 2. 背景

当前仓库已经有约束文档、PRD、superpowers plans 和 bat 启动链路，但缺少一个统一机制来约束 Agent 在长任务中的执行方式：

- Specs：先写目标、约束、验收标准和失败条件，再执行。
- GO：用可复现命令执行、检查、修正，避免靠口头状态推进。
- BATCH：大任务拆成 5-30 个子任务，人工审查拆分方案后才允许并行。
- LOOP：按 Doctor、Debug、Simplify 分层处理问题，不让失败继续扩散。

## 3. 范围

### 3.1 本次包含

- 新增 Specs 模板和写作规则。
- 新增 BATCH 方案模板和并行边界。
- 新增 LOOP 监控分层说明。
- 新增 Go 版 GO 执行器源码，支持执行、检查、修正循环。
- 在工作流约束中登记这套机制。

### 3.2 本次不包含

- 不接入任何远程 LLM API。
- 不自动修改 C++ 源码。
- 不新增 CMake 目标。
- 不创建第二构建目录。
- 不替代 `EmbedDebug.bat` 启动入口。

## 4. 设计约束

- 所有产品代码迭代仍必须遵守 `CLAUDE.md` 和 `docs/constraints/`。
- 新功能代码仍必须先有 PRD。
- 并行子 Agent 必须先经过 BATCH 方案审查，未审查前只允许只读分析、文档和复现。
- GO 执行器默认最多 20 轮或 30 分钟，任一条件触发即停止。
- 所有命令默认在仓库根目录运行，不得写入 `build2/`、`build-debug/` 等平行构建目录。

## 5. 用户故事

1. 作为开发者，我希望每次长任务先有 Specs，避免 Agent 边做边改目标。
2. 作为维护者，我希望大任务并行前先看到 BATCH 拆分方案，确认文件范围不会冲突。
3. 作为验证者，我希望 GO 执行器能重复执行检查命令，并在失败时执行修正命令。
4. 作为项目负责人，我希望 LOOP 把系统体检、Bug 追踪和简化清理分开处理。

## 6. 验收标准

- `docs/superpowers/specs/SPECS_TEMPLATE.md` 存在，包含目标、非目标、约束、验收、失败条件、验证命令。
- `docs/superpowers/specs/PRD_072_Agent_Iteration_Runtime_Specs.md` 存在，作为本 PRD 的执行实例。
- `docs/superpowers/BATCH_PROTOCOL.md` 存在，明确 5-30 子任务、人审、3/6 并行上限。
- `docs/superpowers/LOOP_PROTOCOL.md` 存在，明确 Doctor、Debug、Simplify 三层处理。
- `tools/doctor.ps1` 存在，作为 LOOP Doctor 的本地系统体检入口。
- `tools/debug-trace.ps1` 存在，作为 LOOP Debug 的本地问题追踪入口。
- `tools/simplify-scan.ps1` 存在，作为 LOOP Simplify 的本地只读扫描入口。
- `tools/agent-loop/go.mod` 和 `tools/agent-loop/main.go` 存在。
- `tools/agent-loop/main_test.go` 覆盖配置默认值、安全刹车上限、空命令拒绝和命令保留。
- GO 执行器支持 JSON 配置，包含 `execute`、`check`、`fix` 三段命令。
- GO 执行器包含 20 轮和 30 分钟安全刹车。
- `docs/constraints/02-workflow.md` 明确 Specs -> GO -> BATCH -> LOOP 的执行关系。
- Qt 主程序仍可构建，`EmbedDebug.bat` 仍可启动。

## 7. 失败条件

- 工具或文档绕过现有 PRD / 架构 / UI / 目录约束。
- GO 执行器默认无限循环或无时间刹车。
- BATCH 文档鼓励未经审查的并行写代码。
- 新增 Go 工具被接入 CMake 或影响 EmbedDebug 产品构建。
- 修改后 `EmbedDebug.bat` 无法启动。

## 8. 验证命令

```powershell
cmake --build .\build --config Release --parallel 4
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\verify_embeddebug_launch.ps1
Push-Location .\tools\agent-loop; go test .; Pop-Location
```

`verify_embeddebug_launch.ps1` 是自动化启动探针：它通过 `EmbedDebug.bat` 启动应用，确认出现新的 `EmbedDebug.exe` 进程，然后只关闭本次新启动的进程，避免 GO Loop 被 GUI 长时间占用。

如果当前机器没有 Go 工具链，必须记录 `go version` 的失败原因，并保留 Doctor、Qt 构建和启动探针验证结果。
