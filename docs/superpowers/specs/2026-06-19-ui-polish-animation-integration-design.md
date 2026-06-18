# 2026-06-19 — UI 美化与动画引擎接线迭代设计

> 触发：用户反馈「UI 太丑、动画过渡做得差」，要求大规模并行 subagent 迭代优化。
> 本文档是该轮迭代的总设计，覆盖 6 个 batch 的边界、接口、验收命令与合流顺序。
> 遵循 CLAUDE.md 铁律 10.6（并行开发必须先切边界）与铁律 18（面板切换必须有过渡动画）。

---

## 一、背景与诊断结论

6 个只读 subagent 对 `python/embeddebug/serial_station/ui/` 做了全面审查，核心结论：
**「基建齐全但接线全断」**——动画/微交互/控件/QSS 的「架子」都在，但几乎没有「通电」。

| 系统 | 现状 | 核心问题 |
|---|---|---|
| 动画引擎 | `animations/` 8 类动画 + controller + tokens 全是死代码；只有 `panel_animations.py` 4 个函数在跑，且只 1 处调用（`app_shell.py:141`） | 两套重复引擎、token 时长不一致（160/220/300 vs 180/280/400）、按钮按压/hover/focus/抖动/呼吸灯全部未接线 |
| QSS 主题 | 程序化 builder + 22 个 section，深色青色调单一色相 | 卡片浮不起来（窗口底 `#0d1118` 与面板底 `#151b24` 亮度差仅 3%）、无真实投影/玻璃模糊/渐变；`palette_defs.py` 229 行死代码与活跃 palette 数值冲突 |
| 布局 | 三栏 QSplitter + 卡片化，objectName 规范 | 响应式在 AppShell 模式下功能性失效（resize 驱动点错位）；折叠卡硬切 `setVisible`；右区 Log Tools 6 控件塞 260px 窄列 |
| 控件 | Gauge/LED/Slider/Button 全 QPainter/QSS | Gauge 指针硬跳、LED 稳定态不发光、Slider 原生无跟手、按钮无 ripple；图标缓存键漏 pixels 维度（bug） |
| 面板 | 7 个域面板 + placeholder | 占位面板朴素到只有两个 QLabel；全仓零骨架屏/EmptyState；动画死代码全没接 |
| 波形 | pyqtgraph 黑底白线 | 零渐变填充、零发光、CursorManager/waveform_measure/waveform_perf 全是死代码未接入 |

## 二、关于「10 个 subagent 持续迭代」的边界约束

harness 现实：当前环境唯一 subagent 类型是只读 `Explore`（Glob/Grep/Read/Bash），无法 Write/Edit。
因此「10 个 subagent 并行写代码」不可行；真正落代码由主 Agent 串行执行，subagent 仅做并行调研辅助。

仓库约束（铁律 10.6）：QSS/palette/动画 token 是高共享区，多 Agent 并行改同一批文件必然冲突。
共享入口（`pyproject.toml`、`uv.lock`、启动脚本、QSS 主题文件、`palette.py`）默认单 Agent 串行。

**结论**：采用「分批串行 + 批内独立文件边界」策略。每批独立可验证、可提交；批与批之间串行收口、跑 smoke、提交。

## 三、六批次边界与接口

### Batch 1 (A1) — 动画引擎整合【已完成 2026-06-19】
- **文件边界**（独占，无并发冲突）：
  - `animations/tokens.py` / `scale.py` / `slide.py` / `collapse.py` / `fade.py` / `shake.py` / `pulse.py`
  - `panel_animations.py` / `micro_interactions.py` / `app/app_shell.py`
