/**
 * @file TerminalWidget.cpp
 * @brief 自绘制终端控件实现 - QPainter高性能终端渲染
 *
 * 核心渲染: paintEvent(主循环), paintLine(单行), formatToCache(缓存格式化)
 * 选区管理和搜索功能委托给 TerminalSelectionManager 和 TerminalSearchManager。
 */

#include "terminal/TerminalWidget.h"
#include "utils/HexConverter.h"
#include "core/ThemeManager.h"
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QKeySequence>
#include <QTimer>

namespace {
/**
 * @brief 安全UTF-8解码，将无效字节替换为\xHH而非Unicode替换字符
 *
 * 逐字节检查UTF-8序列有效性，无效字节输出\xHH可读转义，
 * 避免串口二进制数据中的非UTF-8字节被替换为'�'后丢失原始信息(P2-02)
 */
QString safeFromUtf8(const QByteArray& data) {
    QString result; result.reserve(data.size());
    int i = 0;
    // 尝试从data[i]开始解码len字节UTF-8序列，成功则追加并前移
    auto trySeq = [&](int len) -> bool {
        if (i + len > data.size()) return false;
        QString c = QString::fromUtf8(data.mid(i, len));
        if (!c.isEmpty() && c[0].unicode() != 0xFFFD) { result += c; i += len; return true; }
        return false;
    };
    auto esc = [](unsigned char ch) {
        return QString("\\x%1").arg(ch, 2, 16, QChar('0')).toUpper();
    };
    while (i < data.size()) {
        unsigned char ch = static_cast<unsigned char>(data[i]);
        if (ch < 0x80) { result += QLatin1Char(ch); ++i; }            // ASCII
        else if ((ch & 0xE0) == 0xC0 && trySeq(2)) {}                 // 2字节UTF-8
        else if ((ch & 0xF0) == 0xE0 && trySeq(3)) {}                 // 3字节UTF-8(中文)
        else if ((ch & 0xF8) == 0xF0 && trySeq(4)) {}                 // 4字节UTF-8(emoji)
        else { result += esc(ch); ++i; }                              // 无效字节→\xHH
    }
    return result;
}
} // anonymous namespace

// ---- 构造与基本配置 ----
TerminalWidget::TerminalWidget(QWidget* parent)
    : QWidget(parent)
    , m_directionFilter(new DirectionFilter(this))
    , m_selectionManager(new TerminalSelectionManager(this))
    , m_searchManager(new TerminalSearchManager(this))
    , m_contextMenuManager(new TerminalContextMenuManager(this))
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

    connect(m_contextMenuManager, &TerminalContextMenuManager::copyRequested,
            this, [this]() {
                QString text = selectedText();
                if (!text.isEmpty()) QApplication::clipboard()->setText(text);
            });
    connect(m_contextMenuManager, &TerminalContextMenuManager::pasteRequested,
            this, &TerminalWidget::pasteRequested);
    connect(m_contextMenuManager, &TerminalContextMenuManager::clearRequested,
            this, [this]() { clear(); emit clearRequested(); });
    connect(m_contextMenuManager, &TerminalContextMenuManager::selectAllRequested,
            this, &TerminalWidget::selectAll);
    connect(m_contextMenuManager, &TerminalContextMenuManager::searchRequested,
            this, &TerminalWidget::searchRequested);

    // 转发搜索管理器的信号
    connect(m_searchManager, &TerminalSearchManager::searchMatchesChanged,
            this, &TerminalWidget::searchMatchesChanged);
}

/** @brief 设置终端数据模型(断开旧模型信号，连接新模型) @param model TerminalModel指针 */
void TerminalWidget::setModel(TerminalModel* model)
{
    if (m_model) disconnect(m_model, nullptr, this, nullptr);
    m_model = model;
    if (m_model) {
        connect(m_model, &TerminalModel::dataAppended, this, &TerminalWidget::onDataAppended);
        connect(m_model, &TerminalModel::dataCleared, this, &TerminalWidget::onDataCleared);
    }
    m_cachedLineCount = 0;
    m_cachedLines.clear();
    m_directionFilter->reset();
    update();
}

