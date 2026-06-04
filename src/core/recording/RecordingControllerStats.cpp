/**
 * @file RecordingControllerStats.cpp
 * @brief 录制/回放控制器 — 统计查询与重置方法
 *
 * 从 RecordingController.cpp 拆分而来，包含:
 *   - totalRecordings/totalPlaybacks/totalBytesPlayed 等统计getter
 *   - totalRecordedMs/longestRecordingMs/totalFramesRecorded 等时长统计getter
 *   - markersCreated/markersNavigated 标记统计getter
 *   - resetRecordingStatistics() 统计计数器重置
 *
 * 录制/回放交互逻辑保留在 RecordingController.cpp。
 */

#include "core/recording/RecordingController.h"

// ============================================================
// 统计查询接口
// ============================================================

/** @brief 获取累计录制次数 @return 录制总数 */
quint64 RecordingController::totalRecordings() const { return m_totalRecordings; }

/** @brief 获取累计回放次数 @return 回放总数 */
quint64 RecordingController::totalPlaybacks() const { return m_totalPlaybacks; }

/** @brief 获取累计回放字节数 @return 回放字节总数 */
quint64 RecordingController::totalBytesPlayed() const { return m_totalBytesPlayed; }

/** @brief 获取累计错误次数 @return 错误总数 */
quint64 RecordingController::totalErrors() const { return m_totalErrors; }

/** @brief 获取累计书签创建数 @return 书签总数 */
quint64 RecordingController::totalBookmarks() const { return m_totalBookmarks; }

/** @brief 获取累计录制错误数 @return 录制错误总数 */
quint64 RecordingController::totalRecordingErrors() const { return m_totalRecordingErrors; }

/** @brief 获取累计录制总时长(毫秒) @return 时长总和 */
qint64 RecordingController::totalRecordedMs() const { return m_totalRecordedMs; }

/** @brief 获取单次最长录制时长(毫秒) @return 最长时长 */
qint64 RecordingController::longestRecordingMs() const { return m_longestRecordingMs; }

/** @brief 获取累计录制数据帧总数 @return 帧总数 */
quint64 RecordingController::totalFramesRecorded() const { return m_totalFramesRecorded; }

/** @brief 获取累计创建的标记总数 @return 标记创建数 */
quint64 RecordingController::markersCreated() const { return m_markersCreated; }

/** @brief 获取累计导航(跳转)的标记总数 @return 标记导航数 */
quint64 RecordingController::markersNavigated() const { return m_markersNavigated; }

/** @brief 获取累计录制暂停次数 @return 暂停次数 */
quint64 RecordingController::totalPauses() const { return m_totalPauses; }

/** @brief 获取累计录制恢复次数 @return 恢复次数 */
quint64 RecordingController::totalResumes() const { return m_totalResumes; }

/** @brief 获取累计数据段写入次数 @return 段写入次数 */
quint64 RecordingController::totalSegmentWrites() const { return m_totalSegmentWrites; }

/** @brief 获取累计回放手动停止次数 @return 回放停止次数 */
quint64 RecordingController::totalPlaybackStops() const { return m_totalPlaybackStops; }

// ============================================================
// 统计重置
// ============================================================

/** @brief 重置所有统计计数器 */
void RecordingController::resetRecordingStatistics()
{
    m_totalRecordings = 0;
    m_totalPlaybacks = 0;
    m_totalBytesPlayed = 0;
    m_totalErrors = 0;
    m_totalBookmarks = 0;
    m_totalRecordingErrors = 0;
    m_totalRecordedMs = 0;
    m_longestRecordingMs = 0;
    m_totalFramesRecorded = 0;
    m_markersCreated = 0;
    m_markersNavigated = 0;
    m_totalPauses = 0;
    m_totalResumes = 0;
    m_totalSegmentWrites = 0;
    m_totalPlaybackStops = 0;
}
