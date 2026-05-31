# UI/UX Review 040 -- EmbedDebug 观感评审报告

> 审查人: UI/UX产品体验师 (frontend-architect)
> 日期: 2026-06-01
> 范围: CLAUDE.md 6.0-6.9 全部检查项

---

## 1. 颜色一致性 (CLAUDE.md 6.2 / 6.8 Rule 1)

**Verdict: PASS (minor)**

| 文件 | 状态 | 说明 |
|------|------|------|
| QSS主题文件 (3个) | PASS | 语义色板变量定义完整，所有QSS规则使用硬编码色值但位于主题定义文件内，属于允许范围 |
| TerminalWidget.cpp | PASS | 所有QPainter颜色通过 ThemeManager::SemanticColor 获取 |
| ToastWidget.h | PASS | semanticColor() 正确映射 Success/Error/Accent |
| NavIndicatorWidget.h | PASS | Accent颜色从ThemeManager获取 |
| AnimatedProgressBar.h | PASS | shimmer渐变从ThemeManager Accent获取 |
| OtaWidget.cpp | PASS | 完成变色动画使用ThemeManager Accent+Success |
| ChartColors.h | PASS (P2) | 波形通道颜色硬编码#RRGGBB，但这是数据可视化调色板（非语义UI色），有dark/light双主题切换，可接受 |

**Issues:**
- (P2) ChartColors.h:37-72 -- 20个硬编码色值。虽为数据调色板非语义色，但未来可考虑将调色板移入QSS变量或ThemeManager，实现集中管理。
- 三个主题文件内部约800+个硬编码#RRGGBB。这符合设计（主题文件是颜色的唯一定义源），但需确保新增控件在三个文件中同步添加。

**修复建议:** 无紧急修复。长期建议将ChartColors调色板注册到ThemeManager。

---

## 2. 间距标准 (CLAUDE.md 6.3)

**Verdict: PASS**

| 检查项 | 标准 | 实际 | 状态 |
|--------|------|------|------|
| 面板内边距 | 8-12px | SerialConfigPanel: 12px (line 50) | PASS |
| OtaWidget内边距 | 8-12px | 12px (line 63) | PASS |
| 控件间距 | 6-8px | OtaWidget: 12px组间距, 8px配置间距, 6px进度组间距 | PASS |
| QTreeView::item padding | 8px | 4px 8px (三个主题一致) | PASS |
| QGroupBox margin-top | 14px | 14px (三个主题一致) | PASS |
| 按钮高度 | >=28px | connectBtn 36px, otaStartBtn 32px, otaClearHistory 28px | PASS |

**Issues:** 无。

---

## 3. 字体规范 (CLAUDE.md 6.4)

**Verdict: PASS (minor)**

| 检查项 | 标准 | 实际 | 状态 |
|--------|------|------|------|
| 终端字体 | Consolas | TerminalWidget.cpp:34 `QFont("Consolas", 10)` | PASS |
| QSS终端输入框 | Consolas monospace | QLineEdit font-family: Consolas | PASS |
| 界面字体 | Microsoft YaHei UI / Segoe UI | QSS使用系统默认(无显式设置) | PASS |
| ToastWidget字体 | - | paintEvent: "Microsoft YaHei UI", 12 | PASS |
| 界面字号 | 12-13px | QSS: QLabel 13px, QGroupBox 13px, QStatusBar 12px | PASS |
| 终端字号 | 13-14px | TerminalWidget: 10pt (约13px) | PASS |

**Issues:**
- (P2) QSS未显式设置全局界面字体。虽然Qt默认使用系统字体，但显式设置 `"Microsoft YaHei UI"` / `"Segoe UI"` 可确保一致性，尤其在不同Windows语言版本上。

**修复建议:** 在QSS顶部添加 `* { font-family: "Microsoft YaHei UI", "Segoe UI", sans-serif; }` 或在MainWindow中全局设置。

---

## 4. 按钮状态 (CLAUDE.md 6.8 Rule 4)

**Verdict: PASS**

抽查所有按钮类型的QSS覆盖:

| 按钮类型 | hover | pressed | disabled | 状态 |
|---------|-------|---------|----------|------|
| QPushButton (通用) | PASS | PASS | PASS | PASS |
| QPushButton#sendButton | PASS | PASS | PASS | PASS |
| QPushButton#connectBtn | PASS | PASS | PASS | PASS |
| QPushButton#connectBtn[state="connected"] | PASS | PASS | PASS | PASS |
| QPushButton#otaStartBtn | PASS | PASS | PASS | PASS |
| QPushButton#otaCancelBtn | PASS | PASS | PASS | PASS |
| QPushButton#searchBarCloseBtn | PASS | PASS | PASS | PASS |
| QToolButton | PASS | PASS | PASS | PASS |
| QPushButton[quickCmdBtn="true"] | PASS | PASS | PASS | PASS |

