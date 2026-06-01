#include "utils/DataLogger.h"
#include <QDataStream>
#include <QFileInfo>
#include <QDateTime>
#include <QMutexLocker>
#include <algorithm>

DataLogger::DataLogger(QObject* parent)
    : QObject(parent)
    , m_playbackTimer(new QTimer(this))
{
    m_playbackTimer->setSingleShot(false);
    m_playbackTimer->setInterval(1); // 1ms精度
    connect(m_playbackTimer, &QTimer::timeout,
            this, &DataLogger::onPlaybackTick);
}

DataLogger::~DataLogger()
{
    if (m_recording) stopRecording();
    if (m_playing) stopPlayback();
}

// ---- 录制控制 ----

bool DataLogger::startRecording(const QString& filePath)
{
    if (m_recording) {
        stopRecording();
    }

    m_recordFile = new QFile(filePath, this);
    if (!m_recordFile->open(QIODevice::WriteOnly)) {
        emit error(tr("Cannot create log file: %1").arg(filePath));
        delete m_recordFile;
        m_recordFile = nullptr;
        return false;
    }

    m_recordCount = 0;
    m_pauseOffset = 0;
    m_paused = false;
    m_recording = true;
    m_recordTimer.start();

    writeHeader();
    emit recordingStarted();
    return true;
}

void DataLogger::stopRecording()
{
    if (!m_recording) return;

    m_recording = false;
    qint64 duration = m_recordTimer.elapsed() - m_pauseOffset;

    if (m_recordFile) {
        // 回写header中的record count
        m_recordFile->seek(4); // 跳过magic(3) + version(1)
        QDataStream stream(m_recordFile);
        stream << static_cast<quint32>(m_recordCount);
        m_recordFile->close();
        QString path = m_recordFile->fileName();
        delete m_recordFile;
        m_recordFile = nullptr;

        emit recordingStopped(path, m_recordCount, duration);
    }
}

void DataLogger::pauseRecording()
{
    if (!m_recording || m_paused) return;
    m_paused = true;
    m_pauseStartTime = m_recordTimer.elapsed();
}

void DataLogger::resumeRecording()
{
    if (!m_recording || !m_paused) return;
    m_paused = false;
    // 累计暂停时长
    m_pauseOffset += (m_recordTimer.elapsed() - m_pauseStartTime);
}

bool DataLogger::isRecording() const
{
    return m_recording;
}

bool DataLogger::isPaused() const
{
    return m_paused;
}

void DataLogger::logData(const QByteArray& data, Direction dir)
{
    if (!m_recording || m_paused || !m_recordFile) return;

    quint64 timestamp = static_cast<quint64>(m_recordTimer.elapsed() - m_pauseOffset);
    writeRecord(timestamp, dir, data);
    m_recordCount++;
}

int DataLogger::recordCount() const
{
    return m_recordCount;
}

qint64 DataLogger::recordingDuration() const
{
    if (!m_recording) return 0;
    return m_recordTimer.elapsed() - m_pauseOffset;
}

// ---- 回放控制 ----

