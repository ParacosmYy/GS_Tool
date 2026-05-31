/**
 * @file TerminalWidget.cpp
 * @brief 自绘制终端控件实现 - QPainter高性能终端渲染
 *
 * 核心渲染逻辑保留在此文件中:
 *   - paintEvent: 主渲染循环，增量缓存更新，逐行绘制
 *   - paintLine: 单行绘制(时间戳+选区背景+搜索高亮+方向前缀+数据文本)
 *   - formatToCache: 数据行格式化为缓存结构
 *   - 滚动/缩放/键盘事件处理
 *
 * 选区管理和搜索功能分别委托给 TerminalSelectionManager 和 TerminalSearchManager。
 */

#include "terminal/TerminalWidget.h"
#include "utils/HexConverter.h"
#include "core/ThemeManager.h"
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QDebug>

// ---- 构造与基本配置 ----

TerminalWidget::TerminalWidget(QWidget* parent)
    : QWidget(parent)
    , m_directionFilter(new DirectionFilter(this))
    , m_selectionManager(new TerminalSelectionManager(this))
    , m_searchManager(new TerminalSearchManager(this))
{
    m_font = QFont("Consolas", 10);
    m_font.setStyleHint(QFont::Monospace);
    m_fontMetrics = QFontMetrics(m_font);

    // 从ThemeManager加载语义色板
    auto& theme = ThemeManager::instance();
    m_bgColor          = theme.color(ThemeManager::SemanticColor::TermBackground);
    m_rxColor          = theme.color(ThemeManager::SemanticColor::TermRxText);
    m_txColor          = theme.color(ThemeManager::SemanticColor::TermTxText);
    m_timestampColor   = theme.color(ThemeManager::SemanticColor::TermTimestamp);
    m_selectionManager->setSelectionBgColor(
        theme.color(ThemeManager::SemanticColor::TermSelection));
    m_searchManager->setSearchColors(
        theme.color(ThemeManager::SemanticColor::TermSearchHighlight),
        theme.color(ThemeManager::SemanticColor::TermCurrentMatch));

    // 监听主题切换，动态更新颜色
    connect(&theme, &ThemeManager::themeChanged, this, [this]() {
        auto& t = ThemeManager::instance();
        m_bgColor          = t.color(ThemeManager::SemanticColor::TermBackground);
        m_rxColor          = t.color(ThemeManager::SemanticColor::TermRxText);
        m_txColor          = t.color(ThemeManager::SemanticColor::TermTxText);
        m_timestampColor   = t.color(ThemeManager::SemanticColor::TermTimestamp);
        m_selectionManager->setSelectionBgColor(
            t.color(ThemeManager::SemanticColor::TermSelection));
        m_searchManager->setSearchColors(
            t.color(ThemeManager::SemanticColor::TermSearchHighlight),
            t.color(ThemeManager::SemanticColor::TermCurrentMatch));
        update();
    });

    setFont(m_font);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    m_lineHeight = m_fontMetrics.height() + 2;
    setMinimumSize(400, 200);

    // 转发搜索管理器的信号
    connect(m_searchManager, &TerminalSearchManager::searchMatchesChanged,
            this, &TerminalWidget::searchMatchesChanged);
}

void TerminalWidget::setModel(TerminalModel* model)
{
    if (m_model) {
        disconnect(m_model, nullptr, this, nullptr);
    }
    m_model = model;
    if (m_model) {
        connect(m_model, &TerminalModel::dataAppended,
                this, &TerminalWidget::onDataAppended);
        connect(m_model, &TerminalModel::dataCleared,
                this, &TerminalWidget::onDataCleared);
    }
    m_cachedLineCount = 0;
    m_cachedLines.clear();
    m_directionFilter->reset();
    update();
}

void TerminalWidget::setDirectionFilter(DataDirection direction)
{
    m_directionFilter->setDirection(direction);
    m_cachedLineCount = 0;
    m_cachedLines.clear();
    m_directionFilter->reset();
    update();
}

void TerminalWidget::clearDirectionFilter()
{
    m_directionFilter->clearFilter();
    m_cachedLineCount = 0;
    m_cachedLines.clear();
    m_directionFilter->reset();
    update();
}

void TerminalWidget::setDisplayMode(DisplayMode mode)
{
    m_displayMode = mode;
    m_cachedLineCount = 0;
    update();
}

