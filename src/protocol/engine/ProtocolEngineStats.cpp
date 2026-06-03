/**
 * @file ProtocolEngineStats.cpp
 * @brief ProtocolEngine 统计计数器接口实现
 *
 * 从 ProtocolEngine.cpp 拆分而来，包含所有统计相关的
 * getter 方法和计数器重置方法。
 */

#include "protocol/engine/ProtocolEngine.h"

/**
 * @brief 获取已成功解析的帧数
 * @return 成功解析帧计数
 */
int ProtocolEngine::framesParsed() const
{
    return static_cast<int>(m_framesParsed);
}

/**
 * @brief 获取解析错误次数
 * @return 解析错误计数
 */
int ProtocolEngine::parseErrors() const
{
    return m_parseErrors;
}

/**
 * @brief 获取已成功解析的帧数（64位）
 * @return 成功解析帧计数
 */
quint64 ProtocolEngine::framesParsedCount() const
{
    return m_framesParsed;
}

/**
 * @brief 获取因验证失败而被拒绝的帧数
 * @return 被拒绝帧计数
 */
quint64 ProtocolEngine::framesRejected() const
{
    return m_framesRejected;
}

/**
 * @brief 获取引擎处理的总字节数
 * @return 累计处理的字节总数
 */
quint64 ProtocolEngine::totalBytesProcessed() const
{
    return m_totalBytesProcessed;
}

/**
 * @brief 获取校验验证执行总次数(含通过和失败)
 * @return 验证总次数
 */
quint64 ProtocolEngine::totalValidations() const
{
    return m_totalValidations;
}

/**
 * @brief 获取解析错误总数(64位，含校验失败/格式错/溢出)
 * @return 错误总数
 */
quint64 ProtocolEngine::totalParseErrors() const
{
    return m_totalParseErrors;
}

/**
 * @brief 获取最后一次成功解析的时间戳
 * @return 毫秒级时间戳（自Unix纪元起），尚未解析过时返回0
 */
qint64 ProtocolEngine::lastParseTimestamp() const
{
    return m_lastParseTimestamp;
}

/**
 * @brief 获取已处理的数据包总数（含成功和失败）
 * @return 成功解析帧数 + 被拒绝帧数
 */
quint64 ProtocolEngine::totalPacketsProcessed() const
{
    return m_framesParsed + m_framesRejected;
}

/**
 * @brief 获取已解析的字节总数（仅成功解析的帧内字节）
 * @return 字节总数
 */
quint64 ProtocolEngine::totalBytesParsed() const
{
    return m_totalBytesParsed;
}

/**
 * @brief 获取CRC校验错误次数
 * @return CRC错误计数
 */
quint64 ProtocolEngine::totalCrcErrors() const
{
    return m_totalCrcErrors;
}

/**
 * @brief 重置所有解析统计计数器
 *
 * 将帧计数、拒绝计数、字节总数和时间戳全部归零。
 * 不影响当前 schema 设置和缓冲区内容。
 */
void ProtocolEngine::resetParseStatistics()
{
    m_framesParsed = 0;
    m_parseErrors = 0;
    m_framesRejected = 0;
    m_totalBytesProcessed = 0;
    m_totalValidations = 0;
    m_totalParseErrors = 0;
    m_lastParseTimestamp = 0;
    m_totalCrcErrors = 0;
    m_totalBytesParsed = 0;
}

/** @brief 重置所有引擎统计计数器(等同于resetParseStatistics) */
void ProtocolEngine::resetEngineStatistics()
{
    resetParseStatistics();
}

/** @brief 重置所有统计计数器(别名，调用resetEngineStatistics) */
void ProtocolEngine::resetStats()
{
    resetEngineStatistics();
}
