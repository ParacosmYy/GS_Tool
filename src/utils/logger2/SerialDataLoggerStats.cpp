/**
 * @file SerialDataLoggerStats.cpp
 * @brief 高级串口数据日志记录器 -- 统计查询与重置
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 提供 stats() 和 resetStatistics() 方法。
 * 核心逻辑见 @see SerialDataLogger.cpp
 */

#include "utils/logger2/SerialDataLogger.h"

/**
 * @brief 获取全局统计快照
 *
 * 返回包含累计字节数、记录数、文件数、轮转/压缩次数、
 * 缓冲区大小和当前文件大小的统计结构体副本。
 *
 * @return Stats 结构体快照
 */
SerialDataLogger::Stats SerialDataLogger::stats() const
{
    Stats s;
    s.totalBytesLogged  = m_totalBytesLogged;
    s.totalRecords      = m_totalRecords;
    s.totalFilesCreated = m_totalFilesCreated;
    s.totalRotations    = m_totalRotations;
    s.totalCompressions = m_totalCompressions;
    s.bufferSize        = static_cast<quint64>(m_buffer.size());
    s.currentFileSize   = m_currentFileSize;
    return s;
}

/**
 * @brief 重置所有统计计数器
 *
 * 清零累计字节数、记录数、文件数、轮转次数和压缩次数。
 * 不影响当前日志记录状态、配置和缓冲区内容。
 */
void SerialDataLogger::resetStatistics()
{
    m_totalBytesLogged  = 0;
    m_totalRecords      = 0;
    m_totalFilesCreated = 0;
    m_totalRotations    = 0;
    m_totalCompressions = 0;
    m_currentFileSize   = 0;
}