DisplayMode TerminalWidget::displayMode() const { return m_displayMode; }

void TerminalWidget::setShowTimestamp(bool show)
{
    m_showTimestamp = show;
    m_cachedLineCount = 0;
    update();
}

bool TerminalWidget::showTimestamp() const { return m_showTimestamp; }

void TerminalWidget::setShowDirectionPrefix(bool show)
{
    m_showDirectionPrefix = show;
    m_cachedLineCount = 0;
    update();
}

bool TerminalWidget::showDirectionPrefix() const { return m_showDirectionPrefix; }

void TerminalWidget::setAutoScroll(bool autoScroll)
{
    m_autoScroll = autoScroll;
    if (m_autoScroll) {
        m_scrollOffset = m_maxScrollOffset;
        update();
    }
}

bool TerminalWidget::autoScroll() const { return m_autoScroll; }

void TerminalWidget::clear()
{
    m_cachedLines.clear();
    m_cachedLineCount = 0;
    m_directionFilter->reset();
    m_selectionManager->reset();
    update();
}

QString TerminalWidget::selectedText() const
{
    return m_selectionManager->selectedText(m_cachedLines, m_directionFilter);
}

QSize TerminalWidget::sizeHint() const { return QSize(800, 600); }

// ---- 搜索功能 - 委托给 TerminalSearchManager ----

void TerminalWidget::setSearchHighlight(const QString& pattern, bool regex, bool hex)
{
    auto lineAtFn = [this](int idx) -> QByteArray {
        return m_model ? m_model->lineAt(idx).data : QByteArray();
    };
    m_searchManager->setSearchHighlight(pattern, regex, hex,
                                         m_cachedLines, m_directionFilter,
                                         m_cachedLineCount, lineAtFn);
    update();
}

void TerminalWidget::clearSearchHighlight()
{
    m_searchManager->clearSearchHighlight();
    update();
}

int TerminalWidget::searchMatchCount() const { return m_searchManager->searchMatchCount(); }
int TerminalWidget::currentMatchIndex() const { return m_searchManager->currentMatchIndex(); }

void TerminalWidget::gotoNextMatch()
{
    int line = m_searchManager->gotoNextMatch();
    if (line >= 0) { scrollToMatch(line); update(); }
}

void TerminalWidget::gotoPrevMatch()
{
    int line = m_searchManager->gotoPrevMatch();
    if (line >= 0) { scrollToMatch(line); update(); }
}

void TerminalWidget::scrollToMatch(int line)
{
    if (line < m_scrollOffset || line >= m_scrollOffset + m_visibleLines) {
        m_scrollOffset = qMax(0, line - m_visibleLines / 3);
        m_autoScroll = false;
    }
}

void TerminalWidget::refreshSearch()
{
    if (m_searchManager->searchPattern().isEmpty()) return;
    QString pat = m_searchManager->searchPattern();
    bool rx = m_searchManager->searchRegex();
    bool hx = m_searchManager->searchHex();
    m_searchManager->clearSearchHighlight();
    setSearchHighlight(pat, rx, hx);
}

/** @brief 缓存更新后重新执行搜索(在paintEvent中调用，避免递归) */
void TerminalWidget::refreshSearchAfterCacheUpdate()
{
    if (m_searchManager->searchPattern().isEmpty()) return;
    QString pat = m_searchManager->searchPattern();
    bool rx = m_searchManager->searchRegex();
    bool hx = m_searchManager->searchHex();
    m_searchManager->clearSearchHighlight();
    setSearchHighlight(pat, rx, hx);
}

// ---- 单行绘制 ----

