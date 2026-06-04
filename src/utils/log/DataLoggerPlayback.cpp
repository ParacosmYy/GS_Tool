/**
 * @file DataLoggerPlayback.cpp
 * @brief DataLogger 回放控制和定时器回调
 *
 * 从 DataLogger.cpp 拆分而来，包含回放启停/暂停/变速和定时器驱动。
 * 跳转定位、书签管理和统计接口见 DataLoggerSession.cpp。
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
