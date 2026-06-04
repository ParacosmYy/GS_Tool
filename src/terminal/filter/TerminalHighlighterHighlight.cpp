/**
 * @file TerminalHighlighterHighlight.cpp
 * @brief 终端多模式高亮器 - 文本块高亮与统计接口实现
 *
 * 从 TerminalHighlighter.cpp 拆分而来，包含:
 *   - highlightBlock(): 多规则并行高亮文本块(QSyntaxHighlighter核心虚函数)
 *   - buildFormat():    构建QTextCharFormat辅助方法
 *   - totalRulesActive(): 统计活跃规则数
 *   - resetStatistics():  重置所有统计计数器
 *
 * 规则管理方法(addRule/removeRule/updateRule/setRuleColor/setRuleEnabled等)
 * 见 TerminalHighlighter.cpp。
 */

#include "terminal/filter/TerminalHighlighter.h"

/** @brief 高亮文本块中所有匹配内容(多规则并行应用) @param text 待处理文本 */
void TerminalHighlighter::highlightBlock(const QString &text)
{
    bool anyMatched = false;

    // 多规则模式: 应用所有启用的规则
    if (!m_rules.isEmpty()) {
        for (const auto& rule : m_rules) {
            if (!rule.enabled || !rule.regex.isValid()) continue;

            QRegularExpressionMatchIterator it = rule.regex.globalMatch(text);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
                ++m_totalHighlights;
                ++m_totalRegexMatches; ///< 统计: 正则匹配
                anyMatched = true;
            }
        }
    } else {
        // 单规则兼容模式: 使用默认正则
        if (!m_pattern.isEmpty() && m_regex.isValid()) {
            QRegularExpressionMatchIterator it = m_regex.globalMatch(text);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                setFormat(match.capturedStart(), match.capturedLength(), m_format);
                ++m_totalHighlights;
                ++m_totalRegexMatches; ///< 统计: 正则匹配
                anyMatched = true;
            }
        }
    }

    // 更新过滤统计: 有匹配的行算通过，无匹配的行算阻塞
    if (anyMatched) {
        ++m_filterPassCount;
    } else {
        ++m_filterBlockCount;
    }
}

/** @brief 构建高亮文本格式 @param color 高亮颜色 @return 配置好的QTextCharFormat */
QTextCharFormat TerminalHighlighter::buildFormat(const QColor& color) const
{
    QTextCharFormat fmt;
    fmt.setForeground(color);
    fmt.setFontWeight(QFont::Bold);
    return fmt;
}

/** @brief 获取当前活跃(启用)的规则数 @return 活跃规则数 */
quint64 TerminalHighlighter::totalRulesActive() const
{
    quint64 count = 0;
    for (const auto& rule : m_rules) {
        if (rule.enabled) ++count;
    }
    return count;
}

/** @brief 重置所有统计计数器为零 */
void TerminalHighlighter::resetStatistics()
{
    m_totalHighlights = 0;
    m_totalRuleChanges = 0;
    m_filterPassCount = 0;
    m_filterBlockCount = 0;
    m_totalRegexMatches = 0;
    m_totalColorChanges = 0;
    m_highlightErrors = 0;
}
