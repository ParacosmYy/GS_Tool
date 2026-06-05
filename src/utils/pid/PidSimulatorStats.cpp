/**
 * @file PidSimulatorStats.cpp
 * @brief PidSimulator 统计管理 — 统计获取与重置
 *
 * 将统计相关实现从主 .cpp 拆出，保持职责分离。
 *
 * @author EmbedDebug Team
 * @date 2026-06-06
 */

#include "utils/pid/PidSimulator.h"

/**
 * @brief 重置仿真器统计计数器
 *
 * 将 simulationsRun 和 stepsComputed 清零。
 * 仿真器内部状态 (积分/被控对象) 不受影响。
 */
void PidSimulator::resetStatistics()
{
    m_stats = Stats{};
}
