# PRD-015: 过渡动画系统实现

## 背景

CLAUDE.md 6.5 过渡动画规范明确定义了 14 项动画清单，按优先级分为 P0（必须实现）、P1（应该实现）、P2（可以延后）三级。当前代码库中，仅 `TerminalSearchBar` 的展开/收起动画已实现（符合规范），其余动画均未实现。

缺失动画导致的具体体验问题:

1. **面板切换生硬** -- 点击导航树切换面板时，旧面板瞬间消失、新面板瞬间出现，用户无法感知位置变化关系，与 Linear/Arc 的面板切换体验差距巨大。
2. **连接状态静止** -- 连接中（`ConnectionState::Connecting`）状态下状态指示点为静态黄色，用户无法区分"正在连接"和"卡死"，违背 CLAUDE.md 6.6 状态栏规范中的"呼吸动画"要求。
3. **主题切换闪烁** -- 切换暗色/亮色主题时全局样式瞬间切换，在极端色差下造成视觉闪烁。
4. **OTA进度平淡** -- 传输进行中进度条无流动效果，传输完成无变色反馈，与 Vercel 风格的进度体验不符。
5. **导航选中无动效** -- 点击不同导航项时左侧指示线直接跳变到新位置，缺少 Arc 侧边栏级别的滑动感。

本 PRD 覆盖 P0 级全部 4 项动画和 P1 级 2 项动画，合计 6 项动画需求的完整规格定义。

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | 面板滑入/滑出动画 -- 导航树切换面板时的过渡效果 | P0 | core/MainWindow |
| R2 | 搜索栏展开/收起动画 -- Ctrl+F / Esc 触发 | P0 | terminal/TerminalSearchBar |
| R3 | 连接状态呼吸脉冲动画 -- Connecting 状态指示灯 | P0 | core/MainWindow |
| R4 | 主题切换淡入淡出过渡 -- 暗/亮主题切换 | P0 | core/MainWindow, core/ThemeManager |
| R5 | OTA进度条流动动画 -- 传输进行中的渐变流动效果 | P1 | ota/OtaWidget |
| R6 | 导航树选中指示线滑动 -- 点击不同导航项时左侧指示线滑动 | P1 | core/MainWindow |

## 各动画详细规格

---

### R1: 面板滑入/滑出动画（P0）

#### 触发条件

用户点击左侧导航树（`m_navTree`）中的功能节点时，右侧内容区的当前面板需要滑出，新面板需要滑入。触发点为 `MainWindow::connectSignals()` 中 `m_navTree::clicked` 的 lambda 处理。

#### 当前行为分析

当前实现在 `MainWindow.cpp` 第 396-436 行，通过 `setVisible(w == target)` 直接切换面板可见性，无任何过渡效果。旧面板瞬间隐藏，新面板瞬间显示。

#### 动画规格

| 属性 | 滑入（展开） | 滑出（收起） |
|------|------------|------------|
| 目标属性 | `geometry`（水平位移）+ `windowOpacity`（可选淡入） | `geometry`（水平位移）+ `windowOpacity`（可选淡出） |
| 起始值 | x 偏移 +40px（从右侧略微偏移），opacity = 0.0 | x 偏移 0，opacity = 1.0 |
| 结束值 | x 偏移 0（回到原位），opacity = 1.0 | x 偏移 -40px（向左侧略微偏移），opacity = 0.0 |
| 持续时间 | 250ms | 200ms |
| 缓动曲线 | `QEasingCurve::OutCubic` | `QEasingCurve::InCubic` |
| 循环 | 否 | 否 |

#### 实现方案

采用 `QPropertyAnimation` + `QGraphicsOpacityEffect` 组合方案。由于当前面板在同一个 QVBoxLayout 中通过 `setVisible` 切换，动画方案需要调整面板切换逻辑:

1. **记录当前面板的原始 geometry**
2. **旧面板滑出**: 创建 `QPropertyAnimation` 对 `pos` 属性（向左偏移）+ `QGraphicsOpacityEffect` 对 `opacity` 属性（淡出），动画结束后 `hide()` 旧面板并恢复原始位置和透明度
3. **新面板滑入**: 先 `show()` 新面板并设置初始偏移和透明度，然后创建 `QPropertyAnimation` 对 `pos` 属性（从右侧滑入）+ `QGraphicsOpacityEffect` 对 `opacity` 属性（淡入）
4. **时序**: 旧面板滑出和新面板滑入使用 `QSequentialAnimationGroup` 串联执行，先滑出再滑入

#### 集成点

| 类/方法 | 修改内容 |
|---------|---------|
| `MainWindow` 类声明 | 新增 `QPropertyAnimation* m_slideOutAnim`、`QPropertyAnimation* m_slideInAnim` 成员；新增 `void animatePanelSwitch(QWidget* oldPanel, QWidget* newPanel)` 私有方法 |
| `MainWindow::connectSignals()` | 在 `m_navTree::clicked` lambda 中，用 `animatePanelSwitch()` 替换当前的 `setVisible()` 循环 |
| `MainWindow::onConnectionStateChanged()` | 连接成功后切换面板时同样调用 `animatePanelSwitch()` |

