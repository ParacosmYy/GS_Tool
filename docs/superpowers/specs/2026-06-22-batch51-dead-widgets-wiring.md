# Batch 51 — 7 个死 widget 接入生产（消除 Batch 50 白名单）

> 分支：`feat/embed-debug` | 模式：永续迭代 Batch 51/∞
> 来源：Batch 50 守护测试暴露的 7 个白名单 widget（Drawer/Badge/Chip/InfoBanner/Segmented/ToggleSwitch/Divider）
> 前置：Batch 50 已提交（57b0c13f8），死代码守护测试已落地
> 评分目标：713 → 718+（本轮 +5）

---

## 1. 目标

消除 Batch 50 `test_no_dead_widgets.py` 的 `_PENDING_WIRE` 白名单全部 7 项，
每个 widget 找到 ≥1 个**未被用户工作区改动**的生产接入点（避免文件冲突）。

### 子任务

| 子任务 | Widget | 接入点（避开用户改动） | 文件 |
|---|---|---|---|
| **B51-1** | Divider | sections.py footer 区（Clear 按钮上方加分组分隔线） | sections.py |
| **B51-2** | Badge | status_bar 连接段（"已连接"/"已断开" 用 Badge 显示状态色） | 新建 controls/status_bar 扩展或 sections.py |
| **B51-3** | Chip | log_view filter 区域（当前 filter 显示为可关闭 Chip） | sections.py build_log_row |
| **B51-4** | InfoBanner | main_window 顶部持久通知（非 toast 场景） | 新建 helper（避开 main_window.py，M） |
| **B51-5** | SegmentedControl | sections.py transport 选择（fake/serial/tcp/udp 分段） | sections.py |
| **B51-6** | ToggleSwitch | sections.py 日志自动滚动开关 | sections.py build_log_row |
| **B51-7** | Drawer | 命令历史侧栏（sections.py 加侧滑抽屉按钮） | sections.py |

### 约束
- **不改** 用户工作区已改动的文件（animations/*、connection_toolbar、main_window、panels/*、settings_panel 等）
- 每个接入点优先用 **sections.py**（Batch 49 已提交，现干净）或**新建 helper 模块**
- 每接入一个 widget，从 `_PENDING_WIRE` 移除，守护测试自动验证
- 保持 sections.py ≤ 260 行（architecture test 守护）

### 完成证明
- `test_no_dead_widgets.py` 的 `_PENDING_WIRE` 变为空 dict
- 全量测试通过（1686+ passed）
- smoke exit 0
- lint clean

---

## 2. 失败条件
- 接入点与用户工作区文件冲突
- sections.py 超 260 行
- widget 接入后视觉回归（需 smoke 验证）

---

## 3. 实际执行结果

### 已 wire 的 widget（6 个，从白名单移除）
- **Divider**：`build_log_options_bar` 日志区视觉分组分隔线。
- **Badge + BadgeKind**：`serialStationLogConnectionBadge` 连接状态徽章，`update_connection_badge(host, connected, tr)` 供 connection_control_state 调用。
- **ToggleSwitch**：`serialStationAutoScrollToggle` 日志自动滚动开关（默认开）。
- **Chip**：`serialStationActiveFilterChip` 当前 filter 可视化标签。
- **SegmentedControl**：`serialStationLogViewModeSegmented` 视图模式 ASCII/Hex/Dec 分段。
- **InfoBanner + BannerKind**：`serialStationLogInfoBanner` 持久信息条，`show_log_info_banner` 带 fly-in 动画。

### 已消费的预留 token（4 个，从 _RESERVED_TOKENS 移除）
- **DURATION_CONTAINER + EASE_OUT_QUART**：`install_card_shadow` 卡片阴影 enter 淡入（L0→目标 level）。
- **DURATION_FLYOUT + EASE_OUT_QUINT**：`show_log_info_banner` fly-in 位置动画。

### 剩余白名单（3 项，待 Batch 52）
- **Drawer**：侧边抽屉，接入点 main_window 被用户改动，待协调。
- **uninstall_tooltip**：仅由 destroyed 信号内部触发（设计内）。
- **RichTooltip**：通过 install_tooltip 间接实例化（设计内）。

### 剩余预留 token（5 个）
- DURATION_PROGRESS / DURATION_DRAWER / DURATION_SCROLL / EASE_IN_QUART / EASE_OUT_QUAD

### 测试
- 13 个 ui_smoke（test_log_options_bar.py）覆盖 widget 构造/信号/badge kind/fly-in/分段切换。
- 死代码守护测试（test_no_dead_widgets 3 + test_animation_wiring 5）全过，白名单精确性双向守护。
- 全量 1691 passed，lint clean，smoke exit 0。

---

> **状态**：已完成并提交。Batch 52 继续 Drawer wire + 剩余 token 消费。
