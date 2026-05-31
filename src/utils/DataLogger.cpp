#include "utils/DataLogger.h"
#include <QDataStream>
#include <QFileInfo>

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

    quint8 version = 0;
    m_playbackFile->read(reinterpret_cast<char*>(&version), 1);
    if (version != kVersion) {
        emit error(tr("Unsupported log version: %1").arg(version));
        m_playbackFile->close();
        delete m_playbackFile;
        m_playbackFile = nullptr;
        return false;
    }

    // 读取record count
    quint32 count = 0;
    m_playbackFile->read(reinterpret_cast<char*>(&count), 4);
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
    qint64 elapsed = static_cast<qint64>(m_playbackElapsed.elapsed() * m_playbackSpeed);
    qint64 currentTime = m_playbackBaseTime + elapsed;

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