/** @brief 设置数据方向过滤(仅显示指定方向的数据) @param direction 数据方向(TX/RX/Both) */
void TerminalWidget::setDirectionFilter(DataDirection direction)
{
    m_directionFilter->setDirection(direction);
    m_cachedLineCount = 0;
    m_cachedLines.clear();
    m_directionFilter->reset();
    m_selectionManager->reset();  // 方向过滤切换时重置选择，避免坐标空间不一致(BUG-02)
    update();
}

/** @brief 清除数据方向过滤(显示所有方向数据) */
void TerminalWidget::clearDirectionFilter()
{
    m_directionFilter->clearFilter();
    m_cachedLineCount = 0;
    m_cachedLines.clear();
    m_directionFilter->reset();
    m_selectionManager->reset();  // 清除方向过滤时同步重置选择
    update();
}

/** @brief 设置显示模式(HEX/ASCII/HEX-ASCII)并触发重绘 @param mode 显示模式 */
void TerminalWidget::setDisplayMode(DisplayMode mode) { m_displayMode = mode; m_cachedLineCount = 0; update(); }
/** @brief 返回当前显示模式 @return DisplayMode枚举 */
DisplayMode TerminalWidget::displayMode() const { return m_displayMode; }
/** @brief 设置是否显示时间戳并触发重绘 @param show true=显示 */
void TerminalWidget::setShowTimestamp(bool show) { m_showTimestamp = show; m_cachedLineCount = 0; update(); }
/** @brief 返回是否显示时间戳 @return true=显示 */
bool TerminalWidget::showTimestamp() const { return m_showTimestamp; }
/** @brief 设置是否显示方向前缀(TX↑/RX↓)并触发重绘 @param show true=显示 */
void TerminalWidget::setShowDirectionPrefix(bool show) { m_showDirectionPrefix = show; m_cachedLineCount = 0; update(); }
/** @brief 返回是否显示方向前缀 @return true=显示 */
bool TerminalWidget::showDirectionPrefix() const { return m_showDirectionPrefix; }

/** @brief 设置自动滚动到底部(新数据到来时自动滚动) @param autoScroll true=自动滚动 */
void TerminalWidget::setAutoScroll(bool autoScroll)
{
    m_autoScroll = autoScroll;
    if (m_autoScroll) { m_scrollOffset = m_maxScrollOffset; update(); }
}
/** @brief 返回自动滚动状态 @return true=自动滚动已开启 */
bool TerminalWidget::autoScroll() const { return m_autoScroll; }

/** @brief 清除终端内容和缓存，重置滚动位置 */
void TerminalWidget::clear()
{
    m_cachedLines.clear();
    m_cachedLineCount = 0;
    m_directionFilter->reset();
    m_selectionManager->reset();
    m_scrollOffset = 0;          // 防御性重置：防止未来调用模式变更导致滚动位置残留
    m_maxScrollOffset = 0;       // 同步重置最大滚动偏移
    // 同步清空底层数据模型，防止缓存清空后下次paintEvent从模型重建导致旧数据闪现(P2-01)
    if (m_model) m_model->clear();
    update();
}

/** @brief 返回当前选中的文本内容 @return 选中文本字符串 */
QString TerminalWidget::selectedText() const
{
    return m_selectionManager->selectedText(m_cachedLines, m_directionFilter);
}

/** @brief 返回推荐控件大小(800x600) @return 推荐尺寸 */
QSize TerminalWidget::sizeHint() const { return QSize(800, 600); }

// ---- 搜索功能 - 委托给 TerminalSearchManager ----
/** @brief 设置搜索高亮(支持正则/HEX/普通文本) @param pattern 搜索模式 @param regex 是否正则 @param hex 是否HEX模式 */
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

