# PRD-091 - GO Loop 启动验证闭环

## 1. 标题

`PRD-091 - GO Loop 启动验证闭环`

## 2. 目标

- 让 `tools/agent-loop` 的样例配置具备自动化 bat 启动验证能力。
- 缺少 `go.exe` 时，文档和 doctor 结论能给出明确替代验证路径。
- 补强 GO Loop 自身循环测试，证明 execute/check/fix 的关键路径可用。

## 3. 非目标

- 不安装 Go。
- 不把 GO Loop 改写为 PowerShell 引擎。
- 不修改产品 C++ 代码。
- 不修改 `EmbedDebug.bat`。
- 不创建任何平行构建目录。

## 4. 必读约束

- `CLAUDE.md`
- `docs/constraints/01-project-overview.md`
- `docs/constraints/02-workflow.md`
- `docs/constraints/03-architecture.md`
- `docs/constraints/04-coding-standard.md`
- `docs/constraints/06-git-commit.md`
- `docs/constraints/07-directory-structure.md`
- `C:/Users/LWH/.agents/skills/mcu/references/docs/part1_code_style.md`
- `C:/Users/LWH/.agents/skills/mcu/references/docs/checklist.md`

## 5. 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| 工具 | `tools/verify_embeddebug_launch.ps1` | 新增 |
| 工具 | `tools/agent-loop/main.go` | 必要增强 |
| 测试 | `tools/agent-loop/main_test.go` | 修改 |
| 文档 | `tools/agent-loop/README.md` | 修改 |
| 样例 | `tools/agent-loop/sample.embeddebug.json` | 修改 |
| 追踪 | `docs/tracking/SCORE_TRACKING.md` | 修改 |
| 禁止修改 | `EmbedDebug.bat` | 禁止 |
| 禁止修改 | `CMakeLists.txt` | 禁止 |
| 禁止修改 | `src/` | 禁止 |

## 6. 启动探针要求

- 脚本从仓库根目录运行，默认路径为当前目录。
- 启动前记录已有 `EmbedDebug` 进程 ID。
- 通过 `cmd.exe /c EmbedDebug.bat` 启动。
- 最多等待 20 秒寻找新 `EmbedDebug.exe` 进程。
- 成功时输出 `launch_result=PASSED pid=<pid>`。
- 失败时输出 `launch_result=FAILED` 并返回非 0。
- 成功或失败后都不得留下本脚本新启动的 `EmbedDebug.exe` 进程。

## 7. GO Loop 测试要求

- 覆盖配置默认值。
- 覆盖 `max_rounds > 20` 和 `max_minutes > 30` 拒绝。
- 覆盖 check 一次成功。
- 覆盖 check 失败且没有 fix 时返回错误。
- 覆盖 fix 后下一轮 check 成功。

## 8. 验收标准

- [ ] `powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\verify_embeddebug_launch.ps1` 通过。
- [ ] `tools/agent-loop/sample.embeddebug.json` 不再直接调用 `.\\EmbedDebug.bat`。
- [ ] `tools/agent-loop/README.md` 包含缺少 Go 时的手动替代验证命令。
- [ ] `go test ./...` 在有 Go 环境时可覆盖新增循环测试。
- [ ] 当前无 Go 环境时，必须记录 `where.exe go` / `go version` 的失败证据，并运行手动替代验证。
- [ ] `tools/doctor.ps1` 通过，允许保留既有 go.exe warning。
- [ ] `EmbedDebug.bat` 或启动探针验证通过。
- [ ] 工作区不包含 build 产物和平行构建目录。

## 9. 失败条件

- sample 配置启动 GUI 后不退出。
- 启动探针误杀用户已有 `EmbedDebug` 进程。
- 工具依赖 `build2/` 或其他平行目录。
- README 没有说明当前无 Go 的验证替代路径。

## 10. GO 配置

```json
{
  "max_rounds": 20,
  "max_minutes": 30,
  "execute": [
    "powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\doctor.ps1"
  ],
  "check": [
    "powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\verify_embeddebug_launch.ps1"
  ],
  "fix": []
}
```

## 11. BATCH 判定

- 是否需要 BATCH：否。
- 子任务数量：不适用。
- 并行度上限：1。
- 人工审查状态：不适用，本轮触碰共享工具入口，串行更稳。

## 12. LOOP 路由

- Doctor：环境变量、Go、Qt、build 产物异常时先跑 `tools/doctor.ps1`。
- Debug：启动探针失败时检查 `tools/launch_embeddebug.ps1` 和进程检测。
- Simplify：如果 GO Loop 增强开始扩张为通用 CI 系统，收缩回本轮启动验证目标。
