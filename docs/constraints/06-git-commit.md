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
- `EmbedDebug.bat`、`tools/launch_embeddebug.ps1`
- `python/embeddebug/app/`、`python/embeddebug/serial_station/`
- `resources/themes/*.qss`
- `docs/constraints/*.md` 与 `CLAUDE.md`

## 四、AI 永久闭环与分数系统（与 02/workflow 联动）

### 4.1 评分目标

- 工程起始分：`500`
- 目标分：`1000`
- 单 commit 通过后得分：`+1`
- 关闭门禁失败：`+0`

### 4.1a 提交与 push 节奏门禁

- 两次通过门禁的 commit 形成一次推送周期。
- 每个推送周期前必须先完成 `README.md` 企业级精简版更新。
- 本次周期若未更新 README，禁止执行 push。
- README 变更必须包含：启动路径、最小 smoke、周期得分与证据入口。

### 4.2 提交前门禁（A→I→R→C→L）

1. A（Aim）：本轮目标与 PRD 是否一致，是否包含可量化验收条目。
2. I（Inspect）：必须检查 01、02、03、06、07、08、`serial_station_architecture.md` 的关联更新。
3. R（Record）：确保约束文档和目标分日志可追溯。
4. C（Check）：至少有一次可复现证据（Python 测试、启动 smoke 或 PyInstaller 包验证）。
5. L（Limit）：不允许无关文件、build 产物或边界越界。
6. M（Multi-agent）：必须接收 7 角色闭环（A-1/A-2/D-1/D-2/D-3/P-1/U-1）或等价 6+1 合并产出。
7. P（Push）：满足两次提交周期（第 2/4/6 次门禁提交）后，必须在 push 前同步更新 README 与本周期证据摘要。

### 4.1b Push 周期证据最小模板（企业级）

- 周期口径：`提交1 -> 提交2`（或 `3->4`...）
- 分数变化：`n -> n+1`（本周期最低 +1）
- 证据清单：一次 Python 测试 + `EmbedDebug.bat --smoke` 启动 + 约束文档变更路径

### 4.3 Commit Message 规范（建议）

---

```text
<模块名>: <本轮变更摘要>

背景：<为什么改>
验收：<构建/测试/启动证据>
状态：E<U/D>
```

示例：

```text
serial_station: 重构约束文档并补齐可量化闭环

背景：统一 Serial Station 与总体约束口径，新增主流工具对齐项和验收标准。
验收：uv run test-embeddebug-py ; cmd /c EmbedDebug.bat --smoke
状态：E3 U2 D2
积分：从 500 提升到 501（+1）
```

### 4.4 提交时子代理闭环最小要求

- 每次提交至少附上子代理闭环结论条目，未附者视为门禁失败（不得加分）。
- 任何关键冲突未在 `P`/`U` 环节给出处理结论，提交不得通过。
- 若子代理建议一致但证据不足，按 `+0` 处理并补齐证据后再提交。

---

## 五、收口前检查清单

- [ ] 是否有测试证据？
- [ ] 是否有启动证据（若涉及启动链路）？
- [ ] 是否完成最小 smoke：`uv run start-embeddebug --smoke` 与 `cmd /c EmbedDebug.bat --smoke`？
- [ ] 是否有 `E/U/D` 更新？
- [ ] 是否有测试/替身证据（若涉及核心行为）？
- [ ] 是否有约束文档对应更新？
- [ ] 是否有 7 角色子代理闭环（A-1/A-2/D-1/D-2/D-3/P-1/U-1）并有仲裁结论？
- [ ] 若本次为周期结尾（2 次提交）：README 已更新且可追溯证据摘要齐全

## 六、提交前固定高效验收

1. `uv run test-embeddebug-py`
2. `uv run test-embeddebug-tools`
3. `uv run start-embeddebug --smoke`
4. `cmd /c EmbedDebug.bat --smoke`
5. 涉及打包时追加 PyInstaller package + verify
6. 不能代替；若失败则 `Check` 门禁失败
