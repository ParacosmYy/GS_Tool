/**
 * @file StateMachineDesignerStats.cpp
 * @brief 状态机设计器统计方法实现
 *
 * 从 StateMachineDesigner.cpp 拆分而来，包含统计 getter 和 reset 方法。
 */

#include "utils/statemachine/StateMachineDesigner.h"

/** @brief 获取累计添加状态次数 @return 添加状态总次数 */
quint64 StateMachineDesigner::totalStatesAdded() const
{
    return m_totalStatesAdded;
}

/** @brief 获取累计移除状态次数 @return 移除状态总次数 */
quint64 StateMachineDesigner::totalStatesRemoved() const
{
    return m_totalStatesRemoved;
}

/** @brief 获取累计添加迁移次数 @return 添加迁移总次数 */
quint64 StateMachineDesigner::totalTransitionsAdded() const
{
    return m_totalTransitionsAdded;
}

/** @brief 获取累计移除迁移次数 @return 移除迁移总次数 */
quint64 StateMachineDesigner::totalTransitionsRemoved() const
{
    return m_totalTransitionsRemoved;
}

/** @brief 获取累计验证次数 @return 验证总次数 */
quint64 StateMachineDesigner::totalValidations() const
{
    return m_totalValidations;
}

/** @brief 获取累计导出次数 @return 导出总次数 */
quint64 StateMachineDesigner::totalExports() const
{
    return m_totalExports;
}

/** @brief 重置所有统计计数器归零 */
void StateMachineDesigner::resetStatistics()
{
    m_totalStatesAdded = 0;
    m_totalStatesRemoved = 0;
    m_totalTransitionsAdded = 0;
    m_totalTransitionsRemoved = 0;
    m_totalValidations = 0;
    m_totalExports = 0;
}
