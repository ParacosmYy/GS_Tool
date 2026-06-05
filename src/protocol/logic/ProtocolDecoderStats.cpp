/**
 * @file ProtocolDecoderStats.cpp
 * @brief ProtocolDecoder 统计信息接口实现
 *
 * 集中实现所有统计 getter 和 resetStatistics()。
 * 将统计逻辑从主实现文件分离，保持 ProtocolDecoder.cpp 聚焦解码算法。
 */

#include "protocol/logic/ProtocolDecoder.h"

/** @brief 获取累计解码帧数 */
quint64 ProtocolDecoder::totalFramesDecoded() const
{
    return m_totalFramesDecoded;
}

/** @brief 获取累计解码字节数 */
quint64 ProtocolDecoder::totalBytesDecoded() const
{
    return m_totalBytesDecoded;
}

/** @brief 获取累计解码错误数 */
quint64 ProtocolDecoder::totalErrors() const
{
    return m_totalErrors;
}

/** @brief 获取累计协议切换次数 */
quint64 ProtocolDecoder::totalProtocolsSwitched() const
{
    return m_totalProtocolsSwitched;
}

/** @brief 重置所有统计计数器 */
void ProtocolDecoder::resetStatistics()
{
    m_totalFramesDecoded     = 0;
    m_totalBytesDecoded      = 0;
    m_totalErrors            = 0;
    m_totalProtocolsSwitched = 0;
}
