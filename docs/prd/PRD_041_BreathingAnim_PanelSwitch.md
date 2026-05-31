# PRD-041: Breathing Animation Refactor, Panel Slide Animation, PaintEvent Dedup, Toast Debounce

## 背景

代码审查(iteration #41)发现四处违反 CLAUDE.md 规范或存在代码质量问题:

1. **SerialConfigPanel 呼吸动画**使用 QTimer 手动插值(~20fps, 50ms间隔, +/-0.05步进),
   违反 section 6.5 铁律: 所有动画必须使用 QPropertyAnimation, 帧率不低于30fps.
   NavigationController 已有正确的 QPropertyAnimation 呼吸动画实现可复用.

2. **面板切换动画**当前为纯透明度淡入淡出(150ms cross-fade), 缺少 CLAUDE.md section 6.5
   规定的滑入/滑出效果(200ms InCubic 滑出 + 250ms OutCubic 滑入). 需增加 pos/geometry 动画.

3. **TerminalWidget::paintEvent** 的方向过滤模式和普通模式两个渲染路径存在大量重复代码:
   缓存更新、滚动偏移计算、可见行迭代绘制逻辑几乎相同, 仅 totalLines 来源和循环下标映射不同.

4. **连接吐司通知**在自动重连场景下可能快速重复弹出. ConnectionController::onAutoReconnect()
   每3秒触发一次 connectSerial(), 成功后发射 connectionSucceeded/connectionStateChanged,
   每次都会弹出 Success 吐司, 无防抖机制.

## 需求列表

| ID  | 需求描述 | 优先级 | 涉及模块 |
|-----|---------|--------|---------|
| R1  | SerialConfigPanel 呼吸动画: 删除 QTimer 手动插值, 改用 QPropertyAnimation + QGraphicsOpacityEffect, 1500ms循环 InOutSine, opacity 0.3-1.0-0.3 | P1 | serial/SerialConfigPanel |
| R2  | 面板切换动画: 旧面板向左滑出(200ms InCubic), 新面板从右侧滑入(250ms OutCubic), 使用 QPropertyAnimation on pos | P1 | core/NavigationController |
| R3  | TerminalWidget::paintEvent 渲染路径去重: 提取公共的缓存更新+滚动偏移+绘制循环到私有方法 | P2 | terminal/TerminalWidget |
| R4  | 连接吐司防抖: 同类型吐司 2 秒冷却期, 自动重连期间不重复弹出相同消息 | P2 | core/ToastWidget |

## 接口设计

### R1: SerialConfigPanel 呼吸动画重构

**删除成员**:
- `QTimer* m_breathTimer`
- `qreal m_breathOpacity`
- `bool m_breathIncreasing`

**新增成员**:
- `QPropertyAnimation* m_breathAnim = nullptr` -- 1500ms循环呼吸动画
- `QGraphicsOpacityEffect* m_statusEffect = nullptr` -- 状态指示器透明度效果

**修改方法**:

```cpp
// setConnecting(): 启动呼吸动画
void SerialConfigPanel::setConnecting()
{
    m_connecting = true;
    updateStatusIndicator("connecting");
    m_statusIndicator->setToolTip(tr("正在连接..."));

    // 创建或复用 opacity effect
    if (!m_statusEffect) {
        m_statusEffect = new QGraphicsOpacityEffect(m_statusIndicator);
        m_statusIndicator->setGraphicsEffect(m_statusEffect);
    }
    m_statusEffect->setOpacity(1.0);

    // 创建呼吸脉冲动画
    if (m_breathAnim) { m_breathAnim->stop(); delete m_breathAnim; }
    m_breathAnim = new QPropertyAnimation(m_statusEffect, "opacity");
    m_breathAnim->setStartValue(0.3);
    m_breathAnim->setEndValue(1.0);
    m_breathAnim->setDuration(1500);
    m_breathAnim->setEasingCurve(QEasingCurve::InOutSine);
    m_breathAnim->setLoopCount(-1);  // 无限循环
    m_breathAnim->start();
}

// setConnected()/setError() 中停止呼吸动画
if (m_breathAnim) {
    m_breathAnim->stop();
    delete m_breathAnim;
    m_breathAnim = nullptr;
}
if (m_statusEffect) {
    m_statusEffect->setOpacity(1.0);
    // effect 留在控件上, 下次 setConnecting 复用
}
```

### R2: 面板切换滑入/滑出动画

**修改 NavigationController::switchToPanel()**:

当前实现: 并行 cross-fade (150ms opacity-only).

改为: 旧面板向左滑出(200ms InCubic on pos + opacity) -> 新面板从右侧滑入(250ms OutCubic on pos + opacity).

```cpp
void NavigationController::switchToPanel(QWidget* newPanel)
{
    if (m_panelSwitching) return;
    if (m_currentPanel == newPanel) return;

    QWidget* oldPanel = m_currentPanel;
    m_currentPanel = newPanel;

    // 隐藏所有非当前、非旧面板
    for (auto* w : allSwitchablePanels()) {
        if (w && w != newPanel && w != oldPanel) {
            w->setGraphicsEffect(nullptr);
            w->setVisible(false);
        }
    }

    if (!newPanel) return;

    if (oldPanel && oldPanel->isVisible()) {
        m_panelSwitching = true;

        int panelWidth = oldPanel->width();

        // ---- 旧面板: 向左滑出 + 淡出 (200ms InCubic) ----
        QGraphicsOpacityEffect* fadeOutEffect = new QGraphicsOpacityEffect(oldPanel);
        oldPanel->setGraphicsEffect(fadeOutEffect);

        QPropertyAnimation* slideOut = new QPropertyAnimation(oldPanel, "pos");
        slideOut->setStartValue(oldPanel->pos());
        slideOut->setEndValue(oldPanel->pos() - QPoint(panelWidth / 3, 0));
        slideOut->setDuration(200);
        slideOut->setEasingCurve(QEasingCurve::InCubic);

        QPropertyAnimation* fadeOut = new QPropertyAnimation(fadeOutEffect, "opacity");
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.0);
        fadeOut->setDuration(200);
        fadeOut->setEasingCurve(QEasingCurve::InCubic);

        // 滑出完成: 清理旧面板
        connect(slideOut, &QPropertyAnimation::finished, this, [oldPanel]() {
            oldPanel->setGraphicsEffect(nullptr);
            oldPanel->setVisible(false);
        });

        // ---- 新面板: 从右侧滑入 + 淡入 (250ms OutCubic) ----
        newPanel->raise();
        newPanel->setVisible(true);
        QPoint finalPos = newPanel->pos();

        QGraphicsOpacityEffect* fadeInEffect = new QGraphicsOpacityEffect(newPanel);
        fadeInEffect->setOpacity(0.0);
        newPanel->setGraphicsEffect(fadeInEffect);
        newPanel->move(finalPos + QPoint(panelWidth / 3, 0));

        QPropertyAnimation* slideIn = new QPropertyAnimation(newPanel, "pos");
        slideIn->setStartValue(finalPos + QPoint(panelWidth / 3, 0));
        slideIn->setEndValue(finalPos);
        slideIn->setDuration(250);
        slideIn->setEasingCurve(QEasingCurve::OutCubic);

        QPropertyAnimation* fadeIn = new QPropertyAnimation(fadeInEffect, "opacity");
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->setDuration(250);
        fadeIn->setEasingCurve(QEasingCurve::OutCubic);

        // 滑入完成: 清理 effect 和标志
        connect(fadeIn, &QPropertyAnimation::finished, this, [this, newPanel, fadeInEffect]() {
            if (newPanel->graphicsEffect() == fadeInEffect)
                newPanel->setGraphicsEffect(nullptr);
            m_panelSwitching = false;
        });

        // 同时启动旧面板滑出和新面板滑入(并行)
        slideOut->start(QAbstractAnimation::DeleteWhenStopped);
        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
        slideIn->start(QAbstractAnimation::DeleteWhenStopped);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        // 无旧面板, 直接滑入
        newPanel->setVisible(true);
        slideInPanel(newPanel);  // 类似 fadeInPanel 但使用 pos 动画
    }
}
```

### R3: TerminalWidget paintEvent 渲染路径去重

**提取私有方法**:

```cpp
/**
 * @brief 更新缓存并执行可见行渲染
 *
 * paintEvent 的两个渲染路径(方向过滤模式 / 普通模式)共享:
 *   1. 缓存更新循环 (formatToCache)
 *   2. maxScrollOffset 计算
 *   3. autoScroll 同步
 *   4. 可见行迭代绘制循环
 *
 * @param painter QPainter 实例
 * @param totalLines 总行数(过滤后的行数或模型行数)
 * @param indexResolver 将显示行号映射到 m_cachedLines 索引的函数
 */
void renderVisibleLines(QPainter& painter, int totalLines,
                        std::function<int(int)> indexResolver);

/**
 * @brief 更新缓存行数据
 * 将模型中新增的行格式化到 m_cachedLines
 * @param startIdx 起始索引
 * @param count 新增行数
 */
void updateCacheRange(int startIdx, int count);
```

**paintEvent 简化为**:

```cpp
void TerminalWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.fillRect(rect(), m_bgColor);
    painter.setFont(m_font);

    if (!m_model) {
        painter.setPen(m_timestampColor);
        painter.drawText(rect(), Qt::AlignCenter, tr("未连接 - 等待数据..."));
        return;
    }

    int modelTotalLines = m_model->lineCount();

    if (m_directionFilter->isFiltered()) {
        // 方向过滤: 更新缓存 + 喂数据给 filter + 用 filter 的行号映射
        if (m_cachedLineCount > modelTotalLines) {
            m_cachedLineCount = 0; m_cachedLines.clear(); m_directionFilter->reset();
        }
        if (m_cachedLineCount != modelTotalLines) {
            updateCacheRange(m_cachedLineCount, modelTotalLines - m_cachedLineCount);
            m_directionFilter->onDataAppended(modelTotalLines,
                [this](int idx) { return m_model->lineAt(idx); });
            m_cachedLineCount = modelTotalLines;
        }
        int totalLines = m_directionFilter->filteredLineCount();
        if (!m_searchManager->searchPattern().isEmpty() && totalLines > 0)
            refreshSearchAfterCacheUpdate();
        renderVisibleLines(painter, totalLines,
            [this](int displayLine) { return m_directionFilter->modelIndex(displayLine); });
    } else {
        // 普通模式: 更新缓存 + 直接索引映射
        int totalLines = modelTotalLines;
        if (m_cachedLineCount != totalLines) {
            if (m_cachedLineCount > totalLines) {
                m_cachedLineCount = 0; m_cachedLines.clear();
            }
            updateCacheRange(m_cachedLineCount, totalLines - m_cachedLineCount);
            m_cachedLineCount = totalLines;
            if (!m_searchManager->searchPattern().isEmpty() && m_cachedLineCount > 0)
                refreshSearchAfterCacheUpdate();
        }
        renderVisibleLines(painter, totalLines,
            [](int displayLine) { return displayLine; });
    }
}
```

### R4: ToastWidget 防抖机制

**新增静态方法**:

```cpp
/**
 * @brief 显示吐司(带防抖)
 *
 * 同一 (parent, message, type) 组合在 kDebounceMs(2000ms) 内只显示一次.
 * 自动重连期间, "已连接: COM3" 吐司不会在 2 秒内重复弹出.
 *
 * @param parent 父窗口
 * @param message 消息文本
 * @param type 通知类型
 * @param durationMs 显示时长
 */
static void show(QWidget* parent, const QString& message,
                 ToastType type = ToastType::Info, int durationMs = 3000);
```

**实现要点**:

在 `show()` 方法开头增加防抖检查:

```cpp
// 防抖: 相同 parent+message+type 在 2 秒内不重复弹出
static QMap<QString, QElapsedTimer>& debounceMap() {
    static QMap<QString, QElapsedTimer> map;
    return map;
}

static constexpr int kDebounceMs = 2000;

// 在 show() 内部, 创建 toast 之前:
QString key = QString("%1|%2|%3")
    .arg(reinterpret_cast<quintptr>(parent), 0, 16)
    .arg(static_cast<int>(type))
    .arg(message);

if (debounceMap().contains(key) && debounceMap()[key].elapsed() < kDebounceMs) {
    return;  // 冷却期内, 忽略重复吐司
}
debounceMap()[key].start();

// ... 原有创建 toast 的逻辑 ...
```

## 依赖的公共组件

| 组件 | 文件 | 用途 |
|------|------|------|
| QPropertyAnimation | Qt框架 | 呼吸动画、面板滑动动画 |
| QGraphicsOpacityEffect | Qt框架 | 透明度动画载体 |
| QEasingCurve | Qt框架 | InOutSine/InCubic/OutCubic 缓动曲线 |
| ThemeManager | core/ThemeManager | 颜色获取(无变更) |
| ToastWidget | core/ToastWidget | 吐司通知(R4 扩展防抖) |

## 设计模式

| 模式 | 应用 | 说明 |
|------|------|------|
| 策略模式(函数式) | R3 renderVisibleLines 的 indexResolver 参数 | 通过 std::function 将过滤模式和普通模式的索引映射差异参数化, 消除分支重复 |
| 单例状态(静态map) | R4 防抖 debounceMap | ToastWidget 已使用 static map 管理活跃吐司列表, 防抖 map 遵循相同模式 |

## 影响范围

| 文件 | 变更类型 | 影响 |
|------|---------|------|
| src/serial/SerialConfigPanel.h | 修改 | 删除 QTimer/m_breathOpacity/m_breathIncreasing, 新增 m_breathAnim/m_statusEffect |
| src/serial/SerialConfigPanel.cpp | 修改 | 构造函数删除 timer 初始化, setConnecting/setConnected/setError 改用 QPropertyAnimation |
| src/core/NavigationController.h | 修改 | fadeInPanel 改为 slideInPanel, 新增 slideOutPanel |
| src/core/NavigationController.cpp | 修改 | switchToPanel 重写为 slide 动画, fadeInPanel 改为 slideInPanel |
| src/terminal/TerminalWidget.h | 修改 | 新增 renderVisibleLines/updateCacheRange 私有方法声明 |
| src/terminal/TerminalWidget.cpp | 修改 | paintEvent 拆分为两路径调用公共方法 |
| src/core/ToastWidget.h | 修改 | show() 方法内增加防抖逻辑, 新增 kDebounceMs 常量和 debounceMap |

**无新增文件**, 所有变更为现有文件修改.

**无新类**, 无架构影响.

**向后兼容**: 所有变更均为内部实现优化, 公开接口不变.

## 验收标准

### R1: SerialConfigPanel 呼吸动画
- [ ] SerialConfigPanel.h 中无 QTimer* m_breathTimer 成员
- [ ] SerialConfigPanel.cpp 中无 QTimer timeout 手动 opacity 步进代码
- [ ] setConnecting() 使用 QPropertyAnimation + InOutSine, 1500ms, 0.3-1.0
- [ ] setConnected()/setError() 正确停止动画并恢复 opacity 为 1.0
- [ ] 连接中状态指示器呼吸效果视觉流畅, 无卡顿(>30fps)
- [ ] 析构时无资源泄漏(m_breathAnim 正确清理)

### R2: 面板切换动画
- [ ] 点击导航树切换面板时, 旧面板向左滑出(200ms)
- [ ] 新面板从右侧滑入(250ms)
- [ ] 动画使用 QPropertyAnimation on pos + opacity, 非 QTimer
- [ ] 动画期间点击其他导航项不触发重复切换(m_panelSwitching 防重入)
- [ ] 首次切换(无旧面板)时直接滑入, 无闪烁
- [ ] restorePanelByIndex() 不触发动画(启动场景)

### R3: TerminalWidget 渲染去重
- [ ] paintEvent 中无两段几乎相同的缓存更新+绘制代码
- [ ] 公共逻辑提取到 renderVisibleLines() 和 updateCacheRange()
- [ ] 方向过滤模式和普通模式的渲染结果与重构前完全一致
- [ ] 搜索高亮在两种模式下均正常工作
- [ ] TerminalWidget.cpp 总行数减少

### R4: 吐司防抖
- [ ] 同一 (parent, message, type) 在 2 秒内只弹出一次
- [ ] 不同消息或不同类型不受防抖影响
- [ ] 自动重连场景下不出现重复的 "已连接" 吐司
- [ ] 防抖 map 不随时间无限增长(条目在冷却期后可清理)
- [ ] ToastWidget 公开接口 show() 签名不变, 调用方无需修改
