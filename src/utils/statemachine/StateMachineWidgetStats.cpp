/**
 * @file StateMachineWidgetStats.cpp
 * @brief 状态机控件统计方法实现
 *
 * 从 StateMachineWidget.cpp 拆分而来，包含统计 getter 和 reset 方法。
 */

#include "utils/statemachine/StateMachineWidget.h"

/** @brief 获取状态拖动次数 @return 拖动总次数 */
quint64 StateMachineWidget::totalStateMoves() const
{
    return m_totalStateMoves;
}

/** @brief 获取重绘次数 @return 重绘总次数 */
quint64 StateMachineWidget::totalRepaints() const
{
    return m_totalRepaints;
}

/** @brief 获取鼠标点击次数 @return 点击总次数 */
quint64 StateMachineWidget::totalMouseClicks() const
{
    return m_totalMouseClicks;
}

/** @brief 重置所有统计计数器归零 */
void StateMachineWidget::resetStatistics()
{
    m_totalStateMoves = 0;
    m_totalRepaints = 0;
    m_totalMouseClicks = 0;
}
