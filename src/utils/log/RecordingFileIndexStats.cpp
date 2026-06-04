/**
 * @file RecordingFileIndexStats.cpp
 * @brief 录制文件格式统计重置实现
 *
 * 从 RecordingFileIndex.cpp 拆分而来，
 * 包含 resetFileFormatStatistics() 统计重置方法。
 *
 * 统计计数器涵盖:
 *   - 保存/加载操作计数
 *   - 序列化/反序列化错误计数
 *   - 累计读写字节数
 *   - 索引查找和缓存命中计数
 */

#include "utils/log/RecordingFileFormat.h"

/** @brief 重置文件格式统计计数器(包含所有统计)
 *
 * 将以下计数器归零:
 *   - m_totalSaves: 保存次数
 *   - m_totalLoads: 加载次数
 *   - m_totalErrors: 总错误次数
 *   - m_totalBytesWritten: 累计写入字节数
 *   - m_totalBytesRead: 累计读取字节数
 *   - m_serializationErrors: 序列化错误次数
 *   - m_deserializationErrors: 反序列化错误次数
 *   - m_lookupsPerformed: 索引查找次数
 *   - m_cacheHits: 缓存命中次数
 *   - m_totalValidationFailures: 文件校验失败次数
 *   - m_totalCacheMisses: 缓存未命中次数
 *   - m_totalIndexBuilds: 索引构建次数
 *   - m_totalSegmentsLoaded: 数据段加载数
 *   - m_totalSeekOperations: 文件定位操作次数
 */
void RecordingFileFormat::resetFileFormatStatistics()
{
    m_totalSaves = 0;
    m_totalLoads = 0;
    m_totalErrors = 0;
    m_totalBytesWritten = 0;
    m_totalBytesRead = 0;
    m_serializationErrors = 0;
    m_deserializationErrors = 0;
    m_lookupsPerformed = 0;
    m_cacheHits = 0;
    m_totalValidationFailures = 0;
    m_totalCacheMisses = 0;
    m_totalIndexBuilds = 0;
    m_totalSegmentsLoaded = 0;
    m_totalSeekOperations = 0;
}