bool DataLogger::startPlayback(const QString& filePath)
{
    if (m_playing) stopPlayback();

    m_playbackFile = new QFile(filePath, this);
    if (!m_playbackFile->open(QIODevice::ReadOnly)) {
        emit error(tr("Cannot open log file: %1").arg(filePath));
        delete m_playbackFile;
        m_playbackFile = nullptr;
        return false;
    }

    // 验证文件头
    QByteArray magic = m_playbackFile->read(3);
    if (magic != kMagic) {
        emit error(tr("Invalid log file format"));
        m_playbackFile->close();
        delete m_playbackFile;
        m_playbackFile = nullptr;
        return false;
    }

    // 使用QDataStream读取version和count，确保字节序与writeHeader()一致(BigEndian)
    QDataStream headerStream(m_playbackFile);
    headerStream.setByteOrder(QDataStream::BigEndian);

    quint8 version = 0;
    headerStream >> version;
    if (version != kVersion) {
        emit error(tr("Unsupported log version: %1").arg(version));
        m_playbackFile->close();
        delete m_playbackFile;
        m_playbackFile = nullptr;
        return false;
    }

    // 读取record count（BigEndian，与writeHeader()/stopRecording()写入格式一致）
    quint32 count = 0;
    headerStream >> count;
    m_totalRecords = static_cast<int>(count);

    if (m_totalRecords == 0) {
        emit error(tr("Log file is empty"));
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

    // 读取第一条记录的时间戳作为基准
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

void DataLogger::pausePlayback()
{
    if (!m_playing || m_playbackPaused) return;
    m_playbackPaused = true;
    m_playbackTimer->stop();
    // 记录暂停时已经过的时间(考虑速度)，累加到baseTime
    m_playbackBaseTime += static_cast<qint64>(m_playbackElapsed.elapsed() * m_playbackSpeed);
}

void DataLogger::resumePlayback()
{
    if (!m_playing || !m_playbackPaused) return;
    m_playbackPaused = false;
    // m_playbackBaseTime已在pausePlayback()中累加了已播放偏移
    // 重启计时器后elapsed()从0开始，currentTime = baseTime + 0 = 正确的恢复点
    m_playbackElapsed.restart();
    m_playbackTimer->start();
}

void DataLogger::setPlaybackSpeed(qreal speed)
{
    qreal newSpeed = qBound(0.1, speed, 100.0);

    if (m_playing && !m_playbackPaused) {
        // 变速时先将当前已播放时间(按旧速度)累加到baseTime
        m_playbackBaseTime += static_cast<qint64>(m_playbackElapsed.elapsed() * m_playbackSpeed);
        m_playbackElapsed.restart();
    }

    m_playbackSpeed = newSpeed;
}

bool DataLogger::isPlaying() const
{
    return m_playing;
}

// ---- 内部方法 ----

void DataLogger::writeHeader()
{
    if (!m_recordFile) return;

    QDataStream stream(m_recordFile);
    stream.setByteOrder(QDataStream::BigEndian);

    // Magic
    stream.writeRawData(kMagic, 3);
    // Version
    stream << kVersion;
    // Record count (placeholder, updated on close)
    stream << static_cast<quint32>(0);
}

void DataLogger::writeRecord(quint64 timestamp, Direction dir, const QByteArray& data)
{
    if (!m_recordFile) return;

    QDataStream stream(m_recordFile);
    stream.setByteOrder(QDataStream::BigEndian);

    // Timestamp (8 bytes)
    stream << timestamp;
    // Direction (1 byte)
    stream << static_cast<quint8>(dir);
    // Length (4 bytes)
    stream << static_cast<quint32>(data.size());
    // Data
    stream.writeRawData(data.constData(), data.size());
}

bool DataLogger::readNextRecord(RecordHeader& header, QByteArray& data)
{
    if (!m_playbackFile || m_playbackFile->atEnd()) return false;

    QDataStream stream(m_playbackFile);
    stream.setByteOrder(QDataStream::BigEndian);

    // Timestamp
    stream >> header.timestamp;
    if (stream.status() != QDataStream::Ok) return false;

    // Direction
    stream >> header.direction;
    if (stream.status() != QDataStream::Ok) return false;

    // Length
    quint32 length = 0;
    stream >> length;
    if (stream.status() != QDataStream::Ok) return false;

    // Data
    // 防御性校验: 拒绝异常大的记录长度（上限1MB），防止恶意/损坏的 .edl 文件导致崩溃
    if (length > 1024 * 1024) {
        qWarning() << "DataLogger: record too large:" << length;
        return false;
    }
    data.resize(static_cast<int>(length));
    if (stream.readRawData(data.data(), static_cast<int>(length)) != static_cast<int>(length)) {
        return false;
    }

    return true;
}

void DataLogger::onPlaybackTick()
{
    if (!m_playing || m_playbackPaused) return;

    // 计算回放时间进度(考虑速度)
    // currentTime = m_playbackOffset(基准) + m_playbackBaseTime(暂停/变速累积) + elapsed * speed
    qint64 elapsed = static_cast<qint64>(m_playbackElapsed.elapsed() * m_playbackSpeed);
    qint64 currentTime = m_playbackOffset + m_playbackBaseTime + elapsed;

    // 发送所有到期记录
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
            // 文件结束
            stopPlayback();
            return;
        }
    }
}

// ---- 跳转定位(Seek) ----

