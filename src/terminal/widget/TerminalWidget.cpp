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

// ---- 搜索功能 → 见 TerminalWidgetSearch.cpp ----

// 配置/模型/方向过滤/显示模式/缓存格式化方法(setModel/setDirectionFilter/
// setDisplayMode/clear/selectedText/sizeHint/formatToCache等)
// 见 TerminalWidgetConfig.cpp

