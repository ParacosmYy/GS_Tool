# 06 - Git/Commit 与收口纪律（重构版）

> 适用范围：提交前必须遵循，尤其涉及 Python/PyQt 启动链路、打包链路、约束文档更新。

---

## 一、最小提交纪律

- 每轮任务完成后，必须有一次 commit（文档线也可单独提交）。
- 禁止提交构建产物、`build/`、中间文件、自动生成的 moc/ui/qrc 临时产物。
- 禁止提交空提交（无功能/验证增益）。
- 不能提交与本轮无关文件。
- 每次提交仅在门禁通过后可加分，默认本轮加分 1 点（`+1`）且可复用作全局目标评分。

---

## 二、代码提交门禁

- 零测试失败是最低门槛。
- 涉及启动/入口/路径：必须运行 `uv run start-embeddebug --smoke` 与 `cmd /c EmbedDebug.bat --smoke`。
- 涉及打包/依赖/资源：必须运行 `uv run package-embeddebug --version <tag> --clean` 与 `uv run verify-package-embeddebug --package-dir <dir>`。
- 新增产品源码默认落到 `python/embeddebug/`，新增测试默认落到 `tests/python/`。
- 所有新增公共能力需在三轴状态中有 `E` 与 `U` 变更记录。
- 不能在没有证据的情况下宣称 `D` 提升。
- 任何提交都需补 `04/05/03` 相关证据门禁：编码、UI、架构至少一项检查通过。

---

## 三、分层与文件锁

以下文件默认锁定单一负责人（主 Agent）：

- `pyproject.toml`、`uv.lock`
- `EmbedDebug.bat`
- `python/embeddebug/app/`、`python/embeddebug/serial_station/`
- `resources/themes/*.qss`
- `docs/constraints/*.md` 与 `CLAUDE.md`

## 四、闭环、评分与 commit 模板（统一引用 09-closed-loop）

> ⚠️ **本节原定义的 "A-I-R-C-L-M-P 七环"、7 角色子代理回执、"起始分 500" 已全部废除**。
>
> 历史问题：
> - 起始分曾写成 500，与 CLAUDE.md/SCORE_TRACKING 的真实起始分（1，当前 800+）矛盾
> - 7 角色子代理在单 Agent 会话（如 ZCode）无法自动派发，是永远 +0 的幽灵门禁
> - 7 环编号与 02-workflow、serial_station_architecture 的闭环编号三重冲突
>
> **统一权威定义见 [09-closed-loop.md](09-closed-loop.md)**：
> - **§一 单轮闭环**：5 视角自检 + 6 门禁
> - **§二 评分闭环**：起始分 1，目标 1000，每通过门禁 commit +1，canonical 落点 `docs/tracking/SCORE_TRACKING.md`
> - **§五 commit message 模板**（取代本节旧模板）

### 4.1 提交门禁速览（详见 09-closed-loop §一.2）

每次 commit 前必须跑：
1. `uv run test-embeddebug-py`（退出码 0）
2. `uv run start-embeddebug --smoke`（退出码 0）
3. `cmd /c EmbedDebug.bat --smoke`（启动/入口/依赖改动时必跑）
4. `uv run lint-embeddebug-py`（无新错误）
5. `uv run check-constraints`（评分/SSOT/行数漂移检测，退出码 0）

### 4.2 Push 节奏（详见 09-closed-loop §二.2）

- 每 2 个通过门禁的 commit 为 1 个 push 周期
- push 前必须更新 `README.md` 状态表（评分、能力、证据入口）
- push 前若未更新 README，视为收口缺陷，不得进入下一轮加分

### 4.3 Commit Message 模板（唯一权威，见 09-closed-loop §五）

```text
<模块>: <简述改了什么>

门禁: test✓ smoke✓ lint✓ check_constraints✓ [bat✓]
视角: 架构✓ 实现✓ 测试✓ 产品✓ 用户✓
三轴: E<x> U<x> D<x>（本轮变化: <无 / E↑ / U↑ / D↑>）
评分: <旧分> + 1 = <新分>（见 docs/tracking/SCORE_TRACKING.md）
变更: <N> files, <+M> insertions, <-K> deletions
```

若某门禁未跑，写 `X✗（原因：...）`，不得省略。`[bat✓]` 仅在跑了 `cmd /c EmbedDebug.bat --smoke` 时打勾。

---

## 五、收口前检查清单

- [ ] 是否有测试证据？
- [ ] 是否有启动证据（若涉及启动链路）？
- [ ] 是否完成最小 smoke：`uv run start-embeddebug --smoke` 与 `cmd /c EmbedDebug.bat --smoke`？
- [ ] `uv run check-constraints` 退出码 0（评分/SSOT/行数漂移检测通过）？
- [ ] 是否有 `E/U/D` 更新？
- [ ] 是否有测试/替身证据（若涉及核心行为）？
- [ ] 是否有约束文档对应更新？
- [ ] [09-closed-loop §一](09-closed-loop.md) 5 视角自检全部打勾（架构/实现/测试/产品/用户）？
- [ ] [09-closed-loop §一](09-closed-loop.md) 6 门禁全绿（或未跑项已写明原因）？
- [ ] commit message 符合 [09-closed-loop §五](09-closed-loop.md) 模板？
- [ ] 评分已更新到 `docs/tracking/SCORE_TRACKING.md` 首行（唯一处）？
- [ ] 闭环结果已追加到 `docs/tracking/LOOP_STATE.md`？
- [ ] 若本次为 push 周期结尾（2 次提交）：README 已更新且可追溯证据摘要齐全

## 六、提交前固定高效验收

1. `uv run test-embeddebug-py`
2. `uv run test-embeddebug-tools`
3. `uv run start-embeddebug --smoke`
4. `cmd /c EmbedDebug.bat --smoke`
5. 涉及打包时追加 PyInstaller package + verify
6. 不能代替；若失败则 `Check` 门禁失败
