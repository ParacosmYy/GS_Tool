/**
 * @file LogicSamplerStats.cpp
 * @brief LogicSampler 统计信息接口实现
 *
 * 集中实现所有统计 getter 和 resetStatistics()。
 * 将统计逻辑从主实现文件分离，保持 LogicSampler.cpp 聚焦采集核心逻辑。
 */

#include "protocol/logic/LogicSampler.h"

/** @brief 获取累计处理的样本总数 */
quint64 LogicSampler::totalSamplesProcessed() const
{
    return m_totalSamplesProcessed;
}

/** @brief 获取累计接收的原始字节数 */
quint64 LogicSampler::totalBytesReceived() const
{
    return m_totalBytesReceived;
}

/** @brief 获取累计触发次数 */
quint64 LogicSampler::totalTriggersFired() const
{
    return m_totalTriggersFired;
}

/** @brief 获取累计采集会话数 */
quint64 LogicSampler::totalCaptureSessions() const
{
    return m_totalCaptureSessions;
}

/** @brief 获取最近一次采集的持续时间(ms) */
double LogicSampler::captureDurationMs() const
{
    return m_captureDurationMs;
}

/** @brief 重置所有统计计数器 */
void LogicSampler::resetStatistics()
{
    m_totalSamplesProcessed = 0;
    m_totalBytesReceived    = 0;
    m_totalTriggersFired    = 0;
    m_totalCaptureSessions  = 0;
    m_captureDurationMs     = 0.0;
}
