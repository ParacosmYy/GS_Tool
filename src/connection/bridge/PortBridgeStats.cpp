/**
 * @file PortBridgeStats.cpp
 * @brief 多端口桥接引擎 — 统计计数器查询与重置实现
 *
 * 从 PortBridge.cpp 拆分而来，包含全局/单规则统计 getter 和 reset。
 */

#include "connection/bridge/PortBridge.h"

/** @brief 获取所有规则的合并统计 @return 累计统计快照 */
BridgeStats PortBridge::totalStatistics() const
{
    BridgeStats total;
    for (auto it = m_ruleStats.constBegin(); it != m_ruleStats.constEnd(); ++it) {
        total.bytesForwarded  += it.value().bytesForwarded;
        total.bytesFiltered   += it.value().bytesFiltered;
        total.bytesDropped    += it.value().bytesDropped;
        total.packetsForwarded += it.value().packetsForwarded;
        total.errors          += it.value().errors;
    }
    return total;
}

/**
 * @brief 获取指定规则的统计
 * @param ruleName 规则名称
 * @return 该规则的统计快照，规则不存在则返回全零
 */
BridgeStats PortBridge::bridgeStatistics(const QString& ruleName) const
{
    return m_ruleStats.value(ruleName, BridgeStats{});
}

/** @brief 获取当前活跃（已启用）桥接规则数 @return 活跃规则数 */
quint64 PortBridge::totalBridgesActive() const
{
    quint64 count = 0;
    for (const BridgeRule& rule : m_rules) {
        if (rule.enabled) {
            ++count;
        }
    }
    return count;
}

/** @brief 获取全局累计转发字节数 @return 字节数 */
quint64 PortBridge::totalBytesForwarded() const
{
    return totalStatistics().bytesForwarded;
}

/** @brief 获取全局累计过滤字节数 @return 字节数 */
quint64 PortBridge::totalBytesFiltered() const
{
    return totalStatistics().bytesFiltered;
}

/** @brief 获取全局累计丢弃字节数 @return 字节数 */
quint64 PortBridge::totalBytesDropped() const
{
    return totalStatistics().bytesDropped;
}

/** @brief 获取全局累计错误次数 @return 错误数 */
quint64 PortBridge::totalErrors() const
{
    return totalStatistics().errors;
}

/** @brief 重置所有规则的统计计数器归零 */
void PortBridge::resetStatistics()
{
    for (auto it = m_ruleStats.begin(); it != m_ruleStats.end(); ++it) {
        it.value() = BridgeStats{};
    }
}
