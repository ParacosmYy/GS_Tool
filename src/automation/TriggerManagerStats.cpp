/**
 * @file TriggerManagerStats.cpp
 * @brief 触发器管理器 — 统计计数器查询与重置实现
 *
 * 从 TriggerManager.cpp 拆分而来，包含规则/触发器/动作
 * 的统计 getter 和 resetManagerStatistics 方法。
 */

#include "automation/TriggerManager.h"
#include "automation/TriggerEngine.h"

/** @brief 获取累计错误次数（委托引擎） @return 错误总次数 */
quint64 TriggerManager::totalErrors() const { return m_engine ? m_engine->totalErrors() : 0; }
