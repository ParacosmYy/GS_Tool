# PRD-091 GO Loop 启动验证闭环

## 1. 背景

仓库已有 `tools/agent-loop` Go 执行循环，用于 Specs 驱动的 execute → check → fix 迭代。
但当前机器 PATH 中没有 `go.exe`，doctor 会给出 `go.exe is not available on PATH` 警告。
同时 `sample.embeddebug.json` 直接把 `.\\EmbedDebug.bat` 作为 check 命令，容易启动 GUI 后长期占用流程，
不适合作为自动化 GO Loop 的收口检查。

本轮目标是补齐 GO Loop 的启动验证闭环，让工具在可用 Go 环境下能自动检查 bat 启动，并在缺少 Go 时给出明确诊断。

## 2. 目标

- 新增可复用的 bat 启动探针脚本，自动启动 `EmbedDebug.bat`、等待 `EmbedDebug.exe` 进程出现、输出结果并关闭新进程。
- 更新 `tools/agent-loop/sample.embeddebug.json`，用启动探针替代直接执行 bat。
- 更新 `tools/agent-loop/README.md`，说明缺少 `go.exe` 时如何诊断，以及如何手动执行等价命令。
- 给 GO Loop 增加更完整的 Go 单元测试，覆盖命令执行成功、check 失败无 fix、fix 后通过等循环行为。
- 不改 CMake、不改产品源码、不改 `EmbedDebug.bat` 启动入口。

## 3. 非目标

- 本轮不安装 Go 工具链。
- 本轮不绕过 Go 引擎改写成 PowerShell 执行引擎。
- 本轮不修改 `tools/doctor.ps1` 的 warning 口径。
- 本轮不创建或引用第二构建目录。
- 本轮不改 Serial Station 产品代码。

## 4. 范围

| 类型 | 路径 | 动作 |
|------|------|------|
| 工具 | `tools/verify_embeddebug_launch.ps1` | 新增 |
| 工具 | `tools/agent-loop/main.go` | 必要增强 |
| 测试 | `tools/agent-loop/main_test.go` | 补循环测试 |
| 文档 | `tools/agent-loop/README.md` | 更新 |
| 样例 | `tools/agent-loop/sample.embeddebug.json` | 更新 |
| 追踪 | `docs/tracking/SCORE_TRACKING.md` | 记分 |

## 5. 验收标准

- `powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\verify_embeddebug_launch.ps1` 可自动验证 bat 启动并关闭新进程。
- `sample.embeddebug.json` 的 check 使用 `tools\\verify_embeddebug_launch.ps1`，不直接调用 `.\\EmbedDebug.bat`。
- GO Loop 保持 `max_rounds <= 20`、`max_minutes <= 30` 的安全刹车。
- 新增 Go 单测覆盖 fix 后通过的循环路径。
- 如果当前机器没有 `go.exe`，README 必须写明 doctor warning 的含义和手动替代命令。
- `tools/doctor.ps1`、bat 启动探针和工作区状态验证通过。

## 6. 失败条件

- 工具创建或引用 `build2/`、`build-debug/`、`build-release/` 等平行构建目录。
- 启动探针留下 `EmbedDebug.exe` 残留进程。
- GO Loop sample 仍直接执行 GUI bat。
- 缺少 Go 时文档只写“安装 Go”，没有给出可执行的手动验证替代命令。