#### 伪代码

```cpp
void MainWindow::animatePanelSwitch(QWidget* oldPanel, QWidget* newPanel)
{
    // 1. 如果目标面板已经是当前面板，无需动画
    if (oldPanel == newPanel) return;

    // 2. 旧面板滑出动画
    if (oldPanel && oldPanel->isVisible()) {
        QGraphicsOpacityEffect* fadeOut = new QGraphicsOpacityEffect(oldPanel);
        oldPanel->setGraphicsEffect(fadeOut);

        QPropertyAnimation* slideOut = new QPropertyAnimation(oldPanel, "pos");
        slideOut->setStartValue(oldPanel->pos());
        slideOut->setEndValue(oldPanel->pos() + QPoint(-40, 0));
        slideOut->setDuration(200);
        slideOut->setEasingCurve(QEasingCurve::InCubic);

        QPropertyAnimation* opacityOut = new QPropertyAnimation(fadeOut, "opacity");
        opacityOut->setStartValue(1.0);
        opacityOut->setEndValue(0.0);
        opacityOut->setDuration(200);
        opacityOut->setEasingCurve(QEasingCurve::InCubic);

        connect(slideOut, &QPropertyAnimation::finished, oldPanel, [oldPanel, fadeOut]() {
            oldPanel->hide();
            oldPanel->move(oldPanel->pos() + QPoint(40, 0)); // 恢复位置
            fadeOut->setOpacity(1.0);
        });

        slideOut->start(QAbstractAnimation::DeleteWhenStopped);
        opacityOut->start(QAbstractAnimation::DeleteWhenStopped);
    }

    // 3. 新面板滑入动画
    if (newPanel) {
        newPanel->show();
        newPanel->raise();

        QGraphicsOpacityEffect* fadeIn = new QGraphicsOpacityEffect(newPanel);
        newPanel->setGraphicsEffect(fadeIn);
        fadeIn->setOpacity(0.0);

        QPropertyAnimation* slideIn = new QPropertyAnimation(newPanel, "pos");
        QPoint finalPos = newPanel->pos();
        slideIn->setStartValue(finalPos + QPoint(40, 0));
        slideIn->setEndValue(finalPos);
        slideIn->setDuration(250);
        slideIn->setEasingCurve(QEasingCurve::OutCubic);

        QPropertyAnimation* opacityIn = new QPropertyAnimation(fadeIn, "opacity");
        opacityIn->setStartValue(0.0);
        opacityIn->setEndValue(1.0);
        opacityIn->setDuration(250);
        opacityIn->setEasingCurve(QEasingCurve::OutCubic);

        slideIn->start(QAbstractAnimation::DeleteWhenStopped);
        opacityIn->start(QAbstractAnimation::DeleteWhenStopped);
    }
}
```

#### 注意事项

- `QGraphicsOpacityEffect` 设置在 widget 上后，如果该 widget 在动画完成后被 delete，需要确保 effect 也被清理（Qt 父子树自动管理）
- 面板在 QVBoxLayout 中，`pos()` 动画可能与布局管理器冲突。替代方案是使用 `QParallelAnimationGroup` 同时驱动 `geometry` 动画和 opacity 动画，在动画期间临时 `setEnabled(false)` 布局，动画结束后恢复
- 如果 `pos` 动画与布局冲突严重，可降级为纯 opacity 淡入淡出方案（仅用 `QGraphicsOpacityEffect`），放弃位移效果

---

### R2: 搜索栏展开/收起动画（P0）

#### 当前状态

**已实现**。`TerminalSearchBar::activate()` 和 `TerminalSearchBar::deactivate()` 已包含完整的 `QPropertyAnimation` 动画，规格完全符合 CLAUDE.md 6.5 要求:

- 展开: `maximumHeight` 0 -> 36, 200ms, `OutCubic`
- 收起: `maximumHeight` 36 -> 0, 150ms, `InCubic`

#### 剩余工作

无需修改。本项标记为"已完成"，在验收阶段确认动画效果即可。

#### 集成点

| 类/方法 | 状态 |
|---------|------|
| `TerminalSearchBar::activate()` | 已实现展开动画 |
| `TerminalSearchBar::deactivate()` | 已实现收起动画 |

---

### R3: 连接状态呼吸脉冲动画（P0）

#### 触发条件

当连接状态变为 `ConnectionState::Connecting` 时，状态栏左侧的连接状态指示点（彩色圆点）开始呼吸脉冲动画。状态变为 `Connected`、`Disconnected` 或 `Error` 时停止动画。

#### 当前行为分析

