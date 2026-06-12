# Specs - PRD-081 Source Tree Active CMake Audit

## 目标

增强 `tools/source-tree-audit.ps1`，让源码树瘦身报告区分 raw / declared / active 三种 CMake 引用口径，避免被注释和已过滤的生成式 utils 目录误导。

## 非目标

1. 不修改生产 C++。
2. 不修改 `CMakeLists.txt`。
3. 不删除、移动源码文件。
4. 不创建第二构建目录。
5. 不启动子 Agent 并行写代码。

## 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| 审计工具 | `tools/source-tree-audit.ps1` | 增强解析与报告输出 |
| 报告 | `docs/reviews/simplify/source-tree-latest.md` | 刷新审计结果 |
| 文档 | `docs/prd/PRD_081_Source_Tree_Active_CMake_Audit.md` | 新增 PRD |
| 评分 | `docs/tracking/SCORE_TRACKING.md` | 记录阶段分 |
| 禁止修改 | `src/` | 禁止 |
| 禁止修改 | `CMakeLists.txt` | 禁止 |

## 验收命令

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\source-tree-audit.ps1 -OutFile .\docs\reviews\simplify\source-tree-latest.md
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\doctor.ps1
git diff --check
```

## 检查点

1. Summary 中出现 raw / declared / active / filtered generated utils 四类指标。
2. `Active CMake source references` 小于 `Declared CMake source references`。
3. `Filtered generated utils references` 大于 0。
4. `Serial Station Minimal UART Gap` 中 `SerialPortPanel.h/.cpp` 为 `Exists=True` 且 `In CMake=True`。
5. 报告 Interpretation 提醒：active refs 才近似主 GUI 目标编译面。

## BATCH 判定

- 是否需要 BATCH：否。
- 并行度上限：1。
- 原因：本轮修改单一审计脚本，属于共享工具入口。

## LOOP 路由

- Doctor：PowerShell、路径、环境检查失败。
- Debug：报告口径错误或输出路径校验错误。
- Simplify：后续继续基于 active refs 收缩 CMake 或冻结目录。