int TerminalWidget::paintLine(QPainter& painter, const CachedLine& cached, int y, int displayLine)
{
    // 计算时间戳偏移
    int xOffset = 0;
    if (m_showTimestamp) {
        painter.setPen(m_timestampColor);
        QString ts = QDateTime::fromMSecsSinceEpoch(cached.timestamp).toString("HH:mm:ss.zzz");
        painter.drawText(4, y + m_lineHeight - 4, ts);
        xOffset = m_fontMetrics.horizontalAdvance(ts) + 12;
    }

    // 选择背景色(正规化范围)
    if (m_selectionManager->hasSelection()) {
        int selStart = m_selectionManager->normalizedStartLine();
        int selEnd = m_selectionManager->normalizedEndLine();
        if (displayLine >= selStart && displayLine <= selEnd) {
            painter.fillRect(0, y, width(), m_lineHeight, m_selectionManager->selectionBgColor());
        }
    }

    // 搜索高亮
    const auto& matches = m_searchManager->searchMatches();
    if (!matches.isEmpty()) {
        int curIdx = m_searchManager->currentMatchIndex();
        for (int mi = 0; mi < matches.size(); ++mi) {
            const auto& match = matches[mi];
            if (match.line != displayLine) continue;
            int xStart = xOffset + 4 + m_fontMetrics.horizontalAdvance(cached.text.left(match.startCol));
            int matchWidth = m_fontMetrics.horizontalAdvance(cached.text.mid(match.startCol, match.length));
            painter.fillRect(xStart, y + 2, matchWidth, m_lineHeight - 4,
                             (mi == curIdx) ? m_searchManager->currentMatchColor()
                                            : m_searchManager->searchHighlightColor());
        }
    }

    // 方向前缀 "[TX:] " / "[RX:] " 分色渲染
    static const QString kTxPrefix = QStringLiteral("[TX:] ");
    static const QString kRxPrefix = QStringLiteral("[RX:] ");
    bool isTx = (cached.direction == DataDirection::Tx);
    QColor dataColor = isTx ? m_txColor : m_rxColor;

    if (m_showDirectionPrefix) {
        const QString& prefix = isTx ? kTxPrefix : kRxPrefix;
        if (cached.text.startsWith(prefix)) {
            painter.setPen(dataColor.darker(130));
            painter.drawText(xOffset + 4, y + m_lineHeight - 4, prefix);
            int prefixWidth = m_fontMetrics.horizontalAdvance(prefix);
            painter.setPen(dataColor);
            painter.drawText(xOffset + 4 + prefixWidth, y + m_lineHeight - 4,
                             cached.text.mid(prefix.length()));
        } else {
            painter.setPen(dataColor);
            painter.drawText(xOffset + 4, y + m_lineHeight - 4, cached.text);
        }
    } else {
        painter.setPen(dataColor);
        painter.drawText(xOffset + 4, y + m_lineHeight - 4, cached.text);
    }
    return y + m_lineHeight;
}

// ---- 核心渲染 ----

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

    // ---- 方向过滤模式 ----
    if (m_directionFilter->isFiltered()) {
        if (m_cachedLineCount > modelTotalLines) {
            m_cachedLineCount = 0;
            m_cachedLines.clear();
            m_directionFilter->reset();
        }
        if (m_cachedLineCount != modelTotalLines) {
            m_cachedLines.resize(modelTotalLines);
            for (int i = m_cachedLineCount; i < modelTotalLines; ++i)
                m_cachedLines[i] = formatToCache(m_model->lineAt(i));
            m_directionFilter->onDataAppended(modelTotalLines,
                [this](int idx) { return m_model->lineAt(idx); });
            m_cachedLineCount = modelTotalLines;
        }
        int totalLines = m_directionFilter->filteredLineCount();
        if (!m_searchManager->searchPattern().isEmpty() && totalLines > 0)
            refreshSearchAfterCacheUpdate();
        m_maxScrollOffset = qMax(0, totalLines - m_visibleLines);
        if (m_autoScroll) m_scrollOffset = m_maxScrollOffset;
        int endLine = qMin(m_scrollOffset + m_visibleLines + 1, totalLines);
        int y = 0;
        for (int i = m_scrollOffset; i < endLine; ++i) {
            int modelLine = m_directionFilter->modelIndex(i);
            y = paintLine(painter, m_cachedLines[modelLine], y, i);
        }
        return;
    }

    // ---- 普通模式(无方向过滤) ----
    int totalLines = modelTotalLines;
    if (m_cachedLineCount != totalLines) {
        if (m_cachedLineCount > totalLines) {
            m_cachedLineCount = 0;
            m_cachedLines.clear();
        }
        m_cachedLines.resize(totalLines);
        for (int i = m_cachedLineCount; i < totalLines; ++i)
            m_cachedLines[i] = formatToCache(m_model->lineAt(i));
        m_cachedLineCount = totalLines;
        if (!m_searchManager->searchPattern().isEmpty() && m_cachedLineCount > 0)
            refreshSearchAfterCacheUpdate();
        m_maxScrollOffset = qMax(0, totalLines - m_visibleLines);
        if (m_autoScroll) m_scrollOffset = m_maxScrollOffset;
    }
    int endLine = qMin(m_scrollOffset + m_visibleLines + 1, totalLines);
    int y = 0;
    for (int i = m_scrollOffset; i < endLine; ++i)
        y = paintLine(painter, m_cachedLines[i], y, i);
}

