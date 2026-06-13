# PRD-108 - CMake Source List Dedup

## 1. 标题

`PRD-108 - CMake Source List Dedup`

## 2. 目标

- 移除 `cmake/EmbedDebugSources.cmake` 中重复的 chart eye/math 源码和头文件清单项。
- 让项目审计工具报告 `duplicate entries: 0`。
- 本轮三轴目标：工程 `E3 -> E4`，用户不提升，设备不提升。

## 3. 非目标

- 不修改生产 C++。
- 不重排整个 CMake 清单。
- 不处理 generated utils 目录。
- 不改变构建输出路径或启动脚本。

## 4. 约束

- 遵守 `CLAUDE.md`、`docs/constraints/01-project-overview.md`、`docs/constraints/02-workflow.md`、`docs/constraints/04-coding-standard.md`、`docs/constraints/07-directory-structure.md`。
- 构建目录只能是 `build/`。
- 用户已有改动不得回退。

## 5. 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| 构建清单 | `cmake/EmbedDebugSources.cmake` | 删除重复条目 |
| 报告 | `docs/reviews/simplify/source-tree-latest.md` | 刷新 |
| 文档 | `docs/prd/PRD_108_CMake_Source_List_Dedup.md` | 新增 |
| 文档 | `docs/superpowers/specs/PRD_108_CMake_Source_List_Dedup_Specs.md` | 新增 |
| 评分 | `docs/tracking/SCORE_TRACKING.md` | 修改 |
| 禁止修改 | `src/` | 禁止 |
| 禁止修改 | `CMakeLists.txt` | 禁止 |

## 6. 验收标准

- [ ] `python tools/project-audit/project_audit.py --limit 5` 显示 `duplicate entries: 0`。
- [ ] `cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=C:/msys64/mingw64` 配置通过。
- [ ] `cmake --build build --target EmbedDebug --parallel 4` 构建通过。
- [ ] `powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\source-tree-audit.ps1 -OutFile .\docs\reviews\simplify\source-tree-latest.md` 通过。
- [ ] 没有新增第二构建目录。
- [ ] 本轮结束有 commit。

## 7. 失败条件

- 删除后 CMake 无法找到某个 chart eye/math 文件。
- 构建失败且原因指向本轮去重。
- 审计仍显示 duplicate entries 大于 0。
- 需要改生产源码才能收口。

## 8. GO 配置

```json
{
  "max_rounds": 20,
  "max_minutes": 30,
  "execute": [
    "python tools/project-audit/project_audit.py --limit 5",
    "cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=C:/msys64/mingw64",
    "cmake --build build --target EmbedDebug --parallel 4",
    "powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\source-tree-audit.ps1 -OutFile .\\docs\\reviews\\simplify\\source-tree-latest.md"
  ],
  "check": [
    "git status --short"
  ],
  "fix": []
}
```

## 9. BATCH 判定

- 是否需要 BATCH：否。
- 并行度上限：1。
- 共享文件锁：`cmake/EmbedDebugSources.cmake`。

## 10. LOOP 路由

| 失败表现 | 路由 | 必须产出 |
|----------|------|----------|
| CMake 配置或构建失败 | Debug | 失败文件和恢复方案 |
| 审计运行异常 | Doctor | 工具输出摘要 |
| 仍存在大量重复 | Simplify | 下一轮 CMake 清单整理计划 |

## 11. 收口记录

| 项 | 结果 |
|----|------|
| 工程状态 | `E3 -> E4`，CMake 去重、配置、构建和审计通过 |
| 用户状态 | 不提升，产品行为无变化 |
| 设备状态 | 不提升，真实硬件未验证 |
| 验证命令 | `python tools/project-audit/project_audit.py --limit 5`；`cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=C:/msys64/mingw64`；`cmake --build build --target EmbedDebug --parallel 4`；`powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\source-tree-audit.ps1 -OutFile .\docs\reviews\simplify\source-tree-latest.md`；`.\EmbedDebug.bat` 启动探针 |
| commit | 随本轮 `CMake: 移除源码清单重复项` 提交记录 |