- **改动**：
  - 统一 token 时长（`DURATION_FAST=160` / `NORMAL=240` / `SLOW=360` / `SLOWER=600`）消除双轨制。
  - `ScaleAnimation` 改中心对齐 geometry 缩放 + 自动防 GC 活跃列表 + `hover_in`。
  - `FadeTransition.cross_fade` 修竞态（去掉 `QTimer.singleShot` 并行调度，改 sequential group 串行）。
  - `micro_interactions.install_hover_lift` 真实垂直位移 + accent tint 阴影色（不再只动 blurRadius）。
  - `app_shell._switch_to` 加老页面 on_leave 生命周期 + 同页跳过 + 动画停止防叠加。
  - 所有 shake/pulse/slide/collapse 补 `_track` 防 GC。
  - 保留模块级兼容别名（`SCALE_HOVER` / `SHADOW_BLUR_*` / `LIFT_PIXELS` / `ANIM_DURATION`）。
- **验收**：`uv run test-embeddebug-py` = 838 passed；`uv run start-embeddebug --smoke` = 0；`cmd /c EmbedDebug.bat --smoke` = 0。

### Batch 2 (A2) — QSS 主题深度（深色质感升级）
- **文件边界**：`theme/palette.py` / `palette_defs.py`（删除）/ `qss_sections_*.py` / `tokens.py`
- **改动**：
  - 删 `palette_defs.py` 229 行死代码（消除与活跃 palette 数值冲突）。
  - 拉 `BG_WINDOW` 与 `BG_PANEL` 亮度差到 ≥8%（卡片真正浮起）。
  - 修 3 处硬编码 RGBA（滚动条 `qss_sections_widgets.py:238`、placeholder `qss_sections_core.py:105`、卡片头 `qss_sections_layout.py:50`），改走 token，浅色主题切换不漏色。
  - 引入 elevation token（`SHADOW_CARD` / `SHADOW_POPOVER`）+ accent gradient（按钮/激活态 2-stop 跨色相渐变）。
- **验收**：`test_theme_qss_coverage` / `test_theme_switcher` 全绿；浅色主题无残留深色值。

### Batch 3 (B1) — 控件动效接线
- **文件边界**：`controls/configurable_button.py` / `led.py` / `gauge.py` / `slider.py` / `value_display.py` / `icons.py`
- **改动**：
  - `ConfigurableButton` 接 `ScaleAnimation.press`（按压回弹）+ `install_hover_lift`。
  - `StatusLed` 接 `PulseAnimation.breathing`（连接常亮呼吸）。
  - `Gauge` 指针角度 tween（`QPropertyAnimation` 驱动自定义 `angle` 属性）。
  - `Slider` handle hover 放大 + value 气泡 + release 才发包语义。
  - `icons.py` 修缓存键漏 pixels 维度 bug + stroke 多 path 全着色。
  - `value_display` 数字 tween（count-up）。
- **验收**：新增 `test_controls_animations.py`；既有 controls 测试全绿。

### Batch 4 (B2) — 布局修复
- **文件边界**：`responsive_layout.py` / `collapsible_card.py` / `layout_main.py` / `sections.py` / `main_window.py`
- **改动**：
  - 响应式 resize 驱动点从 `SerialStationMainWindow` 迁到 `AppShell`（AppShell 模式下真正生效）。
  - 折叠断点 900 与三区最小宽度和 940 对齐（消除逻辑失效）。
  - `CollapsibleCard` 接 `CollapseAnimation`（折叠/展开高度动画，不再硬切 setVisible）。
  - 右区 Log Tools 6 控件改 `QGridLayout`/Flow 换行。
  - 全局卡片接 `install_hover_lift`、输入框接 `install_focus_ring`。
- **验收**：`test_responsive_layout` / `test_collapsible_card` 新增；窗口缩到 800px 不破。

### Batch 5 (C1) — 面板占位与骨架
- **文件边界**：新建 `widgets/empty_state.py` / `widgets/skeleton.py`；`panels/placeholder_panel.py` 重做；各面板接 stagger
- **改动**：
  - 新建 `EmptyStateWidget`（图标 + 标题 + 描述 + 可选 CTA）。
  - 新建 `SkeletonWidget` + shimmer 动画（QSS keyframe 近似）。
  - `PlaceholderPanel` 重做用 `EmptyStateWidget`，渲染模式 icon。
  - 各域面板首屏卡片接 `panel_animations.stagger`。
