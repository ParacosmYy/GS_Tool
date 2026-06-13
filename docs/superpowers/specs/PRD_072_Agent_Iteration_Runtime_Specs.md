# PRD-072 - Agent Iteration Runtime Specs

## 1. 目标

把 PRD-072 的 Specs / GO / BATCH / LOOP 机制做成仓库内可执行、可审查的最小闭环：

- Specs 有模板，也有本 PRD 的实例。
- GO 执行器有 JSON 配置、20 轮和 30 分钟安全刹车。
- BATCH 有人审前置和 3/6 并行上限。
- LOOP 有 Doctor / Debug / Simplify 分层路由。

## 2. 非目标

- 不修改 EmbedDebug C++ 产品功能。
- 不接入远程 LLM API。
- 不把 Go 工具接入 CMake。
- 不创建第二构建目录。
- 不并行写共享入口文件。

## 3. 必读约束

- `CLAUDE.md`
- `docs/constraints/01-project-overview.md`
- `docs/constraints/02-workflow.md`
- `docs/constraints/04-coding-standard.md`
- `docs/constraints/07-directory-structure.md`
- `docs/prd/PRD_072_Agent_Iteration_Runtime.md`

## 4. 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| PRD | `docs/prd/PRD_072_Agent_Iteration_Runtime.md` | 修改 |
| Specs | `docs/superpowers/specs/` | 新增 / 修改 |
| 协议文档 | `docs/superpowers/BATCH_PROTOCOL.md`, `docs/superpowers/LOOP_PROTOCOL.md` | 修改 |
| Go 工具 | `tools/agent-loop/` | 新增 / 修改 |
| 约束索引 | `docs/constraints/02-workflow.md`, `docs/constraints/07-directory-structure.md` | 修改 |
| 禁止修改 | `src/`, `CMakeLists.txt`, `EmbedDebug.bat`, `tools/launch_embeddebug.ps1` | 禁止，除非进入新的 PRD 或构建线 |

## 5. 验收标准

- [ ] Specs 模板和 PRD-072 实例同时存在。
- [ ] GO 执行器源码存在，并支持 `execute`、`check`、`fix`。
- [ ] GO 执行器拒绝超过 20 轮或 30 分钟的配置。
- [ ] GO 执行器在未配置 `execute` 和 `check` 时失败。
- [ ] BATCH 文档明确 5-30 子任务和人工审查。
- [ ] LOOP 文档明确 Doctor / Debug / Simplify。
- [ ] Qt 主程序构建仍通过。
- [ ] `EmbedDebug.bat` 仍能启动 `build/EmbedDebug.exe`。

## 6. 失败条件

- 工具默认无限循环。
- 文档鼓励未经审查的并行写代码。
- 新增工具影响 CMake 或产品启动。
- 示例配置写入或引用 `build2/`、`build-debug/`、`build-release/`。
- 无法说明未运行 Go 测试的具体原因。

## 7. GO 配置

当前样例配置以 `tools/agent-loop/sample.embeddebug.json` 为准。该配置不直接把 `.\\EmbedDebug.bat` 放进 `check`，而是先跑 Doctor，再用启动探针验证并清理本轮新启动的进程：

```json
{
  "workdir": "../..",
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

运行方式：

```powershell
Push-Location .\tools\agent-loop
go run . -config .\sample.embeddebug.json -dry-run
go run . -config .\sample.embeddebug.json
Pop-Location
```

## 8. BATCH 判定

- 是否需要 BATCH：否。
- 原因：本轮只修改 PRD-072 相关文档和单个 Go 工具目录，文件范围小且共享约定未稳定到可并行写。
- 并行度上限：1。
- 人工审查状态：不需要并行审查。

## 9. LOOP 路由

- Doctor：用于验证 Qt 构建和 `EmbedDebug.bat`。
- Debug：用于处理 Go 配置解析或执行器错误。
- Simplify：用于防止工具逻辑膨胀；Go 工具暂保持单命令入口和简单 JSON 配置。

## 10. 当前验证记录

| 项 | 结果 |
|----|------|
| Specs 模板 | 已存在：`docs/superpowers/specs/SPECS_TEMPLATE.md` |
| PRD-072 实例 | 已存在：`docs/superpowers/specs/PRD_072_Agent_Iteration_Runtime_Specs.md` |
| GO 源码 | 已存在：`tools/agent-loop/main.go` |
| GO 测试 | 已存在：`tools/agent-loop/main_test.go`；`go version` 失败，当前 PATH 无 `go.exe`，因此未运行 |
| GO 验证入口 | `powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\agent-loop\verify.ps1` 输出 `agent_loop_go=UNAVAILABLE reason=go.exe_not_on_PATH`，退出码 `2` |
| BATCH 文档 | 已存在：`docs/superpowers/BATCH_PROTOCOL.md` |
| LOOP 文档 | 已存在：`docs/superpowers/LOOP_PROTOCOL.md` |
| Doctor | `powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1` 通过：0 failure，1 warning（`go.exe` 不在 PATH） |
| 启动探针 | `powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\verify_embeddebug_launch.ps1` 通过：`launch_result=PASSED` |
| 最近收口 | `3e9343908 Agent Runtime: 收口GO验证命令` |

## 11. 剩余验证缺口

当前仓库已经具备 Specs / GO / BATCH / LOOP 的文档和源码闭环，但本机缺少 Go 工具链，无法执行：

```powershell
Push-Location .\tools\agent-loop; go test .; Pop-Location
```

也可以运行统一入口：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\agent-loop\verify.ps1
```

在 `go.exe` 可用前，PRD-072 只能声明“源码、测试和验证入口已落地，Doctor 与启动探针已验证”，不能声明 GO 执行器单测已通过。