`MainWindow::onConnectionStateChanged()` 在 `ConnectionState::Connecting` 时仅设置文字为"连接中..."，通过动态属性 `state="connecting"` 由 QSS 设置黄色文字。没有独立的指示点控件，也没有呼吸动画。

#### 动画规格

| 属性 | 值 |
|------|-----|
| 目标属性 | 自定义 `Q_PROPERTY` `indicatorOpacity`（double: 0.3 ~ 1.0） |
| 起始值 | 0.3 |
| 结束值 | 1.0 |
| 持续时间 | 1500ms |
| 缓动曲线 | `QEasingCurve::InOutSine` |
| 循环 | 无限循环，直到状态离开 Connecting |

#### 实现方案

**方案: 独立状态指示点控件 + 自定义属性动画**

1. 在 `MainWindow::setupStatusBar()` 中，在 `m_connStatusLbl` 左侧新增一个 `QWidget`（`m_connIndicator`），尺寸固定 10x10，圆角 5px，作为彩色状态指示点
2. 为该控件注册自定义属性 `indicatorOpacity`，通过 `Q_PROPERTY` 宏声明，在 setter 中动态更新背景色的 alpha 通道
3. 连接状态为 Connecting 时，创建 `QPropertyAnimation` 对 `indicatorOpacity`，设置 `setLoopCount(-1)`（无限循环），起始值 0.3，结束值 1.0，持续时间 1500ms，缓动曲线 `InOutSine`
4. 状态变化离开 Connecting 时，停止动画并将 opacity 设为 1.0（实色显示）

#### 集成点

| 类/方法 | 修改内容 |
|---------|---------|
| `MainWindow` 类声明 | 新增 `QWidget* m_connIndicator` 成员；新增 `QPropertyAnimation* m_breathAnim` 成员 |
| `MainWindow::setupStatusBar()` | 新建 10x10 圆角指示点控件，插入到 `m_connStatusLbl` 左侧 |
| `MainWindow::onConnectionStateChanged()` | Connecting 时启动呼吸动画，其他状态时停止动画并设为实色 |

#### 伪代码

```cpp
// MainWindow.h 新增:
class MainWindow : public QMainWindow {
    Q_OBJECT
    // ...
private:
    QWidget* m_connIndicator;          // 连接状态指示点
    QPropertyAnimation* m_breathAnim;  // 呼吸动画
};

// MainWindow.cpp setupStatusBar():
m_connIndicator = new QWidget(this);
m_connIndicator->setObjectName("connIndicator");
m_connIndicator->setFixedSize(10, 10);
// 圆角通过QSS实现: border-radius: 5px; background-color: var(--warning);
statusBar()->addWidget(m_connIndicator);
statusBar()->addWidget(m_connStatusLbl, 1);

// MainWindow.cpp onConnectionStateChanged():
case ConnectionState::Connecting:
    // 启动呼吸动画
    if (!m_breathAnim) {
        m_breathAnim = new QPropertyAnimation(m_connIndicator, "indicatorOpacity");
        m_breathAnim->setStartValue(0.3);
        m_breathAnim->setEndValue(1.0);
        m_breathAnim->setDuration(1500);
        m_breathAnim->setEasingCurve(QEasingCurve::InOutSine);
        m_breathAnim->setLoopCount(-1); // 无限循环
    }
    m_breathAnim->start();
    break;

// 其他状态:
default:
    if (m_breathAnim) {
        m_breathAnim->stop();
    }
    m_connIndicator->setOpacity(1.0); // 恢复实色
    break;
```

#### 指示点自定义控件实现

由于 `QWidget` 没有内置的 `indicatorOpacity` 属性，需要创建一个自定义 widget 子类或使用 `QGraphicsOpacityEffect`:

**推荐方案（使用 QGraphicsOpacityEffect）**: 不需要自定义子类。对 `m_connIndicator` 设置 `QGraphicsOpacityEffect`，动画直接驱动 effect 的 `opacity` 属性。

```cpp
// setupStatusBar 中:
QGraphicsOpacityEffect* indicatorEffect = new QGraphicsOpacityEffect(m_connIndicator);
indicatorEffect->setOpacity(1.0);
m_connIndicator->setGraphicsEffect(indicatorEffect);

// onConnectionStateChanged Connecting 时:
QPropertyAnimation* breathAnim = new QPropertyAnimation(indicatorEffect, "opacity");
breathAnim->setStartValue(0.3);
breathAnim->setEndValue(1.0);
breathAnim->setDuration(1500);
breathAnim->setEasingCurve(QEasingCurve::InOutSine);
breathAnim->setLoopCount(-1);
breathAnim->start(QAbstractAnimation::DeleteWhenStopped);
m_breathAnim = breathAnim;

// 离开 Connecting 时:
if (m_breathAnim) {
    m_breathAnim->stop();
    m_breathAnim->deleteLater();
    m_breathAnim = nullptr;
    indicatorEffect->setOpacity(1.0);
}
```

