# PRD-212 Serial Station 体验增强（动画 + 波形游标 + 响应式 + 命令面板）

## 目标

把 B1（主题）+ B2（三区布局）之后的体验差距补齐，对齐 VOFA+ 波形引擎与
VS Code 级交互体验，达成"比 VOFA+ 精美"的体验完整度。本轮交付四项：
面板过渡动画、波形游标与多通道图例、响应式断点、命令面板（Ctrl+P）。

## 范围

- 新增 `ui/command_palette.py`：VS Code 风格命令面板。半透明遮罩 + 居中卡片 +
  模糊匹配列表；子序列评分排序（完全匹配最高、连续命中加分），Enter 执行、Esc 关闭。
- 新增 `ui/waveform_overlays.py`：波形游标 + 多通道图例。
  - `attach_cursors` 给绘图区附加两条可拖拽 InfiniteLine（强调青/警告黄）。
  - `cursor_readout` 计算 ΔX 与两游标处 Y 值（首通道插值）。
  - `WaveformLegend` 多通道色块 + 名称 + 当前值。
  - `build_cursor_hud` 游标读数 HUD。
- 新增 `ui/panel_animations.py`：QPropertyAnimation 过渡动画工厂。
  fade_in/fade_out（QGraphicsOpacityEffect）、slide_in（pos）、card_enter（组合）、
  stagger（错峰）。时长 160~300ms，缓动 OutCubic。
- 新增 `ui/responsive_layout.py`：响应式断点控制器。
  窗口 <900 折叠左区侧栏（splitter sizes 归零），>=980 恢复（滞后避免抖动）。
  发出 sidebar_collapsed/sidebar_expanded 信号供 UI 做过渡。
- 改动 `ui/shortcuts.py`：新增 Ctrl+P → `_open_command_palette`，QShortcut
  objectName `serialStationCommandPaletteShortcut`。
- 改动 `ui/waveform_preview.py`：集成游标、图例、HUD，update_batch 时刷新。
- 改动 `ui/main_window.py`：装配命令面板（注册 8 条命令）、响应式布局、
  resizeEvent 驱动断点、`_open_command_palette` 方法。
- QSS 新增 command_palette/legend/cursor_hud 分区。

## 非目标

- 不改任何 `serialStation*` 既有 objectName（架构测试约束）。
- 不改 action 模块委托边界。
- 不动 controller/core/protocols/services 业务逻辑。
- 不引入新第三方依赖。

## 设计决策

### Ctrl+P 走 shortcuts.handle_key_press

架构测试要求 `main_window.keyPressEvent` 源码不含 `Qt.Key`，因此 Ctrl+P
在 `shortcuts.handle_key_press` 内分发，通过 `SerialStationShortcutHost._open_command_palette`
回调，保持 keyPressEvent 只做 `shortcuts.handle_key_press(self, event)` 委托。

### 响应式用滞后阈值

折叠阈值 900、展开阈值 980，避免在边界附近反复折叠/展开抖动。
折叠只调 splitter sizes 不销毁控件，保证 findChild 仍可达。

### 游标用 pyqtgraph InfiniteLine

可拖拽、带值标签，颜色取 palette 强调色与警告色，对齐 VOFA+ 双游标测量。
读数 HUD 显示 ΔX/Y1/Y2，图例显示通道色块与当前值。

## 三轴状态

| 轴 | 本轮状态 | 说明 |
|---|---|---|
| 工程 | `E4` | 四项体验增强落地，31 个新单测覆盖模糊匹配/游标/图例/动画/断点，全门禁通过 |
| 用户 | `U4` | Ctrl+P 命令面板、双游标测量、多通道图例、响应式折叠、面板过渡动画均可见可用 |
| 设备 | `D2` | 维持替身/loopback 验证口径，真实设备未验证 |

## 验收

- `uv run test-embeddebug-py` → 281 测试全绿。
- `uv run start-embeddebug --smoke` → 退出码 0。
- `cmd /c EmbedDebug.bat --smoke` → 退出码 0。
- QSS 覆盖率守护测试自动覆盖新增 objectName。
- 架构测试全绿（shortcuts.handle_key_press 委托不变、keyPressEvent 不含 Qt.Key）。

## 架构检查

- 新增模块只依赖 PyQt6 + pyqtgraph + 标准库 + theme，不 import controller/core/protocol/service。
- shortcuts.py 扩展协议方法 `_open_command_palette`，handle_key_press 增加 Ctrl+P 分支。
- main_window 新增 resizeEvent 驱动响应式，_open_command_palette 打开面板。
- waveform_preview 集成 overlays，update_batch 时刷新游标/图例/HUD。

## 完成口径

本轮完成后，"比 VOFA+ 精美"的体验维度（主题 + 布局 + 图标 + 卡片 + 动画 +
波形游标 + 多通道图例 + 响应式 + 命令面板）全部落地，主入口启动即可呈现
完整的深色工业风三区卡片化波形调试工站。
