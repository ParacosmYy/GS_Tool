/**
 * @file SerialPortProfilerStats.cpp
 * @brief 串口流量分析器 -- 统计计数器查询与重置实现
 *
 * 从 SerialPortProfiler.cpp 拆分而来，包含累计统计 getter
 * 和 resetStatistics 方法。
 */

#include "serial/profiler/SerialPortProfiler.h"

/** @brief 获取运行累计统计数据 @return ProfilerStats常量引用 */
const ProfilerStats& SerialPortProfiler::stats() const
{
    return m_stats;
}

/** @brief 重置累计统计计数器
 *
 *  将 ProfilerStats 结构体所有字段归零。
 *  不影响当前画像数据(字节分布、包时间等)和采集状态。
 */
void SerialPortProfiler::resetStatistics()
{
    m_stats = ProfilerStats{};
}
