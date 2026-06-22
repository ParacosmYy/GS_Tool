# Batch 50 — 死代码守护测试 + CI 严格化

> 分支：`feat/embed-debug` | 模式：永续迭代 Batch 50/∞
> 来源：CLAUDE.md §"后续路线图" Batch 50 + 用户「一直迭代下去，永远不要停止」持续授权
> 前置：Batch 49 已完成加载态 + 空态完善（commit ec88e650d）
> 评分目标：708 → 713+（本轮 +5，每子任务 +1）

---

## 1. 标题

`Batch 50 - 死代码守护测试（模块级 + token 级）+ CI 严格化（GitHub Actions + ruff）`

---

## 2. 目标

### 用户可观察到什么变化

- **CI badge** 出现在 README，每次 push 到 `feat/embed-debug` 自动跑全量测试（1678+ 测试），失败阻止合入。
- 死代码（写了但未接入生产的模块/方法）一旦引入立即被守护测试拦截，不再累积。
- 动画 token / factory 的「孤儿」(定义但无引用)被检测。
- lint(ruff F401)机械捕获未使用 import。

### 主要交付物

| 子任务 | 主要文件 | 目标 |
|---|---|---|
| **B50-1** | 新建 `tests/python/unit/test_no_dead_widgets.py` | 枚举 `controls/__init__.py.__all__`，断言每个符号有 ≥1 production 调用（non-test/non-init/non-QSS）|
| **B50-2** | 新建 `tests/python/unit/test_animation_wiring.py` | 每个 Batch 40+ 动画模块有 ≥1 production 调用 + 每个动画 token 有 ≥1 factory 引用（DC-1 + DC-2）|
| **B50-3** | wire `elevation_effect` 到 `micro_interactions` + 7 widget 白名单 | 激活新发现的死代码（elevation）+ 为 7 个未 wire 的 widget 建立明确白名单 |
| **B50-4** | 新建 `.github/workflows/ci.yml` + README badge | GitHub Actions 跑 test-embeddebug-py + STRICT_COLORS/FONTS |
| **B50-5** | `pyproject.toml` 加 `[tool.ruff]` + `devtools/lint.py` + `lint-embeddebug-py` script | ruff F401/F841 机械捕获死代码 |
| **B50-6** | `pyproject.toml` 加 `[tool.pytest.ini_options]` | testpaths + addopts(`--strict-markers`) + filterwarnings |

### 三轴目标

- 工程 `E4 → E5`（可维护收口）：守护测试 + CI 自动化将「人工检查」转为「机器强制」。
- 用户 `U4`（保持）：无用户可见行为变化（纯工程基建）。
- 设备 `D2`（保持）：不涉及硬件路径。

### 完成证明命令

```powershell
uv run test-embeddebug-py              # 全量通过，含新增守护测试
uv run start-embeddebug --smoke        # exit 0
cmd /c EmbedDebug.bat --smoke          # exit 0
uv run lint-embeddebug-py              # ruff 0 errors（若实施 B50-5）
```

---

## 3. 非目标

明确不做：
- **不做** 真实硬件验证
- **不做** 7 个死 widget 的完整 wire（那是独立 UI 批次）—— 仅建立白名单豁免
- **不做** mypy/typecheck（ruff 已覆盖主要静态检查，mypy 重）
- **不做** pre-commit hooks（CI workflow 已足够，pre-commit 增加本地负担）
- **不做** 跨平台 CI（项目 Windows-only，单 runner 足够）
- **不做** 覆盖率门禁（coverage 工具引入风险高，defer 到 Batch 51+）

---

## 4. 约束

按 `AGENTS.md §一` 强制加载顺序，本批涉及：
1. [CLAUDE.md](../../../CLAUDE.md) — 铁律 5.10（每轮迭代完成必须提交）
2. [docs/constraints/03-architecture.md](../../constraints/03-architecture.md) — 分层单向依赖
3. [docs/constraints/06-git-commit.md](../../constraints/06-git-commit.md) — 提交门禁
4. [docs/constraints/04-coding-standard.md](../../constraints/04-coding-standard.md) — 测试组织

运行时约束：
- 新增测试文件 ≤ 250 行（铁律 21，`test_python_default_cutover` 守护）
- 新增测试文件禁用 `*_batch*.py` 命名（`test_python_governance_docs` 守护）
- 新增测试必须设 `QT_QPA_PLATFORM=offscreen`（涉及 Qt 的）
- CI workflow 必须用 Windows runner（项目 Windows-only）

---

## 5. 改动范围

