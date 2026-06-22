# Batch 49 — 加载态 + 空态完善（对标铁律 5.9 三轴口径）

> 分支：`feat/embed-debug` | 模式：永续迭代 Batch 49/∞（ralph-loop active）
> 来源：CLAUDE.md §"后续路线图" Batch 49 + 用户「快速推进，少打断」指示
> 前置：Batch 47 已完成 4 条连接路径 `_set_loading`（fake/serial/tcp/udp）；Batch 48 已完成排版/间距 token
> 评分目标：703 → 708+（本轮 +5，每子任务 +1）

---

## 1. 标题

`Batch 49 - 加载态（Connect/Refresh 增强）+ 空态（Log/Waveform/Dashboard）`

---

## 2. 目标

### 用户可观察到什么变化

- 打开应用但未连接时：日志区、波形区、仪表盘画布**均显示精致空态**（图标 + 标题 + 描述 + CTA），不再是裸 QPlainTextEdit 占位符或单行 QLabel。
- 点击「连接」时：连接按钮内嵌 ProgressRing 旋转动画；同时日志区/波形区显示骨架屏（SkeletonBlock）告知用户「正在建立连接…」。
- 点击「刷新端口」时：刷新按钮内嵌 ProgressRing；不再重复实现 loading 逻辑。
- 连接成功并收到第一帧数据时：空态/骨架屏优雅淡出（200ms EASE_OUT_QUART），真实数据淡入。
- 断开连接时：空态重新淡入显示。

### 主要交付物

| 子任务 | 主要文件 | 目标 |
|---|---|---|
| **B49-1** Log 空态 | `python/embeddebug/serial_station/ui/sections.py` + 新建 `ui/log_empty_state.py` | 替换 QPlainTextEdit 占位符为 `EmptyStateWidget`；首条日志写入时淡出 |
| **B49-2** Log 加载态 | 新建 `ui/log_loading_state.py` + 修改 `connection_actions.py` | 连接开始显示 SkeletonBlock「正在建立连接…」；首帧 RX 或连接失败时淡出 |
| **B49-3** Waveform 空态 + 加载态 | `python/embeddebug/serial_station/ui/waveform_preview.py` | 无数据时显示 EmptyStateWidget 覆盖层；连接中显示 ProgressRing |
| **B49-4** Dashboard 画布空态 | `python/embeddebug/serial_station/ui/dashboard/canvas.py` | 无 widget 时显示 EmptyStateWidget「画布为空，拖入控件」 |
| **B49-5** Connection 加载态升级 + Port Refresh 重构 | `connection_actions.py:_set_loading` + `connection_toolbar.py:_refresh_with_loading` | `_set_loading` 内嵌 ProgressRing；`_refresh_with_loading` 复用 `_set_loading`（消除重复） |
| **B49-6** UI smoke 测试 | 新建/扩展 `tests/python/ui_smoke/test_*_states.py` | 每个子任务配套 ui_smoke（qtbot 验证可见性切换） |

### 三轴目标

- 工程 `E4`（自动化测试通过）：每个子任务至少 1 条 ui_smoke 测试覆盖空态/加载态切换。
- 用户 `U3 → U4`（体验完整）：所有关键空区都有可发现、可恢复的空态/加载态。
- 设备 `D2`（保持）：不涉及真实硬件路径，沿用替身验证。

### 完成证明命令

```powershell
uv run test-embeddebug-py              # 全量测试通过，0 failed
uv run start-embeddebug --smoke        # exit 0
cmd /c EmbedDebug.bat --smoke          # exit 0
```

人工验证（连接路径）：
1. 启动应用 → 确认日志/波形/仪表盘显示精致空态（图标 + 文案）
2. 选择 Fake Transport → 点击 Connect → 确认按钮 ProgressRing 旋转 + 骨架屏显示
3. 等待 1-2 秒 → 确认空态/骨架屏淡出，真实数据淡入
4. 点击 Disconnect → 确认空态重新淡入

---

## 3. 非目标

明确不做：

- **不做** 真实硬件验证（D3/D4 状态保持 D2，沿用 fake/tcp/udp 替身）
- **不做** AppShell 页面切换加载态（panels 已同步预构建，无 lazy load 缺口）
- **不做** 其他面板（CAN/BLE/MQTT/RTT/OTA/SVD）的空态 — 这些已有 EmptyStateWidget（B49 范围外）
- **不做** 新增动画原语（复用 Batch 40-48 已落地的 fade/slide/skeleton/progress_ring）
- **不做** ProgressRing 控件本身的重构（仅消费它）
- **不做** ROADMAP_SURPASS_VOFA.md 波形 FFT/直方图实现（独立批次）
- **不做** 文档对账（docs stale 修复属于独立 doc-debt 批次，本批仅 commit 时按 06-git-commit 同步 SCORE_TRACKING）