// ---- 事件处理 ----

void TerminalWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateVisibleRange();
}

void TerminalWidget::wheelEvent(QWheelEvent* event)
{
    int delta = event->angleDelta().y();
    m_scrollOffset -= delta / 120 * 3;
    m_scrollOffset = qMax(0, qMin(m_scrollOffset, m_maxScrollOffset));
    if (delta < 0 && m_scrollOffset < m_maxScrollOffset)
        m_autoScroll = false;
    update();
    event->accept();
}

void TerminalWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_selectionManager->onMousePress(event->position().y(), m_scrollOffset, m_lineHeight);
        update();
    }
    QWidget::mousePressEvent(event);
}

void TerminalWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton) {
        m_selectionManager->onMouseMove(event->position().y(), m_scrollOffset, m_lineHeight);
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void TerminalWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        m_selectionManager->onMouseRelease();
    QWidget::mouseReleaseEvent(event);
}

void TerminalWidget::keyPressEvent(QKeyEvent* event)
{
    // Ctrl+C 复制选中内容
    if (event->key() == Qt::Key_C && event->modifiers() & Qt::ControlModifier) {
        QString text = selectedText();
        if (!text.isEmpty()) QApplication::clipboard()->setText(text);
        return;
    }
    // F3 / Shift+F3 搜索导航
    if (event->key() == Qt::Key_F3) {
        (event->modifiers() & Qt::ShiftModifier) ? gotoPrevMatch() : gotoNextMatch();
        return;
    }
    QWidget::keyPressEvent(event);
}

// ---- 模型数据回调 ----

void TerminalWidget::onDataAppended(int firstNewLine, int count)
{
    Q_UNUSED(firstNewLine);
    Q_UNUSED(count);
    if (m_directionFilter->isFiltered()) { update(); return; }
    if (m_model) {
        m_maxScrollOffset = qMax(0, m_model->lineCount() - m_visibleLines);
        if (m_autoScroll) m_scrollOffset = m_maxScrollOffset;
    }
    update();
}

void TerminalWidget::onDataCleared()
{
    m_cachedLines.clear();
    m_cachedLineCount = 0;
    m_directionFilter->reset();
    m_selectionManager->reset();
    m_scrollOffset = 0;
    m_maxScrollOffset = 0;
    update();
}

void TerminalWidget::updateVisibleRange()
{
    m_visibleLines = height() / m_lineHeight;
    if (m_model) {
        m_maxScrollOffset = m_directionFilter->isFiltered()
            ? qMax(0, m_directionFilter->filteredLineCount() - m_visibleLines)
            : qMax(0, m_model->lineCount() - m_visibleLines);
        m_scrollOffset = qMin(m_scrollOffset, m_maxScrollOffset);
    }
}

// ---- 缓存格式化 ----

CachedLine TerminalWidget::formatToCache(const TerminalLine& line) const
{
    CachedLine cached;
    cached.direction = line.direction;
    cached.timestamp = line.timestamp.toMSecsSinceEpoch();
    QString prefix = m_showDirectionPrefix
        ? ((line.direction == DataDirection::Tx) ? "[TX:] " : "[RX:] ") : QString();

    switch (m_displayMode) {
    case DisplayMode::Hex:
        cached.text = prefix + HexConverter::toHexString(line.data);
        break;
    case DisplayMode::Mixed:
        cached.text = prefix + QString::fromUtf8(line.data) + "  |  " + HexConverter::toHexString(line.data);
        break;
    case DisplayMode::Decimal: {
        QStringList decBytes;
        for (unsigned char b : line.data) decBytes << QString::number(b);
        cached.text = prefix + decBytes.join(' ');
        break;
    }
    case DisplayMode::Text:
    default:
        cached.text = prefix + QString::fromUtf8(line.data);
        break;
    }
    return cached;
}