#### 状态颜色映射（QSS）

指示点颜色通过 QSS 动态属性选择器控制:

```css
/* dark_terminal.qss */
QWidget#connIndicator { border-radius: 5px; }
QWidget#connIndicator[state="connected"]    { background-color: #a6e3a1; }  /* success */
QWidget#connIndicator[state="connecting"]   { background-color: #f9e2af; }  /* warning */
QWidget#connIndicator[state="disconnected"] { background-color: #6c7086; }  /* muted */
QWidget#connIndicator[state="error"]        { background-color: #f38ba8; }  /* error */
```

---

### R4: 主题切换淡入淡出过渡（P0）

#### 触发条件

用户通过工具栏主题下拉框切换主题时（`MainWindow::onThemeChanged()`），整个窗口内容需要平滑过渡，不能瞬间闪烁。

#### 当前行为分析

`ThemeManager::loadTheme()` 直接调用 `qApp->setStyleSheet(qss)` 全局替换样式表，所有控件瞬间重绘为新主题颜色。在暗色到亮色切换时，极端的亮度跳变对眼睛造成不适。

#### 动画规格

| 属性 | 值 |
|-----|-----|
| 目标属性 | `QGraphicsOpacityEffect` 的 `opacity` |
| 起始值 | 1.0 |
| 中间值 | 0.0（淡出到透明） |
| 结束值 | 1.0（淡入到不透明） |
| 持续时间 | 淡出 150ms + 淡入 150ms = 总计 300ms |
| 缓动曲线 | `QEasingCurve::InOutCubic` |
| 循环 | 否 |

#### 实现方案

**窗口级淡出-换肤-淡入方案**:

1. 在 `MainWindow` 的中央控件上设置 `QGraphicsOpacityEffect`
2. 主题切换时，先播放淡出动画（opacity 1.0 -> 0.0，150ms）
3. 淡出完成后，调用 `ThemeManager::loadTheme()` 切换 QSS
4. 切换完成后，播放淡入动画（opacity 0.0 -> 1.0，150ms）

#### 集成点

| 类/方法 | 修改内容 |
|---------|---------|
| `MainWindow` 类声明 | 新增 `QGraphicsOpacityEffect* m_themeTransitionEffect` 成员 |
| `MainWindow::setupUI()` | 为中央控件 `m_mainSplitter` 创建并设置 opacity effect |
| `MainWindow::onThemeChanged()` | 改为先淡出、换肤、再淡入的异步流程 |

#### 伪代码

```cpp
// MainWindow.h 新增:
QGraphicsOpacityEffect* m_themeTransitionEffect;

// MainWindow::setupUI() 末尾:
m_themeTransitionEffect = new QGraphicsOpacityEffect(m_mainSplitter);
m_mainSplitter->setGraphicsEffect(m_themeTransitionEffect);

// MainWindow::onThemeChanged():
void MainWindow::onThemeChanged(int index)
{
    QString themeName = m_themeCombo->itemData(index).toString();
    if (themeName.isEmpty()) return;

    // 步骤1: 淡出动画
    QPropertyAnimation* fadeOut = new QPropertyAnimation(m_themeTransitionEffect, "opacity");
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setDuration(150);
    fadeOut->setEasingCurve(QEasingCurve::InOutCubic);

    // 步骤2: 淡出完成后切换主题
    connect(fadeOut, &QPropertyAnimation::finished, this, [this, themeName]() {
        ThemeManager::instance().loadTheme(themeName);

        // 步骤3: 淡入动画
        QPropertyAnimation* fadeIn = new QPropertyAnimation(m_themeTransitionEffect, "opacity");
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->setDuration(150);
        fadeIn->setEasingCurve(QEasingCurve::InOutCubic);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    });

    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
}
```

#### 注意事项

- `QGraphicsOpacityEffect` 设置在顶层 splitter 上会影响所有子控件的整体透明度，这是期望行为
- 动画期间应禁用主题下拉框的再次切换，防止动画叠加冲突（`m_themeCombo->setEnabled(false)` 在动画开始时，`true` 在淡入完成后恢复）
- 首次启动加载主题时不需要过渡动画（`loadSettings()` 中的主题加载应跳过动画逻辑），可通过标志位 `m_initialThemeLoad` 控制

---

### R5: OTA进度条流动动画（P1）

#### 触发条件

OTA传输开始后（`OtaWidget::onStartTransfer()` -> `setTransferring(true)`），进度条显示流动渐变效果。传输完成时（`OtaWidget::onTransferComplete()`），进度条从 accent 色渐变为 success 色。

#### 当前行为分析

`OtaWidget` 中的 `m_progressBar`（`QProgressBar`）使用默认样式，无流动效果。传输完成时也无视觉变化反馈。

