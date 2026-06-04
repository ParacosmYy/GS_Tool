/**
 * @file TerminalFilterCompat.cpp
 * @brief 终端过滤器 — 单模式兼容接口实现
 *
 * 从 TerminalFilter.cpp 拆分而来，包含:
 *   - setPattern(): 单条正则编译
 *   - match(): 单模式匹配
 *   - captureGroups(): 捕获组提取
 *   - setCaseSensitive(): 大小写敏感切换
 *
 * 多模式过滤接口见 TerminalFilter.cpp。
 * 过滤判断/时间戳/统计/私有方法见 TerminalFilterApply.cpp。
 */

#include "terminal/filter/TerminalFilter.h"

/** @brief 设置单条正则表达式模式并编译 @param pattern 正则表达式字符串 @return true=模式有效，false=编译失败 */
bool TerminalFilter::setPattern(const QString &pattern)
{
    m_pattern = pattern;
    m_regex = compileRegex(pattern, m_caseSensitive);

    if (!m_regex.isValid()) {
        emit patternError(m_regex.errorString());
        return false;
    }
    return true;
}

/** @brief 对文本执行正则匹配(单模式兼容) @param text 待匹配文本 @return true=匹配成功 */
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

/** @brief 提取捕获组内容(单模式兼容) @param text 待提取文本 @return 捕获组字符串列表 */
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

/** @brief 设置大小写敏感并重新编译单模式正则 @param sensitive true=区分大小写 */
void TerminalFilter::setCaseSensitive(bool sensitive)
{
    m_caseSensitive = sensitive;
    // 重新编译正则以应用新选项
    if (!m_pattern.isEmpty()) {
        setPattern(m_pattern);
    }
}