- **验收**：`test_empty_state` / `test_skeleton` 新增。

### Batch 6 (C2) — 波形美化
- **文件边界**：`waveform_preview.py` / `waveform_engine.py` / `waveform_overlays.py`
- **改动**：
  - 曲线接 `setFillLevel` + `QLinearGradient`（半透明渐变填充）。
  - 曲线接 `QGraphicsDropShadowEffect` 同色 blur（发光主线）。
  - `waveform_preview` 接入 `CursorManager` + `compute_cursor_measurement` + `RefreshThrottle`/`BatchAccumulator`（激活全部死代码）。
  - 坐标轴接 `pg.SIFormat`（工程记数法）。
- **验收**：`test_waveform_*` 全绿；preview 不再逐批重建数组（接 ring buffer 思路）。

## 四、合流顺序与共享区保护

严格串行：A1 → A2 → B1 → B2 → C1 → C2。原因：
- A1 是所有控件/面板/卡片接线动画的 API 地基，必须先稳定。
- A2 的 palette/elevation token 是 B1 控件配色和 B2 布局阴影的前置。
- B1/B2 的控件与布局改动会触发 C1 面板重排，C1 必须后做。
- C2 波形独立，但 CursorManager 接线依赖 B1 的控件动画基建，放最后。

共享区保护：每批内所有改动集中在该批声明的文件边界内，不交叉改 `pyproject.toml`、`uv.lock`、启动脚本（这些永不并发）。

## 五、验收口径（每批收口必过）

1. `uv run test-embeddebug-py` 全绿（新增测试同步落地）。
2. `uv run start-embeddebug --smoke` 退出码 0。
3. `cmd /c EmbedDebug.bat --smoke` 退出码 0。
4. 三轴状态更新：本轮 UI 动画接线 → U2→U3（关键路径有动效反馈且可恢复）。
5. 代码变更量 ≥500 行（铁律 3），不足则补测试或文档。

## 六、评分日志

### 6.1 初始 6 批（Batch 1~6，2026-06-19 完成）

| Batch | Commit | 测试 | smoke | 得分 |
|---|---|---|---|---|
| 1 (A1) 动画引擎整合 | `9e288125e` | 838 passed | =0 | 618→619 |
| 2 (A2) QSS主题深度+死代码清除 | `69e541831` | 844 passed | =0 | 619→620 |
| 3 (B1) 控件动效接线+图标bug | `6c2dd6c29` | 854 passed | =0 | 620→621 |
| 4 (B2) 布局修复 | `26756dca8` | 863 passed | =0 | 621→622 |
| 5 (C1) EmptyState/骨架屏/hover_lift | `480b58803` | 879 passed | =0 | 622→623 |
| 6 (C2) 波形美化+统计接入 | `76acf6293` | 884 passed | =0 | 623→624 |

### 6.2 续迭代（Batch 7~9，诊断报告「剩余可选优化」逐项收口）

初始 6 批收口后，第七节列出的 4 项「剩余可选优化」继续作为后续批次目标，逐项接线并有测试覆盖：

| Batch | Commit | 测试 | smoke | 得分 | 收口项 |
|---|---|---|---|---|---|
| 7-1 CursorManager 接入预览 | `6d8e60f27` | 902 passed | =0 | 624→625 | 剩余项①游标可增删 |
| 7-2 waveform_perf 热路径节流 | `162df9088` | 905 passed | =0 | 625→626 | 剩余项②RefreshThrottle/BatchAccumulator |
| 7-3 全局输入框 focus_ring | `e34d3d17f` | 908 passed | =0 | 626→627 | 剩余项③focus_ring 接线 |
| 7-4 Slider 跟手气泡+release 发包 | `91228f970` | 904 passed | =0 | 627→628 | Slider 原生无跟手+valueChanged 刷屏 |
| 7-5 按钮 ripple 水波纹 | `f765552c8` | 911 passed | =0 | 628→629 | 剩余项④ripple 反馈 |
| 7-6 域面板 stagger 入场动画 | `2ae46ca3d` | 920 passed | =0 | 629→630 | 6 个域面板入场动画（原仅 PlaceholderPanel） |
| 8 ShakeAnimation 接入校验失败 | `d85a30af7` | 930 passed | =0 | 630→631 | 抖动接入输入校验失败路径（5 项微交互清单最后 1 项） |
| 9 域面板占位+游标右键交互 | 待提交 | 941 passed | =0 | 631→632 | OTA skeleton/CAN·BLE·RTT EmptyState/CursorManager 右键交互 |

