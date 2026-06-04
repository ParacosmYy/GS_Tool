/**
 * @file SerialDataEncoderStats.cpp
 * @brief 串口数据编解码器统计查询和重置方法实现
 *
 * 从 SerialDataEncoder.cpp 拆分而来，包含 stats() 和 resetStatistics() 方法。
 */

#include "utils/encoder/SerialDataEncoder.h"

/** @brief 获取操作统计快照 @return 当前统计数据的只读引用 */
const SerialDataEncoder::Stats &SerialDataEncoder::stats() const
{
    return m_stats;
}

/** @brief 重置所有统计计数器(编码/解码次数/字节数/错误数/按格式计数全部归零) */
void SerialDataEncoder::resetStatistics()
{
    m_stats.totalEncodes = 0;
    m_stats.totalDecodes = 0;
    m_stats.totalBytesEncoded = 0;
    m_stats.totalBytesDecoded = 0;
    m_stats.encodeErrors = 0;
    m_stats.decodeErrors = 0;
    for (int i = 0; i < 8; ++i) {
        m_stats.operationsByEncoding[i] = 0;
    }
}
