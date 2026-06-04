/**
 * @file ThemeManagerColor.cpp
 * @brief 主题管理器 - 语义色板相关方法实现
 *
 * 从 ThemeManager.cpp 拆分而来，包含:
 *   1. 默认色板加载 (Catppuccin Mocha)
 *   2. 语义色查询接口 (带缓存统计)
 *   3. 从QSS中解析 --semantic-XXX 自定义属性覆盖色板
 */

#include "core/theme/ThemeManager.h"

#include <QRegularExpression>

/** @brief 加载默认色板(Catppuccin Mocha)，QSS解析失败时作为兜底颜色 */
void ThemeManager::loadDefaultColors()
{
    // ---- Catppuccin Mocha 色板（dark_terminal 默认值）----
    m_colorMap[SemanticColor::BgPrimary]       = QColor(30, 30, 46);    // #1e1e2e
    m_colorMap[SemanticColor::BgSecondary]     = QColor(49, 50, 68);    // #313244
    m_colorMap[SemanticColor::BgTertiary]      = QColor(69, 71, 90);    // #45475a
    m_colorMap[SemanticColor::BgHover]         = QColor(69, 71, 90);    // #45475a
    m_colorMap[SemanticColor::TextPrimary]     = QColor(205, 214, 244); // #cdd6f4
    m_colorMap[SemanticColor::TextSecondary]   = QColor(166, 173, 200); // #a6adc8
    m_colorMap[SemanticColor::TextMuted]       = QColor(108, 112, 134); // #6c7086
    m_colorMap[SemanticColor::Accent]          = QColor(137, 180, 250); // #89b4fa
    m_colorMap[SemanticColor::AccentHover]     = QColor(180, 208, 251); // #b4d0fb
    m_colorMap[SemanticColor::AccentPressed]   = QColor(116, 168, 247); // #74a8f7
    m_colorMap[SemanticColor::Border]          = QColor(49, 50, 68);    // #313244
    m_colorMap[SemanticColor::BorderFocus]     = QColor(137, 180, 250); // #89b4fa
    m_colorMap[SemanticColor::Success]         = QColor(166, 227, 161); // #a6e3a1
    m_colorMap[SemanticColor::Warning]         = QColor(249, 226, 175); // #f9e2af
    m_colorMap[SemanticColor::Error]           = QColor(243, 139, 168); // #f38ba8
    m_colorMap[SemanticColor::Scrollbar]       = QColor(69, 71, 90);    // #45475a
    m_colorMap[SemanticColor::ScrollbarHover]  = QColor(88, 91, 112);   // #585b70
    m_colorMap[SemanticColor::Shadow]          = QColor(0, 0, 0, 77);   // rgba(0,0,0,0.3)

    // ---- 终端专用颜色（Catppuccin Mocha）----
    m_colorMap[SemanticColor::TermBackground]      = QColor(30, 30, 46);    // #1e1e2e
    m_colorMap[SemanticColor::TermRxText]          = QColor(205, 214, 244); // #cdd6f4 接收文本
    m_colorMap[SemanticColor::TermTxText]          = QColor(166, 227, 161); // #a6e3a1 发送文本
    m_colorMap[SemanticColor::TermTimestamp]       = QColor(147, 153, 178); // #9399b2 时间戳
    m_colorMap[SemanticColor::TermSelection]       = QColor(69, 71, 90);    // #45475a 选中背景
    m_colorMap[SemanticColor::TermSearchHighlight] = QColor(249, 226, 175, 80);  // #f9e2af 半透明
    m_colorMap[SemanticColor::TermCurrentMatch]    = QColor(249, 226, 175, 180); // #f9e2af 高不透明度
}

/** @brief 按语义色枚举获取对应QColor(未映射时返回灰色并警告) @param color 语义色枚举 @return QColor */
QColor ThemeManager::color(SemanticColor color) const
{
    // 统计：累计语义色查询计数
    ++m_totalColorQueries;

    auto it = m_colorMap.constFind(color);
    if (it != m_colorMap.constEnd()) {
        ++m_totalCacheHits;  // 统计: 语义色缓存命中
        return it.value();
    }
    // 兜底: 返回深灰色，避免程序崩溃
    ++m_totalCacheMisses;  // 统计: 语义色缓存未命中
    qWarning() << "ThemeManager: unmapped SemanticColor" << static_cast<int>(color);
    return QColor(128, 128, 128);
}

/** @brief 从QSS内容中解析语义色板(--semantic-XXX格式覆盖默认值) @param qssContent QSS文件内容 */
void ThemeManager::parseColorsFromQss(const QString& qssContent)
{
    static const QRegularExpression regex(
        QLatin1String(R"(--semantic-(\w+)\s*:\s*([^;]+);)")
    );

    // 枚举名 -> 枚举值映射表
    static const QMap<QString, SemanticColor> nameMap = {
        {"BgPrimary",       SemanticColor::BgPrimary},
        {"BgSecondary",     SemanticColor::BgSecondary},
        {"BgTertiary",      SemanticColor::BgTertiary},
        {"BgHover",         SemanticColor::BgHover},
        {"TextPrimary",     SemanticColor::TextPrimary},
        {"TextSecondary",   SemanticColor::TextSecondary},
        {"TextMuted",       SemanticColor::TextMuted},
        {"Accent",          SemanticColor::Accent},
        {"AccentHover",     SemanticColor::AccentHover},
        {"AccentPressed",   SemanticColor::AccentPressed},
        {"Border",          SemanticColor::Border},
        {"BorderFocus",     SemanticColor::BorderFocus},
        {"Success",         SemanticColor::Success},
        {"Warning",         SemanticColor::Warning},
        {"Error",           SemanticColor::Error},
        {"Scrollbar",       SemanticColor::Scrollbar},
        {"ScrollbarHover",  SemanticColor::ScrollbarHover},
        {"Shadow",          SemanticColor::Shadow},
        {"TermBackground",      SemanticColor::TermBackground},
        {"TermRxText",          SemanticColor::TermRxText},
        {"TermTxText",          SemanticColor::TermTxText},
        {"TermTimestamp",       SemanticColor::TermTimestamp},
        {"TermSelection",       SemanticColor::TermSelection},
        {"TermSearchHighlight", SemanticColor::TermSearchHighlight},
        {"TermCurrentMatch",    SemanticColor::TermCurrentMatch},
    };

    QRegularExpressionMatchIterator it = regex.globalMatch(qssContent);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString name = match.captured(1);
        QString value = match.captured(2).trimmed();

        auto enumIt = nameMap.constFind(name);
        if (enumIt != nameMap.constEnd()) {
            QColor color(value);
            if (color.isValid()) {
                m_colorMap[enumIt.value()] = color;
            } else {
                qWarning() << "ThemeManager: invalid color for" << name << ":" << value;
            }
        }
    }
}