不触碰模块：

- `controllers/`、`core/`、`protocols/`、`services/`、`drivers/` — 本批纯 UI 层
- `app/app_shell.py` — 范围外
- 主题 `theme/` 已有的 palette/tokens — 仅消费，不修改
- `pyproject.toml`、`uv.lock`、`README.md` — 本批不动

---

## 4. 约束

按 `AGENTS.md §一` 强制加载顺序，本批涉及：

1. [CLAUDE.md](../../../CLAUDE.md) — 铁律 5.9（禁止把工程存在当作用户完成）、铁律 15-19（UI 铁律）、铁律 20-22（文件体积）
2. [docs/constraints/03-architecture.md](../../constraints/03-architecture.md) — 分层单向依赖
3. [docs/constraints/05-ui-standard.md](../../constraints/05-ui-standard.md) — §1.1 列表/日志必须有空态文案；§3.1 连续刷新无闪烁；§4.1 提交 3 档状态；§五 视图刷新边界保护
4. [docs/constraints/04-coding-standard.md](../../constraints/04-coding-standard.md) — PyQt 信号/槽集中装配，QObject 生命周期
5. [docs/serial_station_architecture.md](../../serial_station_architecture.md) — UI 只发意图，controller 编排

运行时约束：

- 所有新增控件必须设置 `objectName`（铁律 16）
- 所有新增用户可见文字必须走 `tr()` 或集中常量（铁律 19）
- 所有按钮/可交互控件必须齐全 hover/pressed/disabled 三态（铁律 17）
- 面板切换/状态切换必须有过渡动画，禁止突然出现/消失（铁律 18）— 空态切换用 fade 200ms EASE_OUT_QUART
- 单个 `.py ≤ 300 行`（铁律 20）
- 单个测试文件 `≤ 250 行`，ui_smoke `≤ 225 行`（铁律 21-22）

---

## 5. 改动范围

| 范围 | 路径 | 允许动作 |
|---|---|---|
| **主要新增** | `python/embeddebug/serial_station/ui/log_empty_state.py` | 新建 |
| **主要新增** | `python/embeddebug/serial_station/ui/log_loading_state.py` | 新建 |
| **主要修改** | `python/embeddebug/serial_station/ui/sections.py` | 修改（接入空态/加载态到 log_view 创建逻辑） |
| **主要修改** | `python/embeddebug/serial_station/ui/waveform_preview.py` | 修改（添加空态覆盖层 + 连接 ProgressRing） |
| **主要修改** | `python/embeddebug/serial_station/ui/dashboard/canvas.py` | 修改（添加空态覆盖层） |
| **主要修改** | `python/embeddebug/serial_station/ui/connection_actions.py` | 修改（`_set_loading` 升级内嵌 ProgressRing） |
| **主要修改** | `python/embeddebug/serial_station/ui/connection_toolbar.py` | 修改（`_refresh_with_loading` 复用 `_set_loading`） |
| **主要修改** | `python/embeddebug/serial_station/ui/log_actions.py` 或新 helper | 修改（首条日志写入时触发空态 hide） |
| **测试新增** | `tests/python/ui_smoke/test_log_view_states.py` | 新建（≤225 行） |
| **测试新增** | `tests/python/ui_smoke/test_waveform_states.py` | 新建（≤225 行） |
| **测试新增** | `tests/python/ui_smoke/test_dashboard_states.py` | 新建（≤225 行） |
| **测试新增/扩展** | `tests/python/ui_smoke/test_connection_loading.py` | 新建（ProgressRing 集成） |
| **只读参考** | `python/embeddebug/serial_station/ui/panels/can_panel.py` | 只读（EmptyStateWidget 集成模板） |
| **只读参考** | `python/embeddebug/serial_station/ui/widgets/empty_state.py` | 只读（API） |
| **只读参考** | `python/embeddebug/serial_station/ui/widgets/skeleton.py` | 只读（API） |
| **只读参考** | `python/embeddebug/serial_station/ui/controls/progress_ring.py` | 只读（API） |
| **只读参考** | `python/embeddebug/serial_station/ui/animations/fade.py`、`skeleton.py` | 只读（动画 API） |
| **禁止修改** | `controllers/`、`core/`、`protocols/`、`services/`、`drivers/` | 禁止 |
| **禁止修改** | `theme/`（palette/tokens/qss_builder/qss_sections_*） | 禁止（仅消费已有 token） |
| **禁止修改** | `pyproject.toml`、`uv.lock`、`README.md` | 禁止（提交时单独 docs commit） |

