/**
 * @file LedMatrixSimulatorStats.cpp
 * @brief LED矩阵模拟器 — 统计重置实现
 *
 * 从 LedMatrixSimulator.cpp 拆分而来，专注运行统计计数器的重置逻辑。
 */

#include "widgets/led_matrix/LedMatrixSimulator.h"

/**
 * @brief 重置所有统计计数器为零
 *
 * 清零: totalLedChanges / totalPaints / totalClears / totalImageImports /
 *       totalExports / totalMouseClicks / maxGridSize
 * 注意: LED颜色网格数据不受影响，仅重置累计计数器。
 */
void LedMatrixSimulator::resetStatistics()
{
    m_stats = Stats{};
}
