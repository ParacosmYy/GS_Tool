/**
 * @file DataStreamRecorderStats.cpp
 * @brief 数据流记录器 — 统计重置方法
 *
 * 从 DataStreamRecorder.cpp 拆分而来，包含:
 *   - resetStatistics() 累计全局统计计数器重置
 *
 * 录制逻辑保留在 DataStreamRecorder.cpp。
 */

#include "core/recording/DataStreamRecorder.h"

// ============================================================
// 统计重置
// ============================================================

/** @brief 重置累计全局统计计数器
 *
 * 将所有 m_stats 字段归零。不影响当前录制会话。
 * 典型调用场景: 用户在设置中点击"重置统计"按钮。
 */
void DataStreamRecorder::resetStatistics()
{
    m_stats.totalRecordingSessions = 0;
    m_stats.totalBytesWritten = 0;
    m_stats.totalFilesCreated = 0;
    m_stats.totalRecordingTimeSec = 0.0;
    m_stats.peakThroughputBytesPerSec = 0.0;
}