/** @brief 清除搜索高亮并重绘 */
void TerminalWidget::clearSearchHighlight() { m_searchManager->clearSearchHighlight(); update(); }
/** @brief 返回搜索匹配总数 @return 匹配数量 */
int TerminalWidget::searchMatchCount() const { return m_searchManager->searchMatchCount(); }
/** @brief 返回当前高亮的匹配索引 @return 当前索引 */
int TerminalWidget::currentMatchIndex() const { return m_searchManager->currentMatchIndex(); }

/** @brief 跳转到下一个搜索匹配项 */
void TerminalWidget::gotoNextMatch()
{
    int line = m_searchManager->gotoNextMatch();
    if (line >= 0) scrollToMatch(line); update();
}

/** @brief 跳转到上一个搜索匹配项 */
void TerminalWidget::gotoPrevMatch()
{
    int line = m_searchManager->gotoPrevMatch();
    if (line >= 0) scrollToMatch(line); update();
}

/** @brief 滚动到指定行并确保可见 @param line 目标行号 */
void TerminalWidget::scrollToMatch(int line)
{
    if (line < m_scrollOffset || line >= m_scrollOffset + m_visibleLines) {
        m_scrollOffset = qMax(0, line - m_visibleLines / 3);
        m_autoScroll = false;
    }
}

/** @brief 缓存更新后重新搜索(paintEvent中调用) */
/** @brief 缓存更新后刷新搜索匹配(重新计算所有匹配位置) */
void TerminalWidget::refreshSearchAfterCacheUpdate()
{
    if (m_searchManager->searchPattern().isEmpty()) return;
    QString pat = m_searchManager->searchPattern();
    bool rx = m_searchManager->searchRegex(), hx = m_searchManager->searchHex();
    // setSearchHighlight内部会先清除旧匹配(line 37)，无需额外调用clearSearchHighlight
    // 移除clearSearchHighlight()避免中间态信号导致搜索计数闪烁
    setSearchHighlight(pat, rx, hx);
}