#### 动画规格

| 动画阶段 | 目标属性 | 起始值 | 结束值 | 持续时间 | 缓动曲线 |
|---------|---------|--------|--------|---------|---------|
| 传输中流动 | 自定义 `Q_PROPERTY` `gradientOffset`（渐变偏移量） | 0.0 | 1.0 | 2000ms | `Linear` |
| 完成变色 | 自定义 `Q_PROPERTY` `progressColor`（进度条填充色） | accent 色 | success 色 | 400ms | `OutCubic` |

#### 实现方案

**自定义 QProgressBar 子类方案**:

1. 创建 `AnimatedProgressBar` 类，继承 `QProgressBar`
2. 注册自定义属性 `gradientOffset`（qreal，范围 0.0~1.0）
3. 重写 `paintEvent()`，使用 `QPainter` 绘制带有偏移渐变效果的进度条填充
4. 传输进行中: 启动 `gradientOffset` 的循环动画（0.0->1.0，2000ms，Linear，无限循环）
5. 传输完成: 停止流动动画，启动颜色过渡动画（accent->success，400ms，OutCubic）

#### 集成点

| 类/方法 | 修改内容 |
|---------|---------|
| 新增 `AnimatedProgressBar` | `ota/AnimatedProgressBar.h/cpp`，QProgressBar 子类 |
| `OtaWidget` 类声明 | 将 `QProgressBar* m_progressBar` 替换为 `AnimatedProgressBar*` |
| `OtaWidget::setTransferring(true)` | 启动流动动画 |
| `OtaWidget::onTransferComplete()` | 启动完成变色动画 |
| `OtaWidget::onTransferError()` | 停止动画，恢复默认 |
| `OtaWidget::onCancelTransfer()` | 停止动画，恢复默认 |

#### 伪代码

```cpp
// AnimatedProgressBar.h
class AnimatedProgressBar : public QProgressBar {
    Q_OBJECT
    Q_PROPERTY(qreal gradientOffset READ gradientOffset WRITE setGradientOffset)
    Q_PROPERTY(QColor progressColor READ progressColor WRITE setProgressColor)
public:
    explicit AnimatedProgressBar(QWidget* parent = nullptr);

    qreal gradientOffset() const;
    void setGradientOffset(qreal offset);

    QColor progressColor() const;
    void setProgressColor(const QColor& color);

    void startFlowAnimation();   // 启动流动效果
    void stopFlowAnimation();    // 停止流动效果
    void animateComplete();      // 完成变色动画

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    qreal m_gradientOffset = 0.0;
    QColor m_progressColor;          // 当前填充色，从ThemeManager获取默认accent色
    QPropertyAnimation* m_flowAnim = nullptr;
};
```

```cpp
// AnimatedProgressBar.cpp paintEvent 核心逻辑
void AnimatedProgressBar::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制背景轨道
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor()); // bg-tertiary
    painter.drawRoundedRect(rect(), 3, 3);

    // 计算进度填充区域
    qreal ratio = value() / static_cast<qreal>(maximum());
    int fillWidth = static_cast<int>(width() * ratio);
    QRect fillRect(0, 0, fillWidth, height());

    // 绘制渐变填充（带偏移的线性渐变）
    QLinearGradient gradient(fillRect.topLeft(), fillRect.topRight());
    QColor baseColor = m_progressColor;
    QColor lightColor = baseColor.lighter(130);

    // gradientOffset 控制渐变高亮区域的位置
    qreal highlightPos = m_gradientOffset;
    gradient.setColorAt(0.0, baseColor);
    gradient.setColorAt(qMax(0.0, highlightPos - 0.15), baseColor);
    gradient.setColorAt(highlightPos, lightColor);
    gradient.setColorAt(qMin(1.0, highlightPos + 0.15), baseColor);
    gradient.setColorAt(1.0, baseColor);

    painter.setBrush(gradient);
    painter.drawRoundedRect(fillRect, 3, 3);
}
```

#### 进度条颜色获取

流动动画的基础色从 `ThemeManager` 获取语义色 accent 值。完成变色时从 accent 过渡到 success 色。这些颜色值在 QSS 中定义，C++ 代码通过解析当前 QSS 或 `QApplication::palette()` 获取（与 PRD-014 的迁移原则保持一致，不在 C++ 中硬编码色值）。

---

### R6: 导航树选中指示线滑动（P1）

#### 触发条件

用户点击导航树中的不同项时，左侧的选中指示线（3px 宽的 accent 色竖线）从旧选中项的 Y 位置平滑滑动到新选中项的 Y 位置。

#### 当前行为分析

当前导航树选中效果完全依赖 QSS 的 `::branch:selected` 和 QStandardItemModel 的选中机制，没有自定义的左侧指示线。选中态切换时背景色瞬间变化，无滑动效果。

#### 动画规格

