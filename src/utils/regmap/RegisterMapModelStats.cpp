/**
 * @file RegisterMapModelStats.cpp
 * @brief 寄存器地图模型 -- 统计查询方法
 * @author EmbedDebug Team
 * @date 2026-06-06
 */

#include "utils/regmap/RegisterMapModel.h"

/**
 * @brief 获取统计信息快照
 * @return 统计数据常引用
 */
const RegisterMapModel::Stats &RegisterMapModel::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计计数器为零
 */
void RegisterMapModel::resetStatistics()
{
    m_stats.totalEdits = 0;
    m_stats.totalLoads = 0;
    m_stats.totalSearches = 0;
    m_stats.totalExports = 0;
    m_stats.peakRegisterCount = m_stats.activeRegisterCount;
}
