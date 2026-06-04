/**
 * @file TriggerEngineStats.cpp
 * @brief 触发器引擎统计查询和重置方法实现
 *
 * 从 TriggerEngine.cpp 拆分而来，包含匹配计数查询、
 * 扩展统计getter和统计重置方法。
 */

#include "automation/TriggerEngine.h"

// ---- 基础统计查询 ----

/** @brief 获取累计成功匹配次数 @return 匹配次数 */
int TriggerEngine::matchCount() const
{
    return m_matchCount;
}

/** @brief 获取上次匹配距现在的毫秒数 @return 距上次匹配的毫秒数，无匹配返回-1 */
qint64 TriggerEngine::msSinceLastMatch() const
{
    if (!m_hasMatched) {
        return -1;
    }
    return m_lastMatchTimer.elapsed();
}

/** @brief 重置统计计数(不重置规则) */
void TriggerEngine::resetStatistics()
{
    m_matchCount = 0;
    m_hasMatched = false;
    m_ruleMatchCounts.fill(0);
}

/** @brief 获取指定规则的匹配次数 @param index 规则索引 @return 该规则命中次数，无效索引返回0 */
int TriggerEngine::ruleMatchCount(int index) const
{
    if (index >= 0 && index < m_ruleMatchCounts.size()) {
        return m_ruleMatchCounts.at(index);
    }
    return 0;
}

// ---- 扩展统计 getter ----

/** @brief 获取总评估次数 @return 总评估次数 */
quint64 TriggerEngine::totalEvaluations() const
{
    return m_totalEvaluations;
}

/** @brief 获取总匹配成功次数（quint64精度） @return 总匹配次数 */
quint64 TriggerEngine::totalMatches() const
{
    return m_totalMatches;
}

/** @brief 获取总动作执行次数 @return 总动作执行次数 */
quint64 TriggerEngine::totalActionsExecuted() const
{
    return m_totalActionsExecuted;
}

/** @brief 获取总错误次数 @return 总错误次数 */
quint64 TriggerEngine::totalErrors() const
{
    return m_totalErrors;
}

/** @brief 获取总规则评估次数 @return 规则评估总次数 */
quint64 TriggerEngine::totalTriggersEvaluated() const
{
    return m_totalTriggersEvaluated;
}

/** @brief 获取总触发器命中次数 @return 触发器命中总次数 */
quint64 TriggerEngine::totalTriggersFired() const
{
    return m_totalTriggersFired;
}

/** @brief 获取总跳过禁用规则的次数 @return 跳过禁用规则总次数 */
quint64 TriggerEngine::totalTriggersDisabled() const
{
    return m_totalTriggersDisabled;
}

/** @brief 获取总动作执行错误次数 @return 动作执行错误总次数 */
quint64 TriggerEngine::totalActionErrors() const
{
    return m_totalActionErrors;
}

/** @brief 获取累计规则激活(从禁用切到启用)次数 @return 激活总次数 */
quint64 TriggerEngine::totalRulesActive() const
{
    return m_totalRulesActive;
}

/** @brief 获取历史同时启用规则数峰值 @return 峰值活跃规则数 */
int TriggerEngine::peakRulesActive() const
{
    return m_peakRulesActive;
}

/** @brief 重置所有扩展统计计数器为初始值(不影响规则列表和启用状态) */
void TriggerEngine::resetStats()
{
    m_totalEvaluations = 0;
    m_totalMatches = 0;
    m_totalActionsExecuted = 0;
    m_totalErrors = 0;
    m_totalTriggersEvaluated = 0;
    m_totalTriggersFired = 0;
    m_totalTriggersDisabled = 0;
    m_totalActionErrors = 0;
    m_totalRulesActive = 0;
    m_peakRulesActive = 0;
    resetStatistics();
}
