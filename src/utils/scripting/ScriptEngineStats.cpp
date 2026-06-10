/**
 * @file ScriptEngineStats.cpp
 * @brief Script Engine 统计查询与重置
 *
 * 从 ScriptEngine.cpp 拆分而来，包含 stats getter 和 resetStatistics 方法。
 */

#include "utils/scripting/ScriptEngine.h"

// ============================================================
// 统计查询与重置
// ============================================================

/** @brief 获取统计信息(QVariantMap)，包含 avgDurationMs 平均执行时长 */
QVariantMap ScriptEngine::stats() const
{
    QVariantMap s;
    s["totalExecutions"] = m_totalExecutions;
    s["totalSuccesses"] = m_totalSuccesses;
    s["totalFailures"] = m_totalFailures;
    s["totalTimeouts"] = m_totalTimeouts;
    s["avgDurationMs"] = m_totalExecutions > 0
        ? static_cast<double>(m_totalDurationMs) / m_totalExecutions : 0.0;
    s["cacheHits"] = m_cacheHits;
    s["cacheMisses"] = m_cacheMisses;
    s["cacheSize"] = cacheSize();
    s["scriptCount"] = m_scripts.size();
    return s;
}

/** @brief 重置所有统计计数器(不影响脚本内容、缓存和上下文) */
void ScriptEngine::resetStatistics()
{
    m_totalExecutions = 0;
    m_totalSuccesses = 0;
    m_totalFailures = 0;
    m_totalTimeouts = 0;
    m_totalDurationMs = 0;
    m_cacheHits = 0;
    m_cacheMisses = 0;
}
