/**
 * @file TerminalFilterApply.cpp
 * @brief 终端多模式过滤器 — 过滤判断、时间戳范围过滤、统计接口实现
 *
 * 本文件拆分自 TerminalFilter.cpp，包含:
 *   - 多模式过滤判断 (filter)
 *   - 时间戳范围过滤 (setTimestampRange/clearTimestampRange/checkTimestampRange)
 *   - 统计接口 (matchCount/filterPassCount/filterBlockCount/resetStatistics)
 *   - 高亮颜色接口 (highlightColor)
 *   - 私有方法 (compileRegex/checkTimestampRange)
 */

#include "terminal/filter/TerminalFilter.h"
#include "core/theme/ThemeManager.h"

// ---- 多模式过滤判断 ----

/**
 * @brief 多模式过滤判断: 文本是否通过当前规则集
 *
 * Include模式: 任一启用的规则匹配则通过，全部不匹配则阻塞
 * Exclude模式: 任一启用的规则匹配则阻塞，全部不匹配则通过
 * 无启用的规则时始终通过(不过滤)
 *
 * @param text 待检查文本
 * @return true 文本通过过滤应显示，false 文本被过滤应隐藏
 */
bool TerminalFilter::filter(const QString& text)
{
    // 收集所有启用的规则中匹配的结果
    bool hasEnabled = false;
    bool anyMatched = false;

    for (const auto& rule : m_rules) {
        if (!rule.enabled) continue;
        hasEnabled = true;
        if (rule.regex.isValid() && rule.regex.match(text).hasMatch()) {
            anyMatched = true;
            break;  // 短路: 已知有匹配，无需继续
        }
    }

    // 无启用的规则时不过滤，全部通过
    if (!hasEnabled) {
        ++m_filterPassCount;
        return true;
    }

    bool passed;
    if (m_filterMode == FilterMode::Include) {
        // 包含模式: 匹配则通过
        passed = anyMatched;
    } else {
        // 排除模式: 匹配则阻塞
        passed = !anyMatched;
    }

    if (passed) {
        ++m_filterPassCount;
    } else {
        ++m_filterBlockCount;
    }
    return passed;
}

/**
 * @brief 多模式过滤判断: 文本+时间戳是否通过当前规则集
 *
 * 同时检查正则规则和时间戳范围，两者都通过才返回true。
 *
 * @param text 待检查文本
 * @param timestamp 数据时间戳(epoch毫秒)
 * @return true 文本通过过滤应显示，false 文本被过滤应隐藏
 */
bool TerminalFilter::filter(const QString& text, qint64 timestamp)
{
    // 先检查时间戳范围
    if (!checkTimestampRange(timestamp)) {
        ++m_filterBlockCount;
        return false;
    }

    // 再检查正则规则
    return filter(text);
}

// ---- 时间戳范围过滤 ----

/**
 * @brief 设置时间戳过滤范围
 * @param from 起始时间(含)，无效QDateTime表示不限制
 * @param to 结束时间(含)，无效QDateTime表示不限制
 */
void TerminalFilter::setTimestampRange(const QDateTime& from, const QDateTime& to)
{
    if (from.isValid()) {
        m_timestampFrom = from;
        m_timestampFromMs = from.toMSecsSinceEpoch();
    } else {
        m_timestampFrom = QDateTime();
        m_timestampFromMs = 0;
    }

    if (to.isValid()) {
        m_timestampTo = to;
        m_timestampToMs = to.toMSecsSinceEpoch();
    } else {
        m_timestampTo = QDateTime();
        m_timestampToMs = 0;
    }

    emit filterRulesChanged();
}

/** @brief 清除时间戳过滤范围 */
void TerminalFilter::clearTimestampRange()
{
    m_timestampFrom = QDateTime();
    m_timestampTo = QDateTime();
    m_timestampFromMs = 0;
    m_timestampToMs = 0;
    emit filterRulesChanged();
}

/** @brief 是否启用了时间戳过滤 @return true 起始或结束时间已设置 */
bool TerminalFilter::hasTimestampFilter() const
{
    return m_timestampFrom.isValid() || m_timestampTo.isValid();
}

/** @brief 获取时间戳起始时间 @return 起始QDateTime(无效表示不限制) */
QDateTime TerminalFilter::timestampFrom() const
{
    return m_timestampFrom;
}

/** @brief 获取时间戳结束时间 @return 结束QDateTime(无效表示不限制) */
QDateTime TerminalFilter::timestampTo() const
{
    return m_timestampTo;
}

// ---- 高亮颜色(兼容接口) ----

/** @brief 返回高亮颜色名称 @return 颜色HEX字符串 */
QString TerminalFilter::highlightColor() const
{
    return ThemeManager::instance().color(ThemeManager::SemanticColor::TermSearchHighlight).name();
}

// ---- 统计接口 ----

/** @brief 获取累计匹配次数(单模式兼容) @return 匹配总次数 */
quint64 TerminalFilter::matchCount() const
{
    return m_matchCount;
}

/** @brief 获取最近一次匹配的文本 @return 最后匹配的文本 */
QString TerminalFilter::lastMatchText() const
{
    return m_lastMatch;
}

/** @brief 获取过滤通过的总行数 @return 通过计数 */
quint64 TerminalFilter::filterPassCount() const
{
    return m_filterPassCount;
}

/** @brief 获取过滤阻塞的总行数 @return 阻塞计数 */
quint64 TerminalFilter::filterBlockCount() const
{
    return m_filterBlockCount;
}

/** @brief 重置所有统计计数器为零 */
void TerminalFilter::resetStatistics()
{
    m_matchCount = 0;
    m_lastMatch.clear();
    m_filterPassCount = 0;
    m_filterBlockCount = 0;
}

// ---- 私有方法 ----

/**
 * @brief 编译正则表达式并设置选项
 * @param pattern 正则字符串
 * @param caseSensitive 是否区分大小写
 * @return 编译后的QRegularExpression对象(可能无效)
 */
QRegularExpression TerminalFilter::compileRegex(const QString& pattern, bool caseSensitive) const
{
    QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
    if (!caseSensitive) {
        options |= QRegularExpression::CaseInsensitiveOption;
    }
    QRegularExpression re(pattern, options);
    return re;
}

/**
 * @brief 检查时间戳是否在指定范围内
 * @param timestamp epoch毫秒时间戳
 * @return true 在范围内或未设置范围
 */
bool TerminalFilter::checkTimestampRange(qint64 timestamp) const
{
    if (m_timestampFromMs > 0 && timestamp < m_timestampFromMs) {
        return false;
    }
    if (m_timestampToMs > 0 && timestamp > m_timestampToMs) {
        return false;
    }
    return true;
}
