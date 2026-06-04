/**
 * @file NotificationHistoryStats.cpp
 * @brief 通知历史记录控件 -- 统计重置
 */
#include "core/notification/NotificationHistory.h"

/** @brief 重置所有统计计数器归零 */
void NotificationHistory::resetStatistics()
{
    m_stats.totalNotifications = 0;
    for (int i = 0; i < 5; ++i) {
        m_stats.totalByLevel[i] = 0;
    }
    m_stats.totalSearches = 0;
    m_stats.totalExports = 0;
    m_stats.totalClears = 0;
    m_stats.peakHistorySize = 0;
}