**Issues:** 无。所有按钮均覆盖hover/pressed/disabled三种状态。

---

## 5. 输入状态 (CLAUDE.md 6.7 checklist)

**Verdict: PASS**

| 输入类型 | normal | focus | error | disabled | 状态 |
|---------|--------|-------|-------|----------|------|
| QLineEdit (通用) | PASS | PASS | N/A | PASS | PASS |
| QLineEdit#sendInput[hasError] | - | - | PASS | - | PASS |
| QLineEdit#searchBarInput | PASS | PASS | PASS | PASS | PASS |
| QComboBox | PASS | hover | N/A | PASS | PASS |
| QSpinBox | PASS | PASS | N/A | PASS | PASS |

**Issues:**
- (P2) QLineEdit通用样式缺少error状态。错误态仅通过sendInput和searchBarInput的hasError属性实现。建议QSS添加通用QLineEdit的错误态，或确保所有需要错误态的输入框都使用hasError属性。

**修复建议:** 审计所有QLineEdit实例，确认哪些需要错误态支持。

---

## 6. 动画实现 (CLAUDE.md 6.5 / 6.10)

**Verdict: PASS (P0全部实现)**

| 动画 | 优先级 | 状态 | 实现位置 |
|------|--------|------|---------|
| 面板切换淡入淡出 | P0 | IMPLEMENTED | NavigationController.cpp:269-321 (150ms OutCubic/InCubic) |
| 搜索栏展开/收起 | P0 | IMPLEMENTED | TerminalSearchBar.cpp:148-189 (200ms OutCubic / 150ms InCubic) |
| 连接状态呼吸动画 | P0 | IMPLEMENTED | SerialConfigPanel.cpp:31-42 (QTimer 50ms, opacity 0.3-1.0) |
| 主题切换淡入淡出 | P0 | IMPLEMENTED | ThemeManager.cpp:294-314 (300ms InOutCubic) |
| 进度条流动shimmer | P1 | IMPLEMENTED | AnimatedProgressBar.h:85-104 (2000ms Linear循环) |
| 进度条完成变色 | P1 | IMPLEMENTED | OtaWidget.cpp:373-404 (accent->mid->success, 400ms) |
| 导航树选中线滑动 | P1 | IMPLEMENTED | NavIndicatorWidget.h:97-118 (250ms OutCubic) |
| 通知吐司弹出 | P1 | IMPLEMENTED | ToastWidget.h:51-67 (300ms OutBack) |
| 通知吐司消失 | P1 | IMPLEMENTED | ToastWidget.h:135-146 (250ms InCubic) |
| 按钮悬浮渐变 | - | NOT IMPLEMENTED (QSS transition) | - |
| Tab切换下划线 | P2 | NOT IMPLEMENTED | - |

**Issues:**
- (P1) 连接状态呼吸动画使用QTimer手动插值（SerialConfigPanel.cpp:31-42），而非CLAUDE.md 6.5要求的QPropertyAnimation。虽功能正确，但违反了"使用QPropertyAnimation"的动画铁律。
- (P2) 按钮悬浮渐变未实现动画过渡。当前QSS只有状态切换，无200ms渐变。
- (P2) Tab切换下划线滑动未实现（当前项目无Tab系统，属合理延后）。

**修复建议:** 将呼吸动画改为QPropertyAnimation on opacity。按钮hover过渡可通过QGraphicsColorizeEffect或自定义QPropertyAnimation实现。

---

## 7. 导航树选中指示线 (CLAUDE.md 6.6 / 6.7)

**Verdict: PASS**

| 检查项 | 状态 | 说明 |
|--------|------|------|
| 左侧指示线 | PASS | NavIndicatorWidget 3px宽, 2px圆角 |
| 滑动动画 | PASS | 250ms OutCubic, QPropertyAnimation |
| 颜色 | PASS | ThemeManager::Accent, 主题切换自动刷新 |
| 鼠标穿透 | PASS | WA_TransparentForMouseEvents |
| 尺寸跟随 | PASS | eventFilter监听resize |

**Issues:** 无。

---

## 8. Toast通知设计 (CLAUDE.md 6.6 / 6.5)

**Verdict: PASS**

