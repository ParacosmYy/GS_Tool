/**
 * @file SerialProtocolFuzzerStats.cpp
 * @brief 串口协议模糊测试器 — 统计查询与重置
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 提供 stats() 和 resetStatistics() 方法，
 * 核心变异逻辑见 @see SerialProtocolFuzzer.cpp
 */

#include "utils/fuzzer/SerialProtocolFuzzer.h"

/** @brief 获取累计统计数据快照
 *
 * 返回包含迭代次数、生成字节数、变异点数、平均变异率和崩溃次数的
 * 完整统计信息结构体副本。
 *
 * @return Stats 结构体快照
 */
SerialProtocolFuzzer::Stats SerialProtocolFuzzer::stats() const
{
    Stats s;
    s.totalIterations = m_totalIterations;
    s.totalBytesGenerated = m_totalBytesGenerated;
    s.totalMutations = m_totalMutations;
    s.totalFuzzFields = m_totalFuzzFields;
    s.crashesDetected = m_crashesDetected;

    /* 计算平均变异率 */
    s.avgMutationRate = (m_totalIterations > 0)
                            ? (m_totalMutations / m_totalIterations)
                            : 0;
    return s;
}

/** @brief 重置所有累计统计计数器和迭代位置
 *
 * 清空迭代计数、字节统计、变异计数和崩溃计数，
 * 同时重置迭代器索引到起始位置。
 * 不影响当前配置和 PRNG 种子。
 */
void SerialProtocolFuzzer::resetStatistics()
{
    m_totalIterations = 0;
    m_totalBytesGenerated = 0;
    m_totalMutations = 0;
    m_totalFuzzFields = 0;
    m_crashesDetected = 0;
    m_currentIndex = 0;
}
