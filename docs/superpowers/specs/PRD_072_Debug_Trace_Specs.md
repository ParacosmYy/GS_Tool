# PRD-072 - Debug Trace Specs

## 1. 目标

把 LOOP Protocol 中的 Debug 层落成可运行入口，用于标准化记录 Bug 复现、错误摘要、影响范围、下一步修复动作和回归命令。

## 2. 非目标

- 不自动修改源码。
- 不自动运行修复命令。
- 不创建第二构建目录。
- 不替代 Doctor 体检。
- 不把普通审计报告混入产品代码。

## 3. 必读约束

- `CLAUDE.md`
- `docs/constraints/01-project-overview.md`
- `docs/constraints/02-workflow.md`
- `docs/constraints/04-coding-standard.md`
- `docs/constraints/07-directory-structure.md`
- `docs/superpowers/LOOP_PROTOCOL.md`

## 4. 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| Debug 脚本 | `tools/debug-trace.ps1` | 新增 / 修改 |
| Debug 报告 | `docs/reviews/debug/` | 新增 |
| Specs | `docs/superpowers/specs/PRD_072_Debug_Trace_Specs.md` | 新增 / 修改 |
| LOOP 文档 | `docs/superpowers/LOOP_PROTOCOL.md` | 修改 |
| 目录文档 | `docs/constraints/07-directory-structure.md` | 修改 |
| PRD-072 | `docs/prd/PRD_072_Agent_Iteration_Runtime.md` | 修改 |
| 禁止修改 | `src/`, `CMakeLists.txt`, `EmbedDebug.bat` | 禁止，除非进入新的 bugfix 或构建线任务 |

## 5. 验收标准

- [ ] `tools/debug-trace.ps1` 存在。
- [ ] 脚本能接收标题、严重级别、复现命令、错误摘要、影响范围、候选文件和回归命令。
- [ ] 未传 `-OutFile` 时只输出 Markdown，不写文件。
- [ ] 传 `-OutFile` 时只允许写入仓库内 `docs/reviews/debug/`。
- [ ] 脚本生成的报告包含 Doctor、Debug、Simplify 下一步路由。
- [ ] LOOP 文档登记 Debug 本地入口。
- [ ] Qt 构建和 `EmbedDebug.bat` 启动仍可验证通过。

## 6. 失败条件

- 脚本默认改源码或执行修复命令。
- 脚本允许写入仓库外路径。
- 脚本允许写入 `build/`、`src/` 或任意生产代码目录。
- Debug 报告缺少复现命令或回归命令。

## 7. GO 配置

```json
{
  "max_rounds": 20,
  "max_minutes": 30,
  "execute": [
    "powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\debug-trace.ps1 -Title \"示例问题\" -Severity P2 -ReproCommand \"cmake --build .\\build --config Release --parallel 4\" -ErrorSummary \"示例错误\" -ImpactScope \"构建线\" -RegressionCommand \"cmake --build .\\build --config Release --parallel 4\""
  ],
  "check": [
    "powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\doctor.ps1"
  ],
  "fix": []
}
```

## 8. BATCH 判定

- 是否需要 BATCH：否。
- 原因：本轮只新增一个 Debug 脚本和文档登记，单 Agent 串行更清晰。
- 并行度上限：1。

## 9. LOOP 路由

- Doctor：先确认环境和启动链路。
- Debug：本脚本记录复现、错误和回归路径。
- Simplify：如果候选文件显示问题来自重复实现或过大文件，转入 Simplify。