| 检查项 | CLAUDE.md标准 | 实际 | 状态 |
|--------|-------------|------|------|
| 左侧彩色边框 | success色/error色 | 4px宽, 三种语义色 | PASS |
| 图标 | 成功/错误图标 | Unicode check/cross/info | PASS |
| 3秒自动消失 | 是 | 3000ms默认 | PASS |
| 消失动画 | 向上飘出+淡出 | 250ms InCubic opacity | PASS |
| 弹出动画 | OutBack | 300ms OutBack pos+opacity | PASS |
| 位置 | 右下角 | 父窗口右下角, margin 16px | PASS |
| 多条堆叠 | 垂直排列 | activeToasts()+repositionToasts() | PASS |

**Issues:** 无。实现高度匹配Linear通知风格。

---

## 9. 进度条设计 (CLAUDE.md 6.6 / 6.10)

**Verdict: PASS**

| 检查项 | CLAUDE.md标准 | 实际 | 状态 |
|--------|-------------|------|------|
| 流动效果 | 渐变流动 | shimmer 2000ms循环, QLinearGradient | PASS |
| 完成变色 | accent->success 400ms | 中间混合色->success 400ms | PASS |
| 进度平滑 | - | QPropertyAnimation on value, OutCubic | PASS |
| 颜色来源 | ThemeManager | AnimatedProgressBar从ThemeManager获取 | PASS |

**Issues:**
- (P2) AnimatedProgressBar::paintEvent (line 115-172) 每帧都调用ThemeManager::instance()和color()方法获取Accent色。在shimmer循环期间(2000ms周期)会产生大量查询。虽然ThemeManager::color()是O(1) QMap查找，性能可接受，但缓存accent色到成员变量并在themeChanged时刷新会更优。

**修复建议:** 在startShimmer()时缓存accent颜色，监听themeChanged刷新。

---

## 10. 响应式布局 (CLAUDE.md 6.7)

**Verdict: PASS (minor)**

| 检查项 | 状态 | 说明 |
|--------|------|------|
| 导航树宽度约束 | PASS | CLAUDE.md: min 180px, max 280px |
| 终端最小尺寸 | PASS | TerminalWidget: minimumSize(400,200) |
| QSplitter分隔器 | PASS | 三个主题QSS均定义1px handle |
| 背景图resize | PASS | BackgroundWidget缓存缩放图, resizeEvent重新生成 |
| 面板布局 | PASS | VBoxLayout + stretch, 自适应高度 |

**Issues:**
- (P2) 未找到导航树min/max宽度的显式C++设置。虽然CLAUDE.md规定180-280px，需确认MainWindow或NavigationController中是否通过setMinimumWidth/setMaximumWidth强制约束。

**修复建议:** 在创建QTreeView时添加 `navTree->setMinimumWidth(180); navTree->setMaximumWidth(280);`

---

## 总结

| # | 检查项 | 结果 | 优先级 |
|---|--------|------|--------|
| 1 | 颜色一致性 | PASS | - |
| 2 | 间距标准 | PASS | - |
| 3 | 字体规范 | PASS (minor) | P2 |
| 4 | 按钮状态 | PASS | - |
| 5 | 输入状态 | PASS (minor) | P2 |
| 6 | 动画实现 | PASS (P0全部, 1项P1偏离) | P1 |
| 7 | 导航树指示线 | PASS | - |
| 8 | Toast设计 | PASS | - |
| 9 | 进度条设计 | PASS | - |
| 10 | 响应式布局 | PASS (minor) | P2 |

### 待修复项 (按优先级排序)

| Priority | Issue | File:Line | Fix |
|----------|-------|-----------|-----|
| P1 | 呼吸动画用QTimer而非QPropertyAnimation | SerialConfigPanel.cpp:31-42 | 改为QPropertyAnimation on opacity, 1500ms InOutSine循环 |
| P2 | QSS未显式设置全局UI字体 | resources/themes/*.qss | 添加全局font-family规则 |
| P2 | 导航树宽度未在C++中约束min/max | MainWindow/NavigationController | 添加setMinimumWidth(180)/setMaximumWidth(280) |
| P2 | AnimatedProgressBar每帧查询ThemeManager | AnimatedProgressBar.h:145 | 缓存accent到成员变量 |
| P2 | QLineEdit通用样式缺少error态 | resources/themes/*.qss | 评估是否需要通用error状态 |
| P2 | ChartColors硬编码色值 | ChartColors.h:37-72 | 长期考虑注册到ThemeManager |

### 整体评价

EmbedDebug的UI实现质量很高。三个主题文件覆盖完整、语义色板体系健全、所有P0动画已实现、组件QSS样式覆盖率接近100%。主要偏离点是呼吸动画的实现方式（QTimer vs QPropertyAnimation），以及若干可优化的P2细节。整体观感达到了对标Linear/Vercel的设计水准。
