/**
 * @file ProtocolTimerWidgetStats.cpp
 * @brief 协议定时分析器控件 -- 统计重置
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 提供 resetStatistics() 方法，清零所有内部计数器。
 */

#include "utils/protocol_timer/ProtocolTimerWidget.h"

/**
 * @brief 重置所有统计信息，恢复初始状态
 *
 * 清零事件计数、测量次数、峰值率等全局统计。
 * 不影响当前测量模式和定时器运行状态。
 * 调用者应同时清空 m_events 和 UI 控件。
 */
void ProtocolTimerWidget::resetStatistics()
{
    // 全局统计
    m_stats = Stats{};

    // 突发检测窗口
    m_burstWindow.clear();
}