// ---- 单行绘制 ----
/** @brief 绘制单行终端内容(时间戳+方向前缀+HEX/ASCII数据+搜索高亮) @param painter 画笔 @param cached 缓存行数据 @param y 起始Y坐标 @param displayLine 显示行号 @return 绘制消耗的像素高度 */
int TerminalWidget::paintLine(QPainter& painter, const CachedLine& cached, int y, int displayLine)
{
    int xOffset = 0;
    if (m_showTimestamp) {
        painter.setPen(m_timestampColor);
        QString ts = QDateTime::fromMSecsSinceEpoch(cached.timestamp).toString("HH:mm:ss.zzz");
        painter.drawText(4, y + m_lineHeight - 4, ts);
        xOffset = m_fontMetrics.horizontalAdvance(ts) + 12;
    }

    // 选择背景色
    if (m_selectionManager->hasSelection()) {
        int selStart = m_selectionManager->normalizedStartLine();
        int selEnd = m_selectionManager->normalizedEndLine();
        if (displayLine >= selStart && displayLine <= selEnd)
            painter.fillRect(0, y, width(), m_lineHeight, m_selectionManager->selectionBgColor());
    }

    // 方向前缀 "[TX:] " / "[RX:] " 分色渲染
    // 计算文本绘制基线: 先算出方向前缀占用的像素宽度，
    // 搜索高亮和实际文本内容都基于这个基线对齐，避免双倍偏移
    static const QString kTxPrefix = QStringLiteral("[TX:] ");
    static const QString kRxPrefix = QStringLiteral("[RX:] ");
    bool isTx = (cached.direction == DataDirection::Tx);
    QColor dataColor = isTx ? m_txColor : m_rxColor;

    // textXOffset = 文本内容的起始像素位置（时间戳宽度 + 边距 + 方向前缀宽度）
    int textXOffset = xOffset + 4;
    if (m_showDirectionPrefix) {
        const QString& prefix = isTx ? kTxPrefix : kRxPrefix;
        if (cached.text.startsWith(prefix)) {
            textXOffset += m_fontMetrics.horizontalAdvance(prefix);
        }
    }

    // 搜索高亮 — 委托给 TerminalSearchRenderer 绘制
    TerminalSearchRenderer::drawHighlights(painter, m_fontMetrics, m_searchManager,
                                           cached, displayLine, textXOffset,
                                           y, m_lineHeight, m_showDirectionPrefix);

    // 绘制文本内容（方向前缀 + 实际数据）
    if (m_showDirectionPrefix) {
        const QString& prefix = isTx ? kTxPrefix : kRxPrefix;
        if (cached.text.startsWith(prefix)) {
            painter.setPen(dataColor.darker(130));
            painter.drawText(xOffset + 4, y + m_lineHeight - 4, prefix);
            painter.setPen(dataColor);
            painter.drawText(textXOffset, y + m_lineHeight - 4,
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
/** @brief 自绘事件：绘制可见区域的终端行(时间戳+方向+数据+搜索高亮+选中) @param event 绘制事件 */
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
            m_cachedLineCount = 0; m_cachedLines.clear(); m_directionFilter->reset();
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
            QTimer::singleShot(0, this, [this]() { refreshSearchAfterCacheUpdate(); });
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
            m_cachedLineCount = 0; m_cachedLines.clear();
        }
        m_cachedLines.resize(totalLines);
        for (int i = m_cachedLineCount; i < totalLines; ++i)
            m_cachedLines[i] = formatToCache(m_model->lineAt(i));
        m_cachedLineCount = totalLines;
        if (!m_searchManager->searchPattern().isEmpty() && m_cachedLineCount > 0)
            QTimer::singleShot(0, this, [this]() { refreshSearchAfterCacheUpdate(); });
        m_maxScrollOffset = qMax(0, totalLines - m_visibleLines);
        if (m_autoScroll) m_scrollOffset = m_maxScrollOffset;
    }
    int endLine = qMin(m_scrollOffset + m_visibleLines + 1, totalLines);
    int y = 0;
    for (int i = m_scrollOffset; i < endLine; ++i)
        y = paintLine(painter, m_cachedLines[i], y, i);
}

// ---- 事件处理 ----
/** @brief 窗口大小变化事件：更新可见行范围 @param event 大小变化事件 */
void TerminalWidget::resizeEvent(QResizeEvent* event) { QWidget::resizeEvent(event); updateVisibleRange(); }

/** @brief 鼠标滚轮事件：Ctrl+滚轮缩放字号，普通滚轮上下滚动 @param event 滚轮事件 */
void TerminalWidget::wheelEvent(QWheelEvent* event)
{
    int delta = event->angleDelta().y();
    m_scrollOffset -= delta / 120 * 3;
    m_scrollOffset = qMax(0, qMin(m_scrollOffset, m_maxScrollOffset));
    // 任何手动滚动只要不在底部就禁用自动滚动（修复向上滚动立即回弹问题）
    if (m_scrollOffset < m_maxScrollOffset) m_autoScroll = false;
    update();
    event->accept();
}

/** @brief 鼠标按下事件：记录选区起点 @param event 鼠标事件 */
void TerminalWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_selectionManager->onMousePress(event->position().y(), m_scrollOffset, m_lineHeight);
        update();
    }
    QWidget::mousePressEvent(event);
}

/** @brief 鼠标移动事件：更新文本选区并重绘 @param event 鼠标事件 */
void TerminalWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton) {
        m_selectionManager->onMouseMove(event->position().y(), m_scrollOffset, m_lineHeight);
        update();
    }
    QWidget::mouseMoveEvent(event);
}

