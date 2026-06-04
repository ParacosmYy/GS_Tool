/**
 * @file SerialTimingAnalyzerStats.cpp
 * @brief 串口时序分析器 -- 统计计数器查询与重置实现
 *
 * 从 SerialTimingAnalyzer.cpp 拆分而来，包含累计统计 getter
 * 和 resetStatistics 方法。
 */

#include "serial/timing/SerialTimingAnalyzer.h"

/** @brief 获取运行累计统计 @return TimingRuntimeStats常量引用 */
const TimingRuntimeStats& SerialTimingAnalyzer::stats() const
{
    return m_stats;
}

/** @brief 重置累计统计计数器
 *
 *  将 TimingRuntimeStats 结构体所有字段归零。
 *  不影响当前采样数据(字节采样、帧时序)和采集状态。
 */
void SerialTimingAnalyzer::resetStatistics()
{
    m_stats = TimingRuntimeStats{};
}