---

## 6. 验收标准（先写标准再执行）

### 6.1 功能验收

- [ ] **B49-1 Log 空态**：未连接时，日志区显示 `EmptyStateWidget`（icon="inbox", title="暂无日志", description="连接设备后将显示收发数据", cta_text="连接设备"）；首条日志写入时 `hide_with_fade()`；清空日志时 `show_with_fade()`。（CTA 文案最终在 plan 阶段定）
- [ ] **B49-2 Log 加载态**：点击 Connect 后，日志区显示 `SkeletonBlock`（3 行闪烁）+ QLabel「正在建立连接…」；首帧 RX 或连接失败时淡出。
- [ ] **B49-3 Waveform 空态 + 加载态**：无 batch 时，plot 上方覆盖 `EmptyStateWidget`（icon="activity", title="等待波形数据"）；连接中显示 `ProgressRing` 居中；首次 `submit_batch()` 时空态淡出。
- [ ] **B49-4 Dashboard 空态**：`canvas.items` 为空时，画布中心显示 `EmptyStateWidget`（icon="layout-dashboard", title="画布为空", description="从左侧拖入控件"）；首个 widget 添加时淡出；清空时淡入。
- [ ] **B49-5 Connection ProgressRing**：`_set_loading(True)` 时按钮内嵌 `ProgressRing`（替换或附加 "…" 文本）；`_refresh_with_loading` 改为调用 `_set_loading` 消除重复。
- [ ] **B49-6 测试**：4 个 ui_smoke 测试文件全部通过；每个测试覆盖空态可见→数据到达→淡出→断开→重新可见的完整循环。

### 6.2 三轴验收

- [ ] 工程状态 `E4`：`uv run test-embeddebug-py` 通过；新增 4 个 ui_smoke 全绿。
- [ ] 用户状态 `U3 → U4`：人工启动验证 4 项交互路径全部可观察、可恢复（见 §2 完成证明：空态显示 / 连接中加载态 / 数据到达淡出 / 断开重新空态）。
- [ ] 设备状态 `D2`（保持）：仅使用 fake/tcp替身验证，不涉及真实硬件。

### 6.3 铁律验收

- [ ] 零编译错误（`uv run start-embeddebug --smoke` exit 0）
- [ ] `EmbedDebug.bat --smoke` exit 0
- [ ] 单个 `.py ≤ 300 行`，测试文件符合体积约束
- [ ] 新增控件全部设置 `objectName`
- [ ] 新增用户可见文字全部 `tr()`
- [ ] 状态切换全部带 fade 过渡（200ms EASE_OUT_QUART）
- [ ] commit ≥ 500 行净增代码（不计文档/空白/注释）
- [ ] SCORE_TRACKING.md 更新 Batch 49 条目

### 6.4 文档同步

- [ ] `docs/tracking/SCORE_TRACKING.md` 追加 Batch 49 摘要条目
- [ ] CLAUDE.md §"后续路线图" 标记 Batch 49 完成，更新当前评分

---

## 7. 失败条件

- 出现无法解释的 smoke 启动失败
- 需要修改 `controllers/`/`core/`/`protocols/`/`services/`/`drivers/`（违反架构边界）
- 需要修改 `theme/` token（违反「仅消费」约束）
- 子任务之间发生同文件冲突（特别是 `connection_actions.py` 被 B49-2 和 B49-5 同时修改）
- 单个 commit < 500 行净代码（违反铁律 3）
- 实际改动需要扩大到非目标范围（如必须改 AppShell 或 controllers）
- 无法提供三轴状态证据
- GO 循环触发 20 轮或 30 分钟安全刹车

---

## 8. GO 配置

GO 只执行已经写入 Specs 的命令。循环顺序：execute → check → fix → 下一轮。

```json
{
  "max_rounds": 20,
  "max_minutes": 30,
  "execute": [
    "uv run test-embeddebug-py"
  ],
  "check": [
    "uv run start-embeddebug --smoke",
    "git status --short"
  ],
  "fix": []
}
```

安全刹车：触发后停止，输出最后失败命令和错误摘要，转入 LOOP（Doctor/Debug/Simplify 路由）。

---

## 9. BATCH 判定

