# PRD-109 - Tools Test Direct Entry

## 1. 标题

`PRD-109 - Tools Test Direct Entry`

## 2. 目标

- 修复 `python tools/test_embeddebug_tools.py` 直接运行时找不到 `tools` 包的问题。
- 保持 `uv run test-embeddebug-tools` 入口不回归。
- 本轮三轴目标：工程 `E3 -> E4`，用户不提升，设备不提升。

## 3. 非目标

- 不修改生产 C++。
- 不修改 CMake、启动脚本或构建输出路径。
- 不改工具业务逻辑。
- 不新增 Python 依赖。

## 4. 约束

- 遵守 `CLAUDE.md`、`docs/constraints/01-project-overview.md`、`docs/constraints/02-workflow.md`、`docs/constraints/04-coding-standard.md`、`docs/constraints/07-directory-structure.md`。
- 构建目录只能是 `build/`。
- 用户已有改动不得回退。

## 5. 改动范围

| 范围 | 路径 | 允许动作 |
|------|------|----------|
| 测试工具 | `tools/test_embeddebug_tools.py` | 修改 |
| 文档 | `docs/prd/PRD_109_Tools_Test_Direct_Entry.md` | 新增 |
| 文档 | `docs/superpowers/specs/PRD_109_Tools_Test_Direct_Entry_Specs.md` | 新增 |
| 评分 | `docs/tracking/SCORE_TRACKING.md` | 修改 |
| 禁止修改 | `src/` | 禁止 |
| 禁止修改 | `CMakeLists.txt`, `cmake/` | 禁止 |

## 6. 验收标准

- [ ] `python tools/test_embeddebug_tools.py` 通过。
- [ ] `uv run test-embeddebug-tools` 通过。
- [ ] 没有新增第二构建目录。
- [ ] 本轮结束有 commit。

## 7. 失败条件

- 直接入口仍出现 `ModuleNotFoundError`。
- `uv run test-embeddebug-tools` 回归失败。
- 需要修改被测工具业务逻辑才能收口。

## 8. GO 配置

```json
{
  "max_rounds": 20,
  "max_minutes": 30,
  "execute": [
    "python tools/test_embeddebug_tools.py",
    "uv run test-embeddebug-tools"
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
- 共享文件锁：`tools/test_embeddebug_tools.py`。

## 10. LOOP 路由

| 失败表现 | 路由 | 必须产出 |
|----------|------|----------|
| Python 路径异常 | Debug | `sys.path` 根因和修复 |
| uv 环境异常 | Doctor | 命令输出摘要 |

## 11. 收口记录

| 项 | 结果 |
|----|------|
| 工程状态 | `E3 -> E4`，直接 Python 入口与 uv 入口均通过 |
| 用户状态 | 不提升，产品行为无变化 |
| 设备状态 | 不提升，真实硬件未验证 |
| 验证命令 | `python tools/test_embeddebug_tools.py`；`uv run test-embeddebug-tools` |
| commit | 随本轮 `Tools: 修复自测脚本直接入口` 提交记录 |
