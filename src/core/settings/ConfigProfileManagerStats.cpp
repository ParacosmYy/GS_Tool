/**
 * @file ConfigProfileManagerStats.cpp
 * @brief 设备配置档案管理器 -- 统计重置实现
 *
 * 独立编译单元，实现resetStatistics()方法。
 * 将运行时统计数据清零，便于测试和审计。
 */

#include "core/settings/ConfigProfileManager.h"

/**
 * @brief 重置所有统计数据为零
 *
 * 将所有累计计数器归零，peakProfiles和activeProfileIndex
 * 重置为初始值。不影响档案数据本身，仅清除统计信息。
 * 典型场景: 单元测试重置、统计数据审计周期开始。
 */
void ConfigProfileManager::resetStatistics()
{
    m_stats.totalProfilesCreated = 0;
    m_stats.totalProfilesLoaded = 0;
    m_stats.totalProfilesDeleted = 0;
    m_stats.totalImports = 0;
    m_stats.totalExports = 0;
    m_stats.totalSwitches = 0;
    m_stats.peakProfiles = 0;
    m_stats.activeProfileIndex = -1;
}