- **是否需要 BATCH**：是
- **子任务数量**：6（B49-1 ~ B49-6）
- **并行度上限**：3（B49-1/3/4 可并行；B49-2 依赖 B49-1；B49-5 串行；B49-6 在 B49-1~5 完成后并行）
- **人工审查状态**：待用户审查（本 specs 即审查入口）
- **共享文件锁**：
  - `connection_actions.py`（B49-2 + B49-5 共改 — 必须**串行**：先 B49-5 升级 `_set_loading`，再 B49-2 消费）
  - `sections.py`（B49-1 + B49-2 共改 — 必须**串行**：先 B49-1 加空态，再 B49-2 加载态）
- **子任务是否文件互不重叠**：除上述 2 个共享文件外，是

### 并行执行计划

**Wave 1（并行 3 agent）**：
- B49-1 Log 空态（独立文件 + sections.py 修改）
- B49-3 Waveform 空态 + 加载态（独立文件 waveform_preview.py）
- B49-4 Dashboard 空态（独立文件 dashboard/canvas.py）

**Wave 2（串行，依赖 Wave 1）**：
- B49-5 Connection `_set_loading` 升级 ProgressRing（改 connection_actions.py）
- B49-2 Log 加载态（改 sections.py + 新建 log_loading_state.py，消费 B49-5 的 ProgressRing）
- B49-5b Port Refresh 重构（改 connection_toolbar.py，复用 _set_loading）

**Wave 3（并行，依赖 Wave 1+2）**：
- B49-6a 测试 log + connection loading
- B49-6b 测试 waveform + dashboard

**Wave 4（串行收口）**：
- 跑全量测试 + smoke 验证
- 更新 SCORE_TRACKING.md + CLAUDE.md
- commit

---

## 10. LOOP 路由

| 失败表现 | 路由 | 必须产出 |
|---|---|---|
| smoke 启动失败（PyQt/导入错误） | Doctor | 诊断输出 |
| EmptyStateWidget 显示但淡入淡出失效 | Debug | 复现命令 + 动画时序审查 |
| `connection_actions.py` 修改后控制器回归失败 | Debug | 回归命令 + 候选修复 |
| 新增文件超过 300 行 | Simplify | 拆分方案 |

---

## 11. 收口记录

| 项 | 结果 |
|----|------|
| 工程状态 | `E4 → E4`（保持，新增 4 ui_smoke 测试） |
| 用户状态 | `U3 → U4`（体验完整：所有空区有可发现可恢复状态） |
| 设备状态 | `D2 → D2`（保持，不涉及真实硬件） |
| 验证命令 | `uv run test-embeddebug-py` + `uv run start-embeddebug --smoke` + `cmd /c EmbedDebug.bat --smoke` 全部 exit 0 |
| 评分变更 | `703 → 708+`（每子任务 +1） |
| commit | 待 Wave 4 收口后填写 hash |

---

## 附录 A：参考实现（can_panel.py EmptyStateWidget 集成模板）

```python
# can_panel.py:82-90 (canonical pattern)
self._empty_state = EmptyStateWidget(
    icon_name="cable",
    title=self.tr("CAN 设备未连接"),
    description=self.tr("选择 CAN 接口并点击连接开始调试"),
    cta_text=self.tr("刷新接口"),
    cta_callback=self._on_refresh,
)
self._empty_state.setObjectName("canEmptyState")
layout.addWidget(self._empty_state)

# 数据到达时（line 172）：
self._empty_state.hide_with_fade()

# 重新为空时（line 223）：
self._empty_state.show_with_fade()
```

## 附录 B：动画 token 参考

| 用途 | Token | 时长 |
|---|---|---|
| 空态淡入 | `DURATION_FAST` + `EASE_OUT_QUART` | 200ms |
| 空态淡出 | `DURATION_INSTANT` + `EASE_OUT_QUART` | 160ms |
| 骨架屏闪烁 | `SkeletonAnimation` 默认 | 1000ms loop |
| ProgressRing 旋转 | `DURATION_CONTAINER` | 800ms loop |

## 附录 C：Layering 合规检查

每个子任务必须通过以下检查（governance test 已守护）：

- 新增 UI 文件 import 仅来自 `PyQt6`、`..controls`、`..widgets`、`..animations`、`..theme` — 禁止 import `..controllers`、`..core`、`..protocols`
- 状态切换由 UI 持有（EmptyStateWidget 实例）；触发由 `_actions` 模块函数发起（消费 controller 信号）
- 不在 `main_window.py` 添加业务逻辑（仅 wiring）

---

> **Specs 完成时间**：2026-06-22
> **下一步**：用户审查 → 通过后调用 `writing-plans` skill 输出实施计划 → Wave 1 并行启动
