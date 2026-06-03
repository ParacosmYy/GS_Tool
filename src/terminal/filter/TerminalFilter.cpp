/**
 * @file TerminalFilter.cpp
 * @brief 终端正则过滤器实现
 * @author Serial Tool Team
 * @date 2026-06-02
 */

#include "terminal/filter/TerminalFilter.h"
#include "core/theme/ThemeManager.h"

/** @brief 构造函数，初始化默认正则(大小写不敏感) @param parent 父对象 */
TerminalFilter::TerminalFilter(QObject *parent)
    : QObject(parent)
    , m_caseSensitive(false)
{
}

/** @brief 设置正则表达式模式并编译 @param pattern 正则表达式字符串 @return true=模式有效，false=编译失败 */
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

/** @brief 对文本执行正则匹配 @param text 待匹配文本 @return true=匹配成功 */
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

/** @brief 提取捕获组内容 @param text 待提取文本 @return 捕获组字符串列表 */
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

/** @brief 设置大小写敏感并重新编译正则 @param sensitive true=区分大小写 */
void TerminalFilter::setCaseSensitive(bool sensitive)
{
    m_caseSensitive = sensitive;
    // 重新编译正则以应用新选项
    if (!m_pattern.isEmpty()) {
        setPattern(m_pattern);
    }
}

/** @brief 返回高亮颜色名称 @return 颜色HEX字符串 */
QString TerminalFilter::highlightColor() const
{
    return ThemeManager::instance().color(ThemeManager::SemanticColor::TermSearchHighlight).name();
}

/** @brief 获取累计匹配次数 @return 匹配总次数 */
quint64 TerminalFilter::matchCount() const
{
    return m_matchCount;
}

/** @brief 获取最近一次匹配的文本 @return 最后匹配的文本 */
QString TerminalFilter::lastMatchText() const
{
    return m_lastMatch;
}

/** @brief 重置匹配统计计数器为零 */
void TerminalFilter::resetStatistics()
{
    m_matchCount = 0;
    m_lastMatch.clear();
}
