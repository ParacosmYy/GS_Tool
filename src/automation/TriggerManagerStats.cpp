/**
 * @file TriggerManagerStats.cpp
 * @brief 触发器管理器 — 统计计数器查询与重置实现
 *
 * 从 TriggerManager.cpp 拆分而来，包含规则/触发器/动作
 * 的统计 getter 和 resetManagerStatistics 方法。
 */

#include "automation/TriggerManager.h"
#include "automation/TriggerEngine.h"

/** @brief 获取累计添加规则次数 @return 添加总次数 */
quint64 TriggerManager::totalRulesAdded() const { return m_totalRulesAdded; }

/** @brief 获取累计移除规则次数 @return 移除总次数 */
quint64 TriggerManager::totalRulesRemoved() const { return m_totalRulesRemoved; }

/** @brief 获取累计更新规则次数 @return 更新总次数 */
quint64 TriggerManager::totalRuleUpdates() const { return m_totalRuleUpdates; }

/** @brief 获取累计触发器命中次数 @return 命中总次数 */
quint64 TriggerManager::totalTriggersFired() const { return m_totalTriggersFired; }

/** @brief 获取累计动作执行次数 @return 执行总次数 */
quint64 TriggerManager::totalActionsExecuted() const { return m_totalActionsExecuted; }

/** @brief 获取累计错误次数（委托引擎） @return 错误总次数 */
quint64 TriggerManager::totalErrors() const { return m_engine ? m_engine->totalErrors() : 0; }

/** @brief 重置管理器统计计数器为初始值 */
void TriggerManager::resetManagerStatistics()
{
    m_totalRulesAdded = 0;
    m_totalRulesRemoved = 0;
    m_totalRuleUpdates = 0;
    m_totalTriggersFired = 0;
    m_totalActionsExecuted = 0;
    m_totalRuleImports = 0;
    m_totalRuleExports = 0;
    /* m_totalErrors 由 TriggerEngine::resetStats() 管理 */
}
