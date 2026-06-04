/**
 * @file ScopeChannelManagerStats.cpp
 * @brief ScopeChannelManager 统计计数器重置实现
 */
#include "widgets/scope/ScopeChannelManager.h"

/**
 * @brief 重置所有统计计数器为零
 *
 * 保留 activeChannelCount 和 peakChannelCount 不重置(反映当前真实状态)，
 * 其余累计计数器归零。peakChannelCount 重置为当前活跃通道数。
 */
void ScopeChannelManager::resetStatistics()
{
    m_stats.totalChannelAdds = 0;
    m_stats.totalChannelRemoves = 0;
    m_stats.totalRangeChanges = 0;
    m_stats.totalVisibilityToggles = 0;
    m_stats.totalColorChanges = 0;
    m_stats.totalScaleOperations = 0;
    m_stats.peakChannelCount = m_stats.activeChannelCount; // 重置峰值为当前值
}