qint64 DataLogger::scanToTimestamp(qint64 targetTimestamp)
{
    if (!m_playbackFile) return -1;

    // 回到数据区起始位置（跳过文件头 kHeaderSize 字节）
    m_playbackFile->seek(kHeaderSize);

    qint64 foundTimestamp = -1;
    qint64 lastValidPos = kHeaderSize;
    int recordsFound = 0;

    // 逐条扫描记录，找到 timestamp <= targetTimestamp 的最后一条
    RecordHeader hdr;
    QByteArray data;
    while (readNextRecord(hdr, data)) {
        qint64 recordTs = static_cast<qint64>(hdr.timestamp);
        if (recordTs > targetTimestamp) {
            // 已超过目标时间戳，上条记录就是最近的
            break;
        }
        foundTimestamp = recordTs;
        lastValidPos = m_playbackFile->pos();
        recordsFound++;
    }

    if (foundTimestamp < 0) {
        // 所有记录时间戳都 > targetTimestamp，定位到文件开头（第一条记录之前）
        m_playbackFile->seek(kHeaderSize);
        m_playedRecords = 0;
        m_nextRecordTime = 0;
        // 重新读取第一条记录时间戳
        if (readNextRecord(hdr, data)) {
            m_nextRecordTime = static_cast<qint64>(hdr.timestamp);
            // 重新回到第一条记录之前，等待 playbackTick 正常播放
            m_playbackFile->seek(kHeaderSize);
        }
        return 0;
    }

    // 文件指针定位到最后一条 <= targetTimestamp 的记录之后
    m_playbackFile->seek(lastValidPos);
    m_playedRecords = recordsFound;

    // 读取下一条记录的时间戳
    if (readNextRecord(hdr, data)) {
        m_nextRecordTime = static_cast<qint64>(hdr.timestamp);
        // 回退到这条记录之前（下次 playbackTick 会再次读取它）
        m_playbackFile->seek(lastValidPos);
    } else {
        // 已到文件末尾
        m_nextRecordTime = std::numeric_limits<qint64>::max();
    }

    return foundTimestamp;
}

bool DataLogger::seekToTimestamp(qint64 timestamp)
{
    QMutexLocker locker(&m_mutex);

    // seek 仅在播放模式下有效，录制模式返回 false
    if (!m_playing) {
        return false;
    }

    // 时间戳有效性检查
    if (timestamp < 0) {
        emit error(tr("Invalid seek timestamp: %1").arg(timestamp));
        return false;
    }

    // 暂停播放定时器，防止在 seek 过程中 playbackTick 干扰
    bool wasTimerRunning = m_playbackTimer->isActive();
    if (wasTimerRunning) {
        m_playbackTimer->stop();
    }

    // 扫描文件，定位到目标时间戳最近的记录
    qint64 actualTimestamp = scanToTimestamp(timestamp);
    if (actualTimestamp < 0) {
        emit error(tr("Failed to seek to timestamp: %1").arg(timestamp));
        if (wasTimerRunning) m_playbackTimer->start();
        return false;
    }

    // 更新播放偏移：将 playbackOffset 设为 seek 目标位置，
    // 重置 baseTime 和 elapsed，使 currentTime = offset + 0 = offset
    m_playbackOffset = actualTimestamp;
    m_playbackBaseTime = 0;
    m_playbackElapsed.restart();

    // 恢复播放定时器
    if (wasTimerRunning) {
        m_playbackTimer->start();
    }

    // 更新进度
    if (m_totalRecords > 0) {
        qreal progress = static_cast<qreal>(m_playedRecords) / m_totalRecords;
        emit playbackProgress(progress);
    }

    emit seekCompleted(actualTimestamp);
    return true;
}

bool DataLogger::seekToBookmark(int index)
{
    // 验证书签索引有效性
    if (index < 0 || index >= m_bookmarks.size()) {
        return false;
    }

    // 书签时间戳已与录制文件使用相同时间参考系（距录制开始的相对偏移量），
    // 因此可直接传递给 seekToTimestamp() 进行定位
    return seekToTimestamp(m_bookmarks[index].timestamp);
}

// ---- 书签管理 ----

void DataLogger::addBookmark(const QString& label, const QString& streamId)
{
    qint64 ts;
    if (m_recording) {
        // 录制中: 使用距录制开始的相对偏移量(ms)，与.edl文件时间戳一致
        // 这样 seekToBookmark() 可以直接定位到正确的文件位置
        ts = m_recordTimer.elapsed() - m_pauseOffset;
    } else {
        // 非录制状态: 使用系统纪元时间（此时seek无意义，但保持数据完整性）
        ts = QDateTime::currentDateTime().toMSecsSinceEpoch();
    }
    m_bookmarks.append(DataBookmark(ts, label, streamId));
    std::sort(m_bookmarks.begin(), m_bookmarks.end());
    emit bookmarksChanged();
}

QVector<DataBookmark> DataLogger::bookmarks() const
{
    return m_bookmarks;
}

void DataLogger::removeBookmark(int index)
{
    if (index < 0 || index >= m_bookmarks.size()) return;
    m_bookmarks.removeAt(index);
    emit bookmarksChanged();
}

void DataLogger::clearBookmarks()
{
    if (m_bookmarks.isEmpty()) return;
    m_bookmarks.clear();
    emit bookmarksChanged();
}