/** @brief 鼠标释放事件：完成文本选区，自动复制选中内容到剪贴板 @param event 鼠标事件 */
void TerminalWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) m_selectionManager->onMouseRelease();
    QWidget::mouseReleaseEvent(event);
}

/** @brief 键盘事件：Ctrl+C复制、Ctrl+F搜索、F3/Shift+F3导航匹配、Ctrl+A全选 @param event 键盘事件 */
void TerminalWidget::keyPressEvent(QKeyEvent* event)
{
    // Ctrl+C 复制选中内容 — 精确匹配 Ctrl 修饰键，避免 Ctrl+Shift+C 被误拦截
    if (event->key() == Qt::Key_C && event->modifiers() == Qt::ControlModifier) {
        QString text = selectedText();
        if (!text.isEmpty()) QApplication::clipboard()->setText(text);
        return;
    }
    // Ctrl+A 全选 — 精确匹配，避免 Ctrl+Shift+A 误触发
    if (event->key() == Qt::Key_A && event->modifiers() == Qt::ControlModifier) {
        selectAll();
        return;
    }
    // Ctrl+V 粘贴 — 精确匹配，转发粘贴请求
    if (event->key() == Qt::Key_V && event->modifiers() == Qt::ControlModifier) {
        QString text = QApplication::clipboard()->text();
        if (!text.isEmpty()) emit pasteRequested(text);
        return;
    }
    // Ctrl+F 搜索 — 精确匹配，避免 Ctrl+Shift+F 误触发
    if (event->key() == Qt::Key_F && event->modifiers() == Qt::ControlModifier) {
        emit searchRequested();
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
/** @brief 数据追加回调：更新缓存行计数，自动滚动到底部 @param firstNewLine 首行索引 @param count 新增行数 */
void TerminalWidget::onDataAppended(int firstNewLine, int count)
{
    Q_UNUSED(firstNewLine); Q_UNUSED(count);
    if (m_directionFilter->isFiltered()) { update(); return; }
    if (m_model) {
        m_maxScrollOffset = qMax(0, m_model->lineCount() - m_visibleLines);
        if (m_autoScroll) m_scrollOffset = m_maxScrollOffset;
    }
    update();
}

/** @brief 数据清空回调：重置缓存和滚动位置 */
void TerminalWidget::onDataCleared()
{
    m_cachedLines.clear(); m_cachedLineCount = 0;
    m_directionFilter->reset(); m_selectionManager->reset();
    m_scrollOffset = 0; m_maxScrollOffset = 0;
    update();
}

/** @brief 计算并更新可见行范围(首行索引+可见行数+缓存构建) */
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
        cached.text = prefix + HexConverter::toHexString(line.data); break;
    case DisplayMode::Mixed:
        cached.text = prefix + safeFromUtf8(line.data) + "  |  " + HexConverter::toHexString(line.data); break;
    case DisplayMode::Decimal: {
        QStringList decBytes;
        for (unsigned char b : line.data) decBytes << QString::number(b);
        cached.text = prefix + decBytes.join(' '); break;
    }
    case DisplayMode::Text:
    default:
        cached.text = prefix + safeFromUtf8(line.data); break;
    }
    return cached;
}

// ---- 右键菜单 / 全选 ----
/** @brief 右键菜单事件：弹出复制/全选/清屏/搜索菜单 @param event 右键菜单事件 */
void TerminalWidget::contextMenuEvent(QContextMenuEvent* event)
{
    m_contextMenuManager->showContextMenu(event, !selectedText().isEmpty());
}

/** @brief 全选所有终端行文本 */
void TerminalWidget::selectAll()
{
    int totalLines = m_directionFilter->isFiltered()
        ? m_directionFilter->filteredLineCount()
        : (m_model ? m_model->lineCount() : 0);
    if (totalLines > 0 && m_selectionManager) {
        m_selectionManager->setSelection(0, totalLines - 1);
        update();
    }
}
