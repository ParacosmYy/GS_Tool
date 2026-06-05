/**
 * @file FirmwareDifferStats.cpp
 * @brief 固件差异引擎 — 统计计数器访问器与重置
 * @author EmbedDebug Team
 * @date 2026-06-06
 */

#include "utils/firmware/FirmwareDiffer.h"

/**
 * @brief 获取累计比较调用次数
 * @return compare()/compareRegion() 的累计调用数
 */
quint64 FirmwareDiffer::totalComparisons() const
{
    return m_totalComparisons;
}

/**
 * @brief 获取累计比较字节数
 * @return 所有比较操作涉及的累计字节数
 */
quint64 FirmwareDiffer::totalBytesCompared() const
{
    return m_totalBytesCompared;
}

/**
 * @brief 获取累计发现的差异块数
 * @return 所有比较操作发现的累计差异块数
 */
quint64 FirmwareDiffer::totalDiffsFound() const
{
    return m_totalDiffsFound;
}

/**
 * @brief 获取累计生成的补丁块数
 * @return 可用于生成补丁的累计变更块数
 */
quint64 FirmwareDiffer::totalPatchesGenerated() const
{
    return m_totalPatchesGenerated;
}

/**
 * @brief 重置所有累计统计计数器为初始值
 */
void FirmwareDiffer::resetStatistics()
{
    m_totalComparisons     = 0;
    m_totalBytesCompared   = 0;
    m_totalDiffsFound      = 0;
    m_totalPatchesGenerated = 0;
}