| 属性 | 值 |
|------|-----|
| 目标属性 | 自定义绘制层的 `m_indicatorY`（指示线顶部 Y 坐标） |
| 起始值 | 旧选中项的 Y 坐标 |
| 结束值 | 新选中项的 Y 坐标 |
| 持续时间 | 250ms |
| 缓动曲线 | `QEasingCurve::OutCubic` |
| 循环 | 否 |

#### 实现方案

**自定义 QTreeView 子类 + Overlay 指示线方案**:

1. 创建 `NavigationTreeView` 类，继承 `QTreeView`
2. 重写 `paintEvent()`，在父类绘制完成后，用 `QPainter` 在视口左侧绘制 3px 宽的 accent 色竖线
3. 新增 `QPropertyAnimation` 驱动 `m_indicatorY` 属性，从旧 Y 滑动到新 Y
4. 同时还需要记录 `m_indicatorHeight`（选中项高度），以便绘制正确长度的指示线
5. 重写 `currentChanged()` 虚函数，捕获选中项变化事件

#### 集成点

| 类/方法 | 修改内容 |
|---------|---------|
| 新增 `NavigationTreeView` | `core/NavigationTreeView.h/cpp`，QTreeView 子类 |
| `MainWindow::setupUI()` | 将 `m_navTree` 的类型从 `QTreeView*` 改为 `NavigationTreeView*` |
| `MainWindow` 类声明 | 将 `QTreeView* m_navTree` 改为 `NavigationTreeView* m_navTree` |

#### 伪代码

```cpp
// NavigationTreeView.h
class NavigationTreeView : public QTreeView {
    Q_OBJECT
    Q_PROPERTY(qreal indicatorY READ indicatorY WRITE setIndicatorY)
public:
    explicit NavigationTreeView(QWidget* parent = nullptr);

    qreal indicatorY() const;
    void setIndicatorY(qreal y);

protected:
    void paintEvent(QPaintEvent* event) override;
    void currentChanged(const QModelIndex& current,
                        const QModelIndex& previous) override;

private:
    qreal m_indicatorY = 0.0;
    int m_indicatorHeight = 0;
    QPropertyAnimation* m_slideAnim = nullptr;
};
```

```cpp
// NavigationTreeView.cpp
void NavigationTreeView::currentChanged(const QModelIndex& current,
                                         const QModelIndex& previous)
{
    QTreeView::currentChanged(current, previous);

    // 计算新旧选中项的 Y 坐标
    QRect oldRect = (previous.isValid()) ? visualRect(previous) : visualRect(current);
    QRect newRect = visualRect(current);

    qreal oldY = oldRect.top();
    qreal newY = newRect.top();
    m_indicatorHeight = newRect.height();

    // 创建滑动动画
    if (m_slideAnim && m_slideAnim->state() == QAbstractAnimation::Running) {
        m_slideAnim->stop();
    }

    m_slideAnim = new QPropertyAnimation(this, "indicatorY");
    m_slideAnim->setStartValue(oldY);
    m_slideAnim->setEndValue(newY);
    m_slideAnim->setDuration(250);
    m_slideAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_slideAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void NavigationTreeView::paintEvent(QPaintEvent* event)
{
    // 先让父类绘制完整树形控件
    QTreeView::paintEvent(event);

    // 然后在最上层绘制指示线
    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);

    // 获取 accent 色从调色板
    QColor accentColor = palette().color(QPalette::Highlight);

    // 绘制左侧 3px 指示线
    painter.setPen(Qt::NoPen);
    painter.setBrush(accentColor);
    QRectF indicatorRect(0, m_indicatorY, 3, m_indicatorHeight);
    painter.drawRoundedRect(indicatorRect, 1.5, 1.5);
}
```

#### 注意事项

- 指示线的 accent 色需要从 `ThemeManager` 或 `QPalette` 获取，不能硬编码
- 导航树展开/折叠时，各 item 的 Y 坐标会变化，此时需要同步更新 `m_indicatorY`。可通过监听 `expanded`/`collapsed` 信号来更新
- `visualRect()` 返回的坐标是视口坐标系，直接用于 QPainter 绘制
- QSS 中原有的 `::branch:selected` 和 `:selected` 背景样式应保留，指示线是额外叠加的视觉增强

---

## 动画实现铁律（源自 CLAUDE.md 6.5）

以下规则是所有动画实现的强制约束:

1. **使用 QPropertyAnimation** -- 所有动画必须用 Qt 的动画框架，禁止用 QTimer 手动插值
2. **缓动曲线统一** -- 展开/滑入用 `QEasingCurve::OutCubic`，收起/滑出用 `QEasingCurve::InCubic`
3. **时长限制** -- 最短 100ms（按钮按下），最长 400ms（进度完成），超出禁止
4. **禁止的动画类型**:
   - 禁止弹跳效果（Bounce/Back 除非用于通知弹出）
   - 禁止 3D 旋转/翻转
   - 禁止超过 500ms 的动画
   - 禁止彩虹色渐变或闪烁
