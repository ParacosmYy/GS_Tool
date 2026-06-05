/**
 * @file PidTunerWidgetStats.cpp
 * @brief PidTunerWidget 统计管理 — 统计获取与重置
 *
 * 将统计相关实现从主 .cpp 拆出，保持职责分离。
 *
 * @author EmbedDebug Team
 * @date 2026-06-06
 */

#include "utils/pid/PidTunerWidget.h"

/**
 * @brief 重置调试面板统计计数器
 *
 * 将 simulationsTriggered 和 repaintCount 清零。
 * 不影响当前UI状态或仿真结果。
 */
void PidTunerWidget::resetStatistics()
{
    m_stats = Stats{};
}
