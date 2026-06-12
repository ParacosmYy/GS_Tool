# Specs - PRD-082 Active Utils Dependency Audit

## 目标

增强 `tools/source-tree-audit.ps1`，为 active utils 目录增加外部 include 证据和低风险拆分候选清单。

## 非目标

1. 不修改生产源码。
2. 不修改 `CMakeLists.txt`。
3. 不删除或移动任何文件。
4. 不创建第二构建目录。
5. 不执行 BATCH 并行开发。

## 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| 审计工具 | `tools/source-tree-audit.ps1` | 增加 include 扫描和报告表 |
| 报告 | `docs/reviews/simplify/source-tree-latest.md` | 刷新报告 |
| 文档 | `docs/prd/PRD_082_Active_Utils_Dependency_Audit.md` | 新增 PRD |
| 评分 | `docs/tracking/SCORE_TRACKING.md` | 记录阶段分 |
| 禁止修改 | `src/` | 禁止 |
| 禁止修改 | `CMakeLists.txt` | 禁止 |

## 验收命令

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\source-tree-audit.ps1 -OutFile .\docs\reviews\simplify\source-tree-latest.md
Select-String -Path .\docs\reviews\simplify\source-tree-latest.md -Pattern "Active Utils External Include Evidence","Low-Risk Active Utils Split Candidates"
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1
git diff --check
```

## 检查点

1. `Active Utils External Include Evidence` 表存在。
2. `Low-Risk Active Utils Split Candidates` 表存在。
3. 候选表不包含 canonical utils 目录。
4. Interpretation 明确说明候选只能进入后续 CMake 拆分 PRD，不能直接删除。
5. `Serial Station Minimal UART Gap` 仍保留。

## BATCH 判定

- 是否需要 BATCH：否。
- 并行度上限：1。
- 原因：单一共享审计脚本，必须串行修改和验证。

## LOOP 路由

- Doctor：PowerShell 运行、路径、环境问题。
- Debug：include 统计口径错误。
- Simplify：用候选表进入后续 CMake 拆分或冻结目录计划。
