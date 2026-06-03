/**
 * @file TerminalHighlighter.cpp
 * @brief 终端正则高亮器实现
 * @author Serial Tool Team
 * @date 2026-06-02
 */

#include "terminal/filter/TerminalHighlighter.h"
#include "core/theme/ThemeManager.h"

/**
 * @brief 构造函数，初始化默认高亮格式
 */
TerminalHighlighter::TerminalHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
    , m_color(ThemeManager::instance().color(ThemeManager::SemanticColor::TermSearchHighlight))
{
    m_format.setForeground(m_color);
    m_format.setFontWeight(QFont::Bold);
}

/**
 * @brief 设置高亮正则模式并重新高亮
 */
void TerminalHighlighter::setPattern(const QString &pattern)
{
    m_pattern = pattern;
    m_regex.setPattern(pattern);
    ++m_totalRuleChanges;
    rehighlight();
}

/**
 * @brief 设置高亮颜色
 */
void TerminalHighlighter::setHighlightColor(const QColor &color)
{
    m_color = color;
    m_format.setForeground(m_color);
    ++m_totalRuleChanges;
    rehighlight();
}

/**
 * @brief 高亮文本块中所有匹配内容
 */
void TerminalHighlighter::highlightBlock(const QString &text)
{
    if (m_pattern.isEmpty() || !m_regex.isValid()) {
        return;
    }

    QRegularExpressionMatchIterator it = m_regex.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        setFormat(match.capturedStart(), match.capturedLength(), m_format);
        ++m_totalHighlights;
    }
}

/**
 * @brief 重置所有统计计数器为零
 */
void TerminalHighlighter::resetStatistics()
{
    m_totalHighlights = 0;
    m_totalRuleChanges = 0;
}