5. **性能要求** -- 动画帧率不低于 30fps，不能卡顿掉帧

## 依赖的公共组件

| 组件 | 文件 | 复用方式 |
|------|------|---------|
| `ThemeManager` | `core/ThemeManager.h/cpp` | 主题切换过渡动画，获取语义色值 |
| `Constants` | `core/Constants.h` | `ConnectionState` 枚举用于状态动画触发判断 |
| `QPropertyAnimation` | Qt Animation Framework | 所有动画的底层实现 |
| `QGraphicsOpacityEffect` | Qt Widgets | 淡入淡出、呼吸脉冲动画的 opacity 驱动 |

## 设计模式

| 模式 | 应用场景 | 说明 |
|------|---------|------|
| **观察者模式** | 连接状态变化触发呼吸动画启停 | 通过 `IConnection::stateChanged` 信号驱动 `MainWindow::onConnectionStateChanged()` |
| **策略模式** | 动画启停策略 | 不同状态使用不同的动画策略（Connecting=呼吸，Connected=实色，Disconnected=灰色） |
| **模板方法** | 动画创建流程 | `QPropertyAnimation` 的创建-配置-启动流程统一封装在 `animatePanelSwitch()` 等方法中 |

## 新增文件清单

| 文件 | 层级 | 说明 |
|------|------|------|
| `src/ota/AnimatedProgressBar.h` | 表现层 | 动画进度条控件头文件 |
| `src/ota/AnimatedProgressBar.cpp` | 表现层 | 动画进度条控件实现 |
| `src/core/NavigationTreeView.h` | 表现层 | 带滑动指示线的导航树头文件 |
| `src/core/NavigationTreeView.cpp` | 表现层 | 带滑动指示线的导航树实现 |

## 修改文件清单

| 文件 | 修改内容 | 影响范围 |
|------|---------|---------|
| `src/core/MainWindow.h` | 新增动画相关成员变量和方法声明 | 新增成员: `m_connIndicator`, `m_breathAnim`, `m_themeTransitionEffect`; 新增方法: `animatePanelSwitch()`; 修改 `m_navTree` 类型为 `NavigationTreeView*` |
| `src/core/MainWindow.cpp` | 实现面板切换动画、呼吸脉冲动画、主题过渡动画 | `setupUI()`, `setupStatusBar()`, `onConnectionStateChanged()`, `onThemeChanged()`, `connectSignals()` |
| `src/ota/OtaWidget.h` | 将 `QProgressBar*` 改为 `AnimatedProgressBar*` | `m_progressBar` 类型变更 |
| `src/ota/OtaWidget.cpp` | 传输开始/完成/取消时启动/停止动画 | `setTransferring()`, `onTransferComplete()`, `onTransferError()`, `onCancelTransfer()` |
| `resources/themes/dark_terminal.qss` | 新增指示点、指示线、进度条的 QSS 选择器 | 新增: `QWidget#connIndicator`, `AnimatedProgressBar`, `NavigationTreeView` |
| `resources/themes/modern_dark.qss` | 同上 | 同上 |
| `resources/themes/light.qss` | 同上 | 同上 |
| `CMakeLists.txt` | 新增 `AnimatedProgressBar` 和 `NavigationTreeView` 源文件 | 构建系统 |

## 影响范围

| 区域 | 影响评估 |
|------|---------|
| 面板切换逻辑 | 当前 `setVisible()` 切换改为动画驱动，需要处理动画进行中用户快速连续点击导航项的边界情况 |
| 主题切换流程 | `onThemeChanged()` 从同步变为异步（淡出-换肤-淡入），需确保期间不会重复触发 |
| 连接状态指示 | 新增独立指示点控件，状态栏布局从纯 QLabel 变为 指示点 + QLabel 组合 |
| OTA 进度条 | 基础控件从 `QProgressBar` 替换为子类，QSS 选择器需要适配 |
| 导航树 | 基础控件从 `QTreeView` 替换为子类，MainWindow 中所有对 `m_navTree` 的使用不受影响（子类兼容） |

## 动画参数汇总表

