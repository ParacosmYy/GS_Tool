/**
 * @file TerminalFilter.cpp
 * @brief 终端正则过滤器实现
 * @author Serial Tool Team
 * @date 2026-06-02
 */

#include "terminal/filter/TerminalFilter.h"

/**
 * @brief 构造函数，初始化默认正则（大小写不敏感）
 */
TerminalFilter::TerminalFilter(QObject *parent)
    : QObject(parent)
    , m_caseSensitive(false)
{
}

/**
 * @brief 设置正则表达式模式
 * @param pattern 正则表达式字符串
 * @return 模式是否有效
 */
bool TerminalFilter::setPattern(const QString &pattern)
{
    m_pattern = pattern;

    QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
    if (!m_caseSensitive) {
        options |= QRegularExpression::CaseInsensitiveOption;
    }

    m_regex.setPattern(pattern);
    m_regex.setPatternOptions(options);

    if (!m_regex.isValid()) {
        emit patternError(m_regex.errorString());
        return false;
    }
    return true;
}

/**
 * @brief 对文本执行正则匹配
 */
bool TerminalFilter::match(const QString &text) const
{
    if (!m_regex.isValid()) {
        return false;
    }
    bool result = m_regex.match(text).hasMatch();
    if (result) {
        ++m_matchCount;
        m_lastMatch = text;
    }
    return result;
}

/**
 * @brief 提取捕获组内容
 */
QStringList TerminalFilter::captureGroups(const QString &text) const
{
    if (!m_regex.isValid()) {
        return {};
    }

    QStringList result;
    QRegularExpressionMatch matchObj = m_regex.match(text);
    if (matchObj.hasMatch()) {
        for (int i = 0; i <= matchObj.lastCapturedIndex(); ++i) {
            result.append(matchObj.captured(i));
        }
    }
    return result;
}

/**
 * @brief 设置大小写敏感并重新编译正则
 */
void TerminalFilter::setCaseSensitive(bool sensitive)
{
    m_caseSensitive = sensitive;
    // 重新编译正则以应用新选项
    if (!m_pattern.isEmpty()) {
        setPattern(m_pattern);
    }
}

/**
 * @brief 返回高亮颜色名称
 */
QString TerminalFilter::highlightColor() const
{
    return QStringLiteral("#FF6B35");
}

/**
 * @brief 获取匹配次数
 */
quint64 TerminalFilter::matchCount() const
{
    return m_matchCount;
}

/**
 * @brief 获取最近一次匹配的文本
 */
QString TerminalFilter::lastMatchText() const
{
    return m_lastMatch;
}

/**
 * @brief 重置匹配统计
 */
void TerminalFilter::resetStatistics()
{
    m_matchCount = 0;
    m_lastMatch.clear();
}
