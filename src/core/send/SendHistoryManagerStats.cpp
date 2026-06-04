/**
 * @file SendHistoryManagerStats.cpp
 * @brief 发送历史管理器 - 统计计数器接口实现
 *
 * 从 SendHistoryManager.cpp 拆分而来，包含所有统计 getter 和
 * resetHistoryStatistics 方法。
 */

#include "core/send/SendHistoryManager.h"

/** @brief 获取累计添加的历史记录数 @return 添加次数 */
quint64 SendHistoryManager::totalAdds() const
{
    return m_totalAdds;
}

/** @brief 获取累计清空历史的次数 @return 清空次数 */
quint64 SendHistoryManager::totalClears() const
{
    return m_totalClears;
}

/** @brief 获取累计召回(补全选中)的次数 @return 召回次数 */
quint64 SendHistoryManager::totalRecalls() const
{
    return m_totalRecalls;
}

/** @brief 获取累计搜索/补全弹出次数 @return 搜索次数 */
quint64 SendHistoryManager::totalSearches() const
{
    return m_totalSearches;
}

/** @brief 获取累计选中补全项次数(鼠标点击+键盘Enter) @return 选中次数 */
quint64 SendHistoryManager::totalSelects() const
{
    return m_totalSelects;
}

/** @brief 获取历史列表的峰值大小(条目数) @return 峰值大小 */
quint64 SendHistoryManager::peakHistorySize() const
{
    return m_peakHistorySize;
}

/** @brief 重置所有统计计数器(添加/清空/召回/搜索/选中/峰值) */
void SendHistoryManager::resetHistoryStatistics()
{
    m_totalAdds = 0;
    m_totalClears = 0;
    m_totalRecalls = 0;
    m_totalSearches = 0;
    m_totalSelects = 0;
    m_peakHistorySize = 0;
}
