/**
 * @file BridgeConfigPanelStats.cpp
 * @brief 桥接配置面板 — 统计计数器查询与重置实现
 *
 * 从 BridgeConfigPanel.cpp 拆分而来，包含面板级统计 getter 和 reset。
 */

#include "connection/bridge/BridgeConfigPanel.h"

/** @brief 获取累计创建规则次数 @return 创建次数 */
quint64 BridgeConfigPanel::totalRulesCreated() const
{
    return m_stats.totalRulesCreated;
}

/** @brief 获取累计修改规则次数 @return 修改次数 */
quint64 BridgeConfigPanel::totalRulesModified() const
{
    return m_stats.totalRulesModified;
}

/** @brief 获取累计删除规则次数 @return 删除次数 */
quint64 BridgeConfigPanel::totalRulesDeleted() const
{
    return m_stats.totalRulesDeleted;
}

/** @brief 重置面板统计计数器归零 */
void BridgeConfigPanel::resetStatistics()
{
    m_stats = BridgeConfigPanelStats{};
}
