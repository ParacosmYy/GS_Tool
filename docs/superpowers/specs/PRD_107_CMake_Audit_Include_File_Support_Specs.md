# PRD-107 - CMake Audit Include File Support

## 1. 标题

`PRD-107 - CMake Audit Include File Support`

## 2. 目标

- 修复 PRD-106 后审计工具漏读 `cmake/EmbedDebugSources.cmake` 的问题。
- `tools/project-audit/project_audit.py` 与 `tools/source-tree-audit.ps1` 都能把 `cmake/*.cmake` 片段纳入 CMake 引用统计。
- 为 Python 工具补单元测试。
- 本轮三轴目标：工程 `E3 -> E4`，用户不提升，设备不提升。

## 3. 非目标

- 不修改生产 C++。
- 不修改根 `CMakeLists.txt`。
- 不改变构建、启动或 UI 行为。
- 不删除、移动源码。
- 不提升 Serial Station 的用户状态或设备状态。

## 4. 约束

- 遵守 `CLAUDE.md`、`docs/constraints/01-project-overview.md`、`docs/constraints/02-workflow.md`、`docs/constraints/04-coding-standard.md`、`docs/constraints/07-directory-structure.md`。
- 构建目录只能是 `build/`。
- 报告只能写入 `docs/reviews/simplify/`。
- 用户已有改动不得回退。

## 5. 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| 工具 | `tools/project-audit/project_audit.py` | 修改 |
| 工具 | `tools/source-tree-audit.ps1` | 修改 |
| 测试 | `tools/test_embeddebug_tools.py` | 修改 |
| 报告 | `docs/reviews/simplify/source-tree-latest.md` | 刷新 |
| 文档 | `docs/prd/PRD_107_CMake_Audit_Include_File_Support.md` | 新增 |
| 文档 | `docs/superpowers/specs/PRD_107_CMake_Audit_Include_File_Support_Specs.md` | 新增 |
| 评分 | `docs/tracking/SCORE_TRACKING.md` | 修改 |
| 禁止修改 | `src/` | 禁止 |
| 禁止修改 | `CMakeLists.txt` | 禁止 |

## 6. 验收标准

- [ ] `python tools/project-audit/project_audit.py --limit 5` 的 `listed code entries` 大于 0。
- [ ] `python tools/test_embeddebug_tools.py` 通过。
- [ ] `powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\source-tree-audit.ps1 -OutFile .\docs\reviews\simplify\source-tree-latest.md` 通过。
- [ ] `docs/reviews/simplify/source-tree-latest.md` 中 `Active CMake source references` 大于 0。
- [ ] `docs/reviews/simplify/source-tree-latest.md` 中 `CMake refs under src/apps/serial_station` 大于 0。
- [ ] 没有新增第二构建目录。
- [ ] 本轮结束有 commit。

## 7. 失败条件

- 审计工具仍然只读根 CMake。
- 报告路径越过 `docs/reviews/simplify/`。
- 工具测试失败且无法定位到本轮改动。
- 需要改生产源码或根 CMake 才能收口。

## 8. GO 配置

```json
{
  "max_rounds": 20,
  "max_minutes": 30,
  "execute": [
    "python tools/test_embeddebug_tools.py",
    "python tools/project-audit/project_audit.py --limit 5",
    "powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\source-tree-audit.ps1 -OutFile .\\docs\\reviews\\simplify\\source-tree-latest.md"
  ],
  "check": [
    "Select-String -Path .\\docs\\reviews\\simplify\\source-tree-latest.md -Pattern \"Active CMake source references\",\"CMake refs under src/apps/serial_station\"",
    "git status --short"
  ],
  "fix": []
}
```

## 9. BATCH 判定

- 是否需要 BATCH：否。
- 并行度上限：1。
- 共享文件锁：审计工具和报告。

## 10. LOOP 路由

| 失败表现 | 路由 | 必须产出 |
|----------|------|----------|
| Python/PowerShell 环境异常 | Doctor | 命令输出摘要 |
| CMake 引用统计仍为 0 | Debug | 复现命令和解析修复 |
| 报告过慢或噪声过高 | Simplify | 下一轮审计工具瘦身计划 |

## 11. 收口记录

| 项 | 结果 |
|----|------|
| 工程状态 | `E3 -> E4`，工具测试与审计报告刷新通过 |
| 用户状态 | 不提升，产品行为无变化 |
| 设备状态 | 不提升，真实硬件未验证 |
| 验证命令 | `uv run test-embeddebug-tools`；`python tools/project-audit/project_audit.py --limit 5`；`powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\source-tree-audit.ps1 -OutFile .\docs\reviews\simplify\source-tree-latest.md`；`git diff --check` |
| commit | 随本轮 `Tools: 兼容拆分后的CMake清单审计` 提交记录 |
