/**
 * @file DataLoggerPlayback.cpp
 * @brief DataLogger 回放控制、跳转定位和书签管理
 *
 * 从 DataLogger.cpp 拆分而来，包含回放启停/暂停/变速、
 * 时间戳跳转、书签增删和统计接口。
 */

#include "utils/log/DataLogger.h"
#include <QDataStream>
#include <QDateTime>
#include <algorithm>

// ---- 回放控制 ----

/** @brief 开始数据回放(打开录制文件→扫描索引→启动定时器) @param filePath 录制文件路径 @return true=回放启动成功 */
bool DataLogger::startPlayback(const QString& filePath)
{
    if (m_playing) stopPlayback();

    m_playbackFile = new QFile(filePath, this);
    if (!m_playbackFile->open(QIODevice::ReadOnly)) {
        ++m_totalErrors;
        emit error(tr("无法打开日志文件: %1").arg(filePath));
        delete m_playbackFile;
        m_playbackFile = nullptr;
        return false;
    }

    QByteArray magic = m_playbackFile->read(3);
    if (magic != kMagic) {
        ++m_totalErrors;
        emit error(tr("无效的日志文件格式"));
        m_playbackFile->close();
        delete m_playbackFile;
        m_playbackFile = nullptr;
        return false;
    }

    QDataStream headerStream(m_playbackFile);
    headerStream.setByteOrder(QDataStream::BigEndian);

    quint8 version = 0;
    headerStream >> version;
    if (version != kVersion) {
        ++m_totalErrors;
        emit error(tr("不支持的日志版本: %1").arg(version));
        m_playbackFile->close();
        delete m_playbackFile;
        m_playbackFile = nullptr;
        return false;
    }

    quint32 count = 0;
    headerStream >> count;
    m_totalRecords = static_cast<int>(count);

    if (m_totalRecords == 0) {
        ++m_totalErrors;
        emit error(tr("日志文件为空"));
        m_playbackFile->close();
        delete m_playbackFile;
        m_playbackFile = nullptr;
        return false;
    }

    m_playedRecords = 0;
    m_playbackBaseTime = 0;
    m_playbackOffset = 0;
    m_nextRecordTime = 0;
    m_playbackPaused = false;
    m_playing = true;
    ++m_totalPlaybacks;

    RecordHeader hdr;
    QByteArray data;
    if (readNextRecord(hdr, data)) {
        m_nextRecordTime = static_cast<qint64>(hdr.timestamp);
        emit playbackData(data, hdr.direction);
        m_playedRecords = 1;
        m_playbackBaseTime = 0;
    }

    m_playbackElapsed.start();
    m_playbackTimer->start();
    return true;
}

/** @brief 停止回放，关闭文件，发射playbackStopped信号 */
void DataLogger::stopPlayback()
{
    if (!m_playing) return;
    m_playing = false;
    m_playbackTimer->stop();
    if (m_playbackFile) {
        m_playbackFile->close();
        delete m_playbackFile;
        m_playbackFile = nullptr;
    }
    emit playbackFinished();
}

/** @brief 暂停回放 */
void DataLogger::pausePlayback()
{
    if (!m_playing || m_playbackPaused) return;
    m_playbackPaused = true;
    m_playbackTimer->stop();
    m_playbackBaseTime += static_cast<qint64>(m_playbackElapsed.elapsed() * m_playbackSpeed);
}

/** @brief 恢复回放 */
void DataLogger::resumePlayback()
{
    if (!m_playing || !m_playbackPaused) return;
    m_playbackPaused = false;
    m_playbackElapsed.restart();
    m_playbackTimer->start();
}

/** @brief 设置回放速率 @param speed 速率倍数(1.0=正常) */
void DataLogger::setPlaybackSpeed(qreal speed)
{
    qreal newSpeed = qBound(0.1, speed, 100.0);
    if (m_playing && !m_playbackPaused) {
        m_playbackBaseTime += static_cast<qint64>(m_playbackElapsed.elapsed() * m_playbackSpeed);
        m_playbackElapsed.restart();
    }
    m_playbackSpeed = newSpeed;
}

/** @brief 查询是否正在回放 @return true=回放中 */
bool DataLogger::isPlaying() const { return m_playing; }

// ---- 回放定时器 ----

/** @brief 回放定时器回调，按时间戳发射下一条记录 */
void DataLogger::onPlaybackTick()
{
    if (!m_playing || m_playbackPaused) return;

    qint64 elapsed = static_cast<qint64>(m_playbackElapsed.elapsed() * m_playbackSpeed);
    qint64 currentTime = m_playbackOffset + m_playbackBaseTime + elapsed;

    while (m_playing && m_nextRecordTime <= currentTime) {
        RecordHeader hdr;
        QByteArray data;
        if (readNextRecord(hdr, data)) {
            emit playbackData(data, hdr.direction);
            m_nextRecordTime = static_cast<qint64>(hdr.timestamp);
            m_playedRecords++;
            if (m_totalRecords > 0) {
                qreal progress = static_cast<qreal>(m_playedRecords) / m_totalRecords;
                emit playbackProgress(progress);
            }
        } else {
            stopPlayback();
            return;
        }
    }
}

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

quint64 DataLogger::totalLogsWritten() const { return m_totalLogsWritten; }
quint64 DataLogger::totalBookmarks() const { return m_totalBookmarks; }
quint64 DataLogger::totalRecords() const { return m_totalRecords; }
quint64 DataLogger::totalBytesRecorded() const { return m_totalBytesRecorded; }
quint64 DataLogger::totalPlaybacks() const { return m_totalPlaybacks; }
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