| 范围 | 路径 | 允许动作 |
|---|---|---|
| **测试新增** | `tests/python/unit/test_no_dead_widgets.py` | 新建（≤250 行）|
| **测试新增** | `tests/python/unit/test_animation_wiring.py` | 新建（≤250 行）|
| **代码修改** | `python/embeddebug/serial_station/ui/micro_interactions.py` | wire `elevation_effect` |
| **配置新增** | `.github/workflows/ci.yml` | 新建 |
| **配置修改** | `pyproject.toml` | 加 `[tool.ruff]` + `[tool.pytest.ini_options]` + `lint-embeddebug-py` script + ruff 依赖 |
| **代码新增** | `python/embeddebug/devtools/lint.py` | 新建（ruff 包装器）|
| **文档修改** | `README.md` | 加 CI badge |
| **文档修改** | `CLAUDE.md` + `SCORE_TRACKING.md` | 更新 Batch 50 + 评分 |

---

## 6. 验收标准

### 6.1 功能验收

- [ ] **B50-1**：`test_no_dead_widgets.py` 枚举 `controls.__all__`，白名单列出 7 个未 wire widget（Drawer/Badge/Chip/InfoBanner/Segmented/ToggleSwitch/Divider），断言白名单外的每个符号有 ≥1 production import。
- [ ] **B50-2**：`test_animation_wiring.py` 断言每个 Batch 40+ 动画模块(bounce_path/glow/rotate/typewriter/elastic_snap/color_tween已删/page_slide已删 等)有 ≥1 production 调用；每个 `AnimationTokens.DURATION_*`/`EASE_*` token 有 ≥1 factory 引用。
- [ ] **B50-3**：`elevation_effect` 被 `micro_interactions` 引用（wire 到 hover_lift 或类似），白名单 7 widget 文档化。
- [ ] **B50-4**：`.github/workflows/ci.yml` 在 push/PR 时跑 `uv run test-embeddebug-py`（Windows runner），README 加 badge。
- [ ] **B50-5**：`uv run lint-embeddebug-py` 跑 ruff，0 errors。
- [ ] **B50-6**：`pyproject.toml` 有 `[tool.pytest.ini_options]`，testpaths + addopts。

### 6.2 三轴验收

- [ ] 工程 `E4 → E5`：守护测试 + CI 自动化落地。
- [ ] 用户 `U4`（保持）：无行为变化。
- [ ] 设备 `D2`（保持）。

### 6.3 铁律验收

- [ ] 零编译错误，全量测试通过（1678+ passed，0 failed）。
- [ ] 新增测试 ≤ 250 行。
- [ ] commit ≥ 500 行净代码。
- [ ] SCORE_TRACKING.md 更新 Batch 50 条目。

---

## 7. 失败条件

- 守护测试本身误报（白名单不全导致正常 widget 被标死）
- CI workflow 语法错误（无法在 GitHub 上运行）
- ruff 引入大量误报（需合理配置 ignore 规则）
- pytest strict 模式导致既有 warning 测试失败
- 单个 commit < 500 行净代码
- 触发 20 轮或 30 分钟安全刹车

---

## 8. GO 配置

```json
{
  "max_rounds": 20,
  "max_minutes": 30,
  "execute": ["uv run test-embeddebug-py"],
  "check": ["uv run start-embeddebug --smoke", "uv run lint-embeddebug-py", "git status --short"],
  "fix": []
}
```

---

## 9. BATCH 判定

- **子任务数量**：6（B50-1 ~ B50-6）
- **并行度上限**：2（B50-1/B50-2 测试可并行；B50-4/B50-5/B50-6 配置可并行；B50-3 串行）
- **共享文件锁**：`pyproject.toml`（B50-5 + B50-6 共改 — 串行合并）

---

## 10. LOOP 路由

| 失败表现 | 路由 | 必须产出 |
|---|---|---|
| 守护测试误报正常 widget | Debug | 调整白名单或 wire |
| ruff 误报 | Debug | 调整 ignore 规则 |
| CI workflow 语法错 | Doctor | yml lint |
| pytest strict 触发 warning | Debug | filterwarnings 调整 |

---

## 11. 收口记录

| 项 | 结果 |
|----|------|
| 工程状态 | `E4 → E5`（守护测试 + CI 自动化）|
| 用户状态 | `U4`（保持）|
| 设备状态 | `D2`（保持）|
| 验证命令 | 全量 test + smoke + lint 全部 exit 0 |
| 评分变更 | `708 → 713+` |
| commit | 待收口后填写 hash |

---

> **Specs 完成时间**：2026-06-22
> **下一步**：用户持续授权「一直迭代」→ 直接执行 Wave 1（B50-1/B50-2 守护测试并行）
