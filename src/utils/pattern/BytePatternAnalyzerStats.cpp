/**
 * @file BytePatternAnalyzerStats.cpp
 * @brief 字节模式搜索器统计查询和重置方法实现
 *
 * 从 BytePatternAnalyzer.cpp 拆分而来，包含 stats() 和 resetStatistics()。
 */

#include "utils/pattern/BytePatternAnalyzer.h"

/** @brief 获取累计统计数据快照 @return Stats 结构体副本 */
BytePatternAnalyzer::Stats BytePatternAnalyzer::stats() const
{
    /* 同步模式数量(可能被外部间接修改)后返回快照 */
    m_stats.patternsRegistered = static_cast<quint64>(m_patterns.size());
    return m_stats;
}

/** @brief 重置所有累计统计计数器(搜索次数/命中数/扫描字节数/耗时归零) */
void BytePatternAnalyzer::resetStatistics()
{
    m_stats.totalSearches = 0;
    m_stats.totalMatches = 0;
    m_stats.totalBytesScanned = 0;
    m_stats.searchTimeMs = 0;
    /* 注意: patternsRegistered 不重置，它反映当前注册数 */
}