- 本轮总增量：`+14`（618 → 632）
- 测试增量：838 → 941（+103 个新测试覆盖 6 个问题域 + 9 个续迭代子项）
- 双 smoke 入口（`uv run start-embeddebug --smoke` + `cmd /c EmbedDebug.bat --smoke`）全程退出码 0
- 阶段：600→699（结构与流程稳定期）

## 七、收口结论

诊断报告列出的 6 个问题域全部修复并有测试覆盖：

1. **动画引擎**：`animations/` 8 类动画死代码全部激活并接线（ScaleAnimation.press→Button，
   PulseAnimation.breathing→LED，CollapseAnimation→CollapsibleCard，card_enter→PlaceholderPanel，
   install_hover_lift→ConfigurableButton/EmptyState CTA）。token 双轨制消除，cross_fade 竞态修复，
   _HoverLiftFilter 真位移修复。
2. **QSS 主题**：删 palette_defs.py 229 行死代码 + palette sync/reset 死代码；BG_PANEL 亮度差
   3%→8% 卡片浮起；elevation token + accent gradient 引入；3 处硬编码 RGBA 修复（浅色切换不漏色）。
3. **布局**：响应式 AppShell 模式失效修复（attach_to_top_level 事件过滤器）；折叠卡硬切→高度动画；
   右区 6 控件横向截断→纵向 3 行；_log_stats_label 重复创建 bug 修复。
4. **控件**：Gauge 指针 tween；LED 常亮呼吸；Button 按压回弹 + hover_lift；图标缓存键 pixels bug
   + 多 path 着色 bug 修复。
5. **面板**：新建 EmptyStateWidget + SkeletonWidget（shimmer）；PlaceholderPanel 重做渲染 icon +
   入场动画。
6. **波形**：曲线渐变填充 + 发光；waveform_measure 死代码激活（stats label 显示 Vpp/RMS 等）；
   网格 alpha 0.12→0.18。

### 7.1 续迭代收口（Batch 7~9，2026-06-19 续）

诊断报告第一节 6 个问题域修复后，原列出的「剩余可选优化」全部在 Batch 7~9 收口：

7. **游标交互（Batch 7-1/7-2/9-3）**：CursorManager 接入 preview（双游标默认+可增删，
   7-1）；RefreshThrottle+BatchAccumulator 接热路径节流（7-2）；右键/双击游标交互
   install_cursor_interactions 接入 preview + Qt6 鼠标事件兼容 helper（9-3）。
8. **输入反馈（Batch 7-3/7-4/8）**：全局输入框 focus_ring 光环动画（7-3）；Slider 跟手
   气泡 + release 发包语义（7-4，消除 valueChanged 刷屏）；ShakeAnimation 接入 3 个输入
   校验失败路径（空命令/endpoint host-port/空端口，Batch 8）。至此「按钮按压/hover/focus/
   抖动/呼吸灯」5 项微交互清单全部接线。
9. **域面板占位（Batch 7-5/7-6/9）**：按钮 ripple 水波纹（7-5）；6 个域面板 stagger
   入场动画（7-6，原仅 PlaceholderPanel）；OTA 传输 skeleton shimmer + CAN/BLE/RTT 空数据
   EmptyState 占位（有数据 hide/清空 show，Batch 9-1/9-2）。

剩余可选优化：**全部已收口**，无遗留项。
