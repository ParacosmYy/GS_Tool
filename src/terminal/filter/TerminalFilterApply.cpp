/**
 * @file TerminalFilterApply.cpp
 * @brief 终端多模式过滤器 — 过滤判断与时间戳范围过滤
 *
 * 本文件拆分自 TerminalFilter.cpp，包含:
 *   - 多模式过滤判断 (filter)
 *   - 时间戳范围过滤 (setTimestampRange/clearTimestampRange/checkTimestampRange)
 *
 * 统计接口与辅助方法见 TerminalFilterStats.cpp。
 */

#include "terminal/filter/TerminalFilter.h"

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

// 统计接口与辅助方法见 TerminalFilterStats.cpp