| # | 动画名称 | 优先级 | 目标属性 | 起始值 | 结束值 | 时长 | 缓动曲线 | 循环 |
|---|---------|--------|---------|--------|--------|------|---------|------|
| R1a | 面板滑入 | P0 | pos + opacity | (40,0) + 0.0 | (0,0) + 1.0 | 250ms | OutCubic | 否 |
| R1b | 面板滑出 | P0 | pos + opacity | (0,0) + 1.0 | (-40,0) + 0.0 | 200ms | InCubic | 否 |
| R2a | 搜索栏展开 | P0 | maximumHeight | 0 | 36 | 200ms | OutCubic | 否 |
| R2b | 搜索栏收起 | P0 | maximumHeight | 36 | 0 | 150ms | InCubic | 否 |
| R3 | 状态呼吸脉冲 | P0 | QGraphicsOpacityEffect::opacity | 0.3 | 1.0 | 1500ms | InOutSine | 无限 |
| R4a | 主题淡出 | P0 | QGraphicsOpacityEffect::opacity | 1.0 | 0.0 | 150ms | InOutCubic | 否 |
| R4b | 主题淡入 | P0 | QGraphicsOpacityEffect::opacity | 0.0 | 1.0 | 150ms | InOutCubic | 否 |
| R5a | 进度条流动 | P1 | gradientOffset (自定义) | 0.0 | 1.0 | 2000ms | Linear | 无限 |
| R5b | 进度条完成 | P1 | progressColor (自定义) | accent | success | 400ms | OutCubic | 否 |
| R6 | 指示线滑动 | P1 | indicatorY (自定义) | 旧Y | 新Y | 250ms | OutCubic | 否 |

## 验收标准

### 功能验收

| # | 验收条件 | 验证方法 |
|---|---------|---------|
| AC1 | 点击导航树切换面板时，旧面板向左滑出+淡出（200ms），新面板从右侧滑入+淡入（250ms），过渡平滑无闪烁 | 手动测试: 依次点击"配置"、"终端"、"统计"等导航项，观察过渡效果 |
| AC2 | 快速连续点击不同导航项（间隔 < 200ms），面板切换不出现残影、重叠或布局错乱 | 手动测试: 快速连续点击3-5个不同导航项 |
| AC3 | Ctrl+F 打开搜索栏，搜索栏从顶部向下滑出展开（200ms），Esc 关闭时向上收回（150ms） | 手动测试: Ctrl+F 和 Esc 反复切换 |
| AC4 | 连接串口时，状态栏指示点在 Connecting 状态下显示呼吸脉冲效果（0.3-1.0 opacity，1500ms 周期） | 手动测试: 连接一个不存在的串口，观察 Connecting 状态下指示点呼吸效果 |
| AC5 | 连接成功后呼吸动画停止，指示点变为实色绿色；断开后变为灰色 | 手动测试: 连接真实串口后观察状态变化 |
| AC6 | 切换暗色到亮色主题时，窗口内容先淡出（150ms），切换后淡入（150ms），无瞬间闪烁 | 手动测试: 切换 dark_terminal -> light 主题 |
| AC7 | 主题切换动画期间，再次切换主题不会导致动画叠加或崩溃 | 手动测试: 快速反复切换不同主题 |
| AC8 | 应用首次启动加载保存的主题时，无淡入淡出动画（直接加载） | 启动验证: 设置非默认主题后重启应用 |
| AC9 | OTA传输进行中，进度条填充色有渐变高亮流动效果（2000ms 周期） | 手动测试: 启动OTA传输，观察进度条流动效果 |
| AC10 | OTA传输完成时，进度条从 accent 色平滑过渡到 success 色（400ms） | 手动测试: 完成一次完整OTA传输 |
| AC11 | 点击不同导航项时，左侧 3px accent 色指示线从旧位置平滑滑动到新位置（250ms） | 手动测试: 依次点击不同导航项，观察指示线滑动 |
| AC12 | 所有动画帧率不低于 30fps，无卡顿掉帧 | 使用 Qt Creator GammaRay 或手动目测 |

### 代码质量验收

| # | 验收条件 |
|---|---------|
| AC13 | 所有动画使用 `QPropertyAnimation` 实现，无 `QTimer` 手动插值 |
| AC14 | 缓动曲线符合规范: 展开/滑入 OutCubic，收起/滑出 InCubic，呼吸 InOutSine |
| AC15 | 所有动画时长在 100ms-400ms 范围内（呼吸动画 1500ms 除外，这是循环周期） |
| AC16 | 动画对象使用 `QAbstractAnimation::DeleteWhenStopped` 自动回收，无内存泄漏 |
| AC17 | 新增控件 `AnimatedProgressBar` 和 `NavigationTreeView` 在三个 QSS 主题文件中均有样式定义 |
| AC18 | 指示点颜色使用 QSS 动态属性选择器，无 C++ 硬编码色值 |
| AC19 | 进度条颜色从 ThemeManager/QPalette 获取，无硬编码 |
| AC20 | 零编译错误，零编译警告 |
| AC21 | EmbedDebug.bat 启动验证通过 |

### 性能验收

| # | 验收条件 |
|---|---------|
| AC22 | 6 项动画同时运行时（面板切换 + 呼吸 + 进度流动），CPU 占用不超过 10% |
| AC23 | 动画进行中终端数据接收不受影响（帧率、数据完整性） |
| AC24 | `AnimatedProgressBar::paintEvent()` 中无不必要的对象创建（QGradient 等复用） |
