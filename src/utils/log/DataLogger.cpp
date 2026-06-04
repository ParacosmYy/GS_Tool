/**
 * @file DataLogger.cpp
 * @brief 数据录制管理器实现 — 录制启停、暂停/恢复、文件I/O
 *
 * 自定义EDL二进制格式的日志录制（含时间戳、方向、数据）。
 * 回放控制/跳转/书签见 DataLoggerPlayback.cpp。
 */

#include "utils/log/DataLogger.h"
#include "shared/TimerConstants.h"
#include <QDataStream>
#include <QFileInfo>
#include <QDateTime>
#include <QMutexLocker>

/** @brief 构造数据录制器(创建回放定时器) @param parent 父对象 */
DataLogger::DataLogger(QObject* parent)
    : QObject(parent)
    , m_playbackTimer(new QTimer(this))
{
    m_playbackTimer->setSingleShot(false);
    m_playbackTimer->setInterval(Timers::kPlaybackPrecisionMs);
    connect(m_playbackTimer, &QTimer::timeout,
            this, &DataLogger::onPlaybackTick);
}

/** @brief 析构数据录制器，静默停止录制和回放(不发射信号) */
DataLogger::~DataLogger()
{
    // 析构时静默停止: 仅释放资源，不发射信号(避免析构期间回调访问半销毁对象)
    if (m_recording) {
        m_recording = false;
        if (m_recordFile) {
            m_recordFile->close();
            delete m_recordFile;
            m_recordFile = nullptr;
        }
    }
    if (m_playing) {
        m_playing = false;
        if (m_playbackTimer) m_playbackTimer->stop();
        if (m_playbackFile) {
            m_playbackFile->close();
            delete m_playbackFile;
            m_playbackFile = nullptr;
        }
    }
}

// ---- 录制控制 ----

/** @brief 开始数据录制(若已在录制则先停止) @param filePath 录制文件路径 @return true=录制启动成功 */
bool DataLogger::startRecording(const QString& filePath)
{
    if (m_recording) stopRecording();

    m_recordFile = new QFile(filePath, this);
    if (!m_recordFile->open(QIODevice::WriteOnly)) {
        ++m_totalErrors;
        emit error(tr("无法创建日志文件: %1").arg(filePath));
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
    ++m_totalRecordStarts;  ///< 累计录制启动次数
    emit recordingStarted();
    return true;
}

/** @brief 停止录制，回写header中的recordCount，发射recordingStopped信号 */
void DataLogger::stopRecording()
{
    if (!m_recording) return;
    m_recording = false;
    qint64 duration = m_recordTimer.elapsed() - m_pauseOffset;

    if (m_recordFile) {
        if (m_recordFile->seek(4)) {
            QDataStream stream(m_recordFile);
            stream << static_cast<quint32>(m_recordCount);
        }
        m_recordFile->close();
        QString path = m_recordFile->fileName();
        delete m_recordFile;
        m_recordFile = nullptr;
        emit recordingStopped(path, m_recordCount, duration);
    }
}

/** @brief 暂停录制，记录暂停起始时间并递增暂停计数 */
void DataLogger::pauseRecording()
{
    if (!m_recording || m_paused) return;
    m_paused = true;
    ++m_totalRecordingPauses;  ///< 累计录制暂停次数
    m_pauseStartTime = m_recordTimer.elapsed();
}

/** @brief 恢复录制，累加暂停时长到m_pauseOffset并递增恢复计数 */
void DataLogger::resumeRecording()
{
    if (!m_recording || !m_paused) return;
    m_paused = false;
    ++m_totalRecordingResumes;  ///< 累计录制恢复次数
    m_pauseOffset += (m_recordTimer.elapsed() - m_pauseStartTime);
}

/** @brief 查询是否正在录制 @return true=录制中 */
bool DataLogger::isRecording() const { return m_recording; }

/** @brief 查询录制是否暂停 @return true=已暂停 */
bool DataLogger::isPaused() const { return m_paused; }

/** @brief 记录一条数据到日志文件 @param data 原始字节数据 @param dir 数据方向(RX/TX) */
void DataLogger::logData(const QByteArray& data, Direction dir)
{
    if (!m_recording || m_paused || !m_recordFile) return;

    quint64 timestamp = static_cast<quint64>(m_recordTimer.elapsed() - m_pauseOffset);
    writeRecord(timestamp, dir, data);
    m_recordCount++;
    ++m_totalLogsWritten;
    ++m_totalRecords;
    m_totalBytesRecorded += static_cast<quint64>(data.size());
}

/** @brief 获取当前录制会话的记录条数 @return 已记录的数据条数 */
int DataLogger::recordCount() const { return m_recordCount; }

/** @brief 获取当前录制会话的持续时间(毫秒，扣除暂停时间) */
qint64 DataLogger::recordingDuration() const
{
    if (!m_recording) return 0;
    return m_recordTimer.elapsed() - m_pauseOffset;
}

