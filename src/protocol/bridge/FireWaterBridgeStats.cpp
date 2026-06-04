/**
 * @file FireWaterBridgeStats.cpp
 * @brief FireWater协议桥 - 统计接口实现
 *
 * 从 FireWaterBridge.cpp 拆分而来，包含帧计数、错误计数、
 * 字节处理统计和重置方法。
 */

#include "protocol/bridge/FireWaterBridge.h"

/** @brief 获取已解析的帧计数 @return 帧数 */
quint64 FireWaterBridge::frameCount() const
{
    return m_frameCount;
}

/** @brief 获取解析错误计数 @return 错误数 */
quint64 FireWaterBridge::errorCount() const
{
    return m_errorCount;
}

/** @brief 获取已处理的字节总数 @return 字节数 */
qint64 FireWaterBridge::totalBytesProcessed() const
{
    return m_totalBytes;
}

/** @brief 获取已解码通道总数(跨所有帧累加) @return 通道解码总数 */
quint64 FireWaterBridge::totalChannelsDecoded() const
{
    return m_totalChannelsDecoded;
}

/** @brief 获取FireWater协议匹配(成功解析)的总帧数 @return 匹配总帧数 */
quint64 FireWaterBridge::fireWaterMatches() const
{
    return m_fireWaterMatches;
}

/** @brief 重置统计数据(帧计数/错误计数/字节数/通道解码数/协议匹配数) */
void FireWaterBridge::resetStatistics()
{
    m_frameCount = 0;
    m_errorCount = 0;
    m_totalBytes = 0;
    m_totalChannelsDecoded = 0;
    m_fireWaterMatches = 0;
}
