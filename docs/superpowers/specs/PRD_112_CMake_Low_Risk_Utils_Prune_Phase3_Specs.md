# PRD-112 - CMake Low Risk Utils Prune Phase 3

## 1. 标题

`PRD-112 - CMake Low Risk Utils Prune Phase 3`

## 2. 目标

- 从主 GUI 目标 CMake 清单中移除第三批低风险 active utils 候选目录。
- 保留源码，不删除、不移动。
- 本轮三轴目标：工程 `E3 -> E4`，用户不提升，设备不提升。

## 3. 非目标

- 不修改生产 C++。
- 不修改测试 CMake。
- 不处理 `src/utils/scripting/`。
- 不处理 generated numbered utils。
- 不改变启动脚本、资源路径或用户入口。

## 4. 约束

- 遵守 `CLAUDE.md`、`docs/constraints/01-project-overview.md`、`docs/constraints/02-workflow.md`、`docs/constraints/04-coding-standard.md`、`docs/constraints/07-directory-structure.md`。
- 构建目录只能是 `build/`。
- 用户已有改动不得回退。
- 本轮改 CMake 清单，收口必须验证 `EmbedDebug.bat`。

## 5. 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| 构建清单 | `cmake/EmbedDebugSources.cmake` | 删除指定目录条目 |
| 报告 | `docs/reviews/simplify/source-tree-latest.md` | 刷新 |
| 文档 | `docs/prd/PRD_112_CMake_Low_Risk_Utils_Prune_Phase3.md` | 新增 |
| 文档 | `docs/superpowers/specs/PRD_112_CMake_Low_Risk_Utils_Prune_Phase3_Specs.md` | 新增 |
| 评分 | `docs/tracking/SCORE_TRACKING.md` | 修改 |
| 禁止修改 | `src/` | 禁止 |
| 禁止修改 | `tests/CMakeLists.txt` | 禁止 |
| 禁止修改 | `EmbedDebug.bat`, `tools/launch_embeddebug.ps1` | 禁止 |

## 6. 验收标准

- [ ] `python tools/project-audit/project_audit.py --limit 5` 显示 `duplicate entries: 0` 和 `missing listed files: 0`。
- [ ] `cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=C:/msys64/mingw64` 配置通过。
- [ ] `cmake --build build --target EmbedDebug --parallel 4` 构建通过。
- [ ] `powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\source-tree-audit.ps1 -OutFile .\docs\reviews\simplify\source-tree-latest.md` 通过。
- [ ] `.\EmbedDebug.bat` 启动探针通过。
- [ ] 没有新增第二构建目录。
- [ ] 本轮结束有 commit。

## 7. 失败条件

- 主目标构建失败且原因指向被移除的 utils 目录。
- 审计显示缺失 CMake 文件或重复条目。
- 需要修改生产源码才能完成。
- `EmbedDebug.bat` 启动探针失败。

## 8. GO 配置

```json
{
  "max_rounds": 20,
  "max_minutes": 30,
  "execute": [
    "python tools/project-audit/project_audit.py --limit 5",
    "cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=C:/msys64/mingw64",
    "cmake --build build --target EmbedDebug --parallel 4",
    "powershell -NoProfile -ExecutionPolicy Bypass -File .\\tools\\source-tree-audit.ps1 -OutFile .\\docs\\reviews\\simplify\\source-tree-latest.md",
    ".\\EmbedDebug.bat"
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
| CMake 配置/构建失败 | Debug | 失败符号和恢复方案 |
| 启动脚本失败 | Doctor | 工具链与运行库诊断摘要 |
| 审计指标异常 | Simplify | 下一轮候选调整 |

## 11. 收口记录

| 项 | 结果 |
|----|------|
| 工程状态 | `E3 -> E4`，审计、配置、构建和启动探针通过 |
| 用户状态 | 不提升 |
| 设备状态 | 不提升 |
| 验证命令 | `python tools/project-audit/project_audit.py --limit 5`；`cmake -G Ninja -B build -DCMAKE_PREFIX_PATH=C:/msys64/mingw64`；`cmake --build build --target EmbedDebug --parallel 4`；`powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\source-tree-audit.ps1 -OutFile .\docs\reviews\simplify\source-tree-latest.md`；`.\EmbedDebug.bat` 启动探针；`git diff --check` |
| commit | 随本轮 `CMake: 移除第三批低风险utils候选` 提交记录 |
