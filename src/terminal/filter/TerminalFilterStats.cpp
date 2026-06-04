/**
 * @file TerminalFilterStats.cpp
 * @brief 终端多模式过滤器 — 统计接口与辅助方法实现
 *
 * 本文件从 TerminalFilterApply.cpp 拆分而来，包含:
 *   - 统计接口 (matchCount/filterPassCount/filterBlockCount/totalFilters/resetStatistics)
 *   - 高亮颜色接口 (highlightColor)
 *   - 私有方法 (compileRegex/checkTimestampRange)
 *
 * 过滤判断与时间戳过滤见 TerminalFilterApply.cpp。
 */

#include "terminal/filter/TerminalFilter.h"
#include "core/theme/ThemeManager.h"

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

/** @brief 获取累计添加过滤规则总数 @return 规则添加计数 */
quint64 TerminalFilter::totalFilters() const
{
    return m_totalFilters;
}

/** @brief 获取累计规则启用次数 @return 启用计数 */
quint64 TerminalFilter::totalFilterEnables() const
{
    return m_totalFilterEnables;
}

/** @brief 获取累计规则禁用次数 @return 禁用计数 */
quint64 TerminalFilter::totalFilterDisables() const
{
    return m_totalFilterDisables;
}

/** @brief 获取累计正则匹配命中次数 @return 匹配命中计数 */
quint64 TerminalFilter::totalFilterMatches() const
{
    return m_totalFilterMatches;
}

/** @brief 获取累计过滤模式切换次数 @return 模式切换计数 */
quint64 TerminalFilter::totalModeChanges() const
{
    return m_totalModeChanges;
}

/** @brief 获取累计时间戳过滤设置次数 @return 时间戳过滤设置计数 */
quint64 TerminalFilter::totalTimestampFiltersSet() const
{
    return m_totalTimestampFiltersSet;
}

/** @brief 重置所有统计计数器为零 */
void TerminalFilter::resetStatistics()
{
    m_matchCount = 0;
    m_lastMatch.clear();
    m_filterPassCount = 0;
    m_filterBlockCount = 0;
    m_totalFilters = 0;
    m_totalFilterEnables = 0;
    m_totalFilterDisables = 0;
    m_totalFilterMatches = 0;
    m_totalModeChanges = 0;
    m_totalTimestampFiltersSet = 0;
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
