/**
 * @file JustFloatBridgeStats.cpp
 * @brief JustFloat协议桥 - 统计接口实现
 *
 * 从 JustFloatBridge.cpp 拆分而来，包含帧计数、错误计数、
 * 字节处理统计、通道解码峰值和重置方法。
 */

#include "protocol/bridge/JustFloatBridge.h"

/** @brief 获取已解析的帧计数 @return 帧数 */
quint64 JustFloatBridge::frameCount() const
{
    return m_frameCount;
}

/** @brief 获取解析错误计数 @return 错误数 */
quint64 JustFloatBridge::errorCount() const
{
    return m_errorCount;
}

/** @brief 获取已处理的字节总数 @return 字节数 */
qint64 JustFloatBridge::totalBytesProcessed() const
{
    return m_totalBytes;
}

/** @brief 获取已解码通道总数(跨所有帧累加) @return 通道解码总数 */
quint64 JustFloatBridge::totalChannelsDecoded() const
{
    return m_totalChannelsDecoded;
}

/** @brief 获取单帧最大通道数峰值 @return 峰值通道数 */
quint64 JustFloatBridge::peakChannelsPerFrame() const
{
    return m_peakChannelsPerFrame;
}

/** @brief 重置统计数据(帧计数/错误计数/字节数/通道解码数/峰值通道数)，不影响通道配置 */
void JustFloatBridge::resetStatistics()
{
    m_frameCount = 0;
    m_errorCount = 0;
    m_totalBytes = 0;
    m_totalChannelsDecoded = 0;
    m_peakChannelsPerFrame = 0;
}
