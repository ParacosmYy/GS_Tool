/**
 * @file DataLoggerSession.cpp
 * @brief DataLogger 会话管理 — 跳转定位、书签管理和统计接口
 *
 * 从 DataLoggerPlayback.cpp 拆分而来，包含时间戳跳转/书签定位、
 * 书签增删、会话累计统计查询和重置方法。
 */

#include "utils/log/DataLogger.h"
#include <QDateTime>
#include <algorithm>

// ---- 跳转定位(Seek) ----

/** @brief 线性扫描录制文件到目标时间戳 @param targetTimestamp 目标时间戳(ms) @return 实际定位到的偏移量 */
qint64 DataLogger::scanToTimestamp(qint64 targetTimestamp)
{
    if (!m_playbackFile) return -1;
    if (!m_playbackFile->seek(kHeaderSize)) return -1;

    qint64 foundTimestamp = -1;
    qint64 lastValidPos = kHeaderSize;
    int recordsFound = 0;

    RecordHeader hdr;
    QByteArray data;
    while (readNextRecord(hdr, data)) {
        qint64 recordTs = static_cast<qint64>(hdr.timestamp);
        if (recordTs > targetTimestamp) break;
        foundTimestamp = recordTs;
        lastValidPos = m_playbackFile->pos();
        recordsFound++;
    }

    if (foundTimestamp < 0) {
        if (!m_playbackFile->seek(kHeaderSize)) return -1;
        m_playedRecords = 0;
        m_nextRecordTime = 0;
        if (readNextRecord(hdr, data)) {
            m_nextRecordTime = static_cast<qint64>(hdr.timestamp);
            m_playbackFile->seek(kHeaderSize);
        }
        return 0;
    }

    if (!m_playbackFile->seek(lastValidPos)) return -1;
    m_playedRecords = recordsFound;

    if (readNextRecord(hdr, data)) {
        m_nextRecordTime = static_cast<qint64>(hdr.timestamp);
        m_playbackFile->seek(lastValidPos);
    } else {
        m_nextRecordTime = std::numeric_limits<qint64>::max();
    }

    return foundTimestamp;
}

/** @brief 跳转到指定时间戳位置 @param timestamp 目标时间戳(ms) @return true=跳转成功 */
bool DataLogger::seekToTimestamp(qint64 timestamp)
{
    QMutexLocker locker(&m_mutex);
    if (!m_playing) return false;
    if (timestamp < 0) {
        emit error(tr("无效的查找时间戳: %1").arg(timestamp));
        return false;
    }

    bool wasTimerRunning = m_playbackTimer->isActive();
    if (wasTimerRunning) m_playbackTimer->stop();

    qint64 actualTimestamp = scanToTimestamp(timestamp);
    if (actualTimestamp < 0) {
        emit error(tr("查找时间戳失败: %1").arg(timestamp));
        if (wasTimerRunning) m_playbackTimer->start();
        return false;
    }

    m_playbackOffset = actualTimestamp;
    m_playbackBaseTime = 0;
    m_playbackElapsed.restart();
    if (wasTimerRunning) m_playbackTimer->start();

    if (m_totalRecords > 0) {
        qreal progress = static_cast<qreal>(m_playedRecords) / m_totalRecords;
        emit playbackProgress(progress);
    }
    emit seekCompleted(actualTimestamp);
    return true;
}

/** @brief 跳转到指定书签位置 @param index 书签索引 @return true=跳转成功 */
bool DataLogger::seekToBookmark(int index)
{
    if (index < 0 || index >= m_bookmarks.size()) return false;
    return seekToTimestamp(m_bookmarks[index].timestamp);
}

// ---- 书签管理 ----

/** @brief 添加书签(标记当前录制位置) @param label 书签标签 @param streamId 数据流标识 */
void DataLogger::addBookmark(const QString& label, const QString& streamId)
{
    qint64 ts;
    if (m_recording) {
        ts = m_recordTimer.elapsed() - m_pauseOffset;
    } else {
        ts = QDateTime::currentDateTime().toMSecsSinceEpoch();
    }
    m_bookmarks.append(DataBookmark(ts, label, streamId));
    ++m_totalBookmarks;
    std::sort(m_bookmarks.begin(), m_bookmarks.end());
    emit bookmarksChanged();
}

/** @brief 获取所有书签列表 @return 书签向量 */
QVector<DataBookmark> DataLogger::bookmarks() const { return m_bookmarks; }

/** @brief 删除指定索引的书签 @param index 书签索引 */
void DataLogger::removeBookmark(int index)
{
    if (index < 0 || index >= m_bookmarks.size()) return;
    m_bookmarks.removeAt(index);
    emit bookmarksChanged();
}

/** @brief 清空所有书签 */
void DataLogger::clearBookmarks()
{
    if (m_bookmarks.isEmpty()) return;
    m_bookmarks.clear();
    emit bookmarksChanged();
}

// ---- 会话统计 ----

/** @brief 获取累计写入日志条数 @return 日志写入计数 */
quint64 DataLogger::totalLogsWritten() const { return m_totalLogsWritten; }

/** @brief 获取累计创建书签数量 @return 书签计数 */
quint64 DataLogger::totalBookmarks() const { return m_totalBookmarks; }

/** @brief 获取累计处理的记录总数 @return 记录计数 */
quint64 DataLogger::totalRecords() const { return m_totalRecords; }

/** @brief 获取累计录制的字节总数 @return 字节数 */
quint64 DataLogger::totalBytesRecorded() const { return m_totalBytesRecorded; }

/** @brief 获取累计回放操作次数 @return 回放计数 */
quint64 DataLogger::totalPlaybacks() const { return m_totalPlaybacks; }

/** @brief 获取累计错误次数 @return 错误计数 */
quint64 DataLogger::totalErrors() const { return m_totalErrors; }

/** @brief 重置所有统计计数器 */
void DataLogger::resetStats()
{
    m_totalLogsWritten = 0;
    m_totalBookmarks = 0;
    m_totalRecords = 0;
    m_totalBytesRecorded = 0;
    m_totalPlaybacks = 0;
    m_totalErrors = 0;
}
