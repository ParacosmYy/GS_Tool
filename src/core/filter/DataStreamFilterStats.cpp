/**
 * @file DataStreamFilterStats.cpp
 * @brief 数据流过滤器统计查询和重置方法实现
 *
 * 从 DataStreamFilter.cpp 拆分而来，包含统计信息查询和重置方法。
 */

#include "core/filter/DataStreamFilter.h"

/** @brief 获取运行时统计信息 @return 统计数据的const引用 */
const DataStreamFilter::Stats& DataStreamFilter::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计计数器为初始值
 *
 * 不影响规则列表、规则启用状态和正则缓存。
 * 重置后activeRules会立即重新计算。
 */
void DataStreamFilter::resetStatistics()
{
    m_stats.totalInputPackets = 0;
    m_stats.totalPassed = 0;
    m_stats.totalDropped = 0;
    m_stats.totalModified = 0;
    m_stats.totalAlerts = 0;
    m_stats.totalRuleEvaluations = 0;
    m_stats.totalRegexMatches = 0;
    m_stats.totalThresholdChecks = 0;
    m_stats.peakQueueSize = 0;

    // activeRules跟随实际规则状态，不在重置时清零
    m_stats.activeRules = static_cast<int>(
        std::count_if(m_rules.cbegin(), m_rules.cend(),
                      [](const FilterRule& r) { return r.enabled; }));
}
