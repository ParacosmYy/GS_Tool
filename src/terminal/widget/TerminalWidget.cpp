/**
 * @file TerminalWidget.cpp
 * @brief 自绘制终端控件实现 — 构造/配置/搜索/缓存格式化
 *
 * 渲染逻辑(paintEvent, paintLine)已拆分至 TerminalWidgetPaint.cpp。
 * 输入事件处理(resizeEvent, wheelEvent, mousePress/Move/ReleaseEvent,
 * keyPressEvent)已拆分至 TerminalWidgetEvents.cpp。
 * 槽函数/右键菜单/统计计数已拆分至 TerminalWidgetSlots.cpp。
 * 本文件保留: 构造/析构、模型配置、方向过滤、显示模式、搜索委托、
 * 缓存格式化(formatToCache)。
 */

#include "terminal/widget/TerminalWidget.h"
#include "utils/crypto/HexConverter.h"
#include "core/theme/ThemeManager.h"
#include "shared/AppConstants.h"
#include "shared/TimerConstants.h"
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>

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
/** @brief 构造终端控件，初始化字体、主题颜色、选区/搜索/右键菜单管理器 @param parent 父Widget */
TerminalWidget::TerminalWidget(QWidget* parent)
    : QWidget(parent)
    , m_directionFilter(new DirectionFilter(this))
    , m_selectionManager(new TerminalSelectionManager(this))
    , m_searchManager(new TerminalSearchManager(this))
    , m_contextMenuManager(new TerminalContextMenuManager(this))
{
    setObjectName("terminalWidget");

    m_font = QFont(TerminalDefaults::kFontFamily, TerminalDefaults::kFontSize);
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

// ---- 搜索功能 → 见 TerminalWidgetSearch.cpp ----

// ---- 缓存格式化 ----
/** @brief 将原始TerminalLine格式化为缓存行(根据显示模式生成HEX/ASCII/混合文本) @param line 原始终端行数据 @return 格式化后的缓存行 */
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

