/**
 * @file DataLogger.cpp
 * @brief 数据录制/回放管理器实现
 *
 * 实现自定义二进制格式的日志录制（含时间戳、方向、数据），
 * 以及基于时间戳的随机访问回放。支持录制启停、回放控制和回放速率调整。
 */
#include "utils/log/DataLogger.h"
#include "core/theme/Constants.h"
#include <QDataStream>
#include <QFileInfo>
#include <QDateTime>
#include <QMutexLocker>
#include <algorithm>

/** @brief 构造数据录制/回放器(创建回放定时器) @param parent 父对象 */
DataLogger::DataLogger(QObject* parent)
    : QObject(parent)
    , m_playbackTimer(new QTimer(this))
{
    m_playbackTimer->setSingleShot(false);
    m_playbackTimer->setInterval(Timers::kPlaybackPrecisionMs); // 1ms精度
    connect(m_playbackTimer, &QTimer::timeout,
            this, &DataLogger::onPlaybackTick);
}

DataLogger::~DataLogger()
{
    // 析构时静默停止: 仅释放资源，不发射信号(避免析构期间回调访问半销毁对象)
    // 正常停止由RecordingController调用stopRecording()/stopPlayback()完成
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
    if (m_recording) {
        stopRecording();
    }

    m_recordFile = new QFile(filePath, this);
    if (!m_recordFile->open(QIODevice::WriteOnly)) {
        ++m_totalErrors;  // 文件创建失败
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
        // 回写header中的record count
        if (m_recordFile->seek(4)) { // 跳过magic(3) + version(1)
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
/** @brief 暂停录制，记录暂停起始时间 */
void DataLogger::pauseRecording()
{
    if (!m_recording || m_paused) return;
    m_paused = true;
    m_pauseStartTime = m_recordTimer.elapsed();
}

/** @brief 恢复录制，累加暂停时长到m_pauseOffset */
void DataLogger::resumeRecording()
{
    if (!m_recording || !m_paused) return;
    m_paused = false;
    // 累计暂停时长
    m_pauseOffset += (m_recordTimer.elapsed() - m_pauseStartTime);
}

/** @brief 查询是否正在录制 @return true=录制中 */
bool DataLogger::isRecording() const
{
    return m_recording;
}
/** @brief 查询录制是否暂停 @return true=已暂停 */
bool DataLogger::isPaused() const
{
    return m_paused;
}

/** @brief 记录一条数据到日志文件 @param data 原始字节数据 @param dir 数据方向(RX/TX) */
void DataLogger::logData(const QByteArray& data, Direction dir)
{
    if (!m_recording || m_paused || !m_recordFile) return;

    quint64 timestamp = static_cast<quint64>(m_recordTimer.elapsed() - m_pauseOffset);
    writeRecord(timestamp, dir, data);
    m_recordCount++;
    ++m_totalLogsWritten;  // 累计写入计数
    ++m_totalRecords;      // 累计录制记录计数
    m_totalBytesRecorded += static_cast<quint64>(data.size());  // 累计录制字节计数
}
/** @brief 获取当前录制会话已记录的数据条数 @return 已记录条数 */
int DataLogger::recordCount() const { return m_recordCount; }
/** @brief 获取当前录制会话的持续时间(毫秒，扣除暂停时间) @return 持续时间(ms) */
qint64 DataLogger::recordingDuration() const
{
    if (!m_recording) return 0;
    return m_recordTimer.elapsed() - m_pauseOffset;
}

// ---- 回放控制 ----

/** @brief 开始数据回放(打开录制文件→扫描索引→启动定时器) @param filePath 录制文件路径 @return true=回放启动成功 */
bool DataLogger::startPlayback(const QString& filePath)
{
    if (m_playing) stopPlayback();

    m_playbackFile = new QFile(filePath, this);
    if (!m_playbackFile->open(QIODevice::ReadOnly)) {
        ++m_totalErrors;  // 文件打开失败
        emit error(tr("无法打开日志文件: %1").arg(filePath));
        delete m_playbackFile;
        m_playbackFile = nullptr;
        return false;
    }

    // 验证文件头
    QByteArray magic = m_playbackFile->read(3);
    if (magic != kMagic) {
        ++m_totalErrors;  // 格式验证失败
        emit error(tr("无效的日志文件格式"));
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
        ++m_totalErrors;  // 版本不匹配
        emit error(tr("不支持的日志版本: %1").arg(version));
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
        ++m_totalErrors;  // 空文件
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
    ++m_totalPlaybacks;  // 累计回放启动计数

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
    // 记录暂停时已经过的时间(考虑速度)，累加到baseTime
    m_playbackBaseTime += static_cast<qint64>(m_playbackElapsed.elapsed() * m_playbackSpeed);
}

/** @brief 恢复回放 */
void DataLogger::resumePlayback()
{
    if (!m_playing || !m_playbackPaused) return;
    m_playbackPaused = false;
    // m_playbackBaseTime已在pausePlayback()中累加了已播放偏移
    // 重启计时器后elapsed()从0开始，currentTime = baseTime + 0 = 正确的恢复点
    m_playbackElapsed.restart();
    m_playbackTimer->start();
}
/** @brief 设置回放速率 @param speed 速率倍数(1.0=正常) */
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
/** @brief 查询是否正在回放 @return true=回放中 */
bool DataLogger::isPlaying() const
{
    return m_playing;
}

// ---- 回放定时器 ----

/** @brief 回放定时器回调，按时间戳发射下一条记录 */
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

/** @brief 线性扫描录制文件到目标时间戳(构建时间索引) @param targetTimestamp 目标时间戳(ms) @return 实际定位到的偏移量 */
qint64 DataLogger::scanToTimestamp(qint64 targetTimestamp)
{
    if (!m_playbackFile) return -1;

    // 回到数据区起始位置（跳过文件头 kHeaderSize 字节）
    if (!m_playbackFile->seek(kHeaderSize)) return -1;

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
        if (!m_playbackFile->seek(kHeaderSize)) return -1;
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
    if (!m_playbackFile->seek(lastValidPos)) return -1;
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

/** @brief 跳转到指定时间戳位置(先扫描索引再seek) @param timestamp 目标时间戳(ms) @return true=跳转成功 */
bool DataLogger::seekToTimestamp(qint64 timestamp)
{
    QMutexLocker locker(&m_mutex);

    // seek 仅在播放模式下有效，录制模式返回 false
    if (!m_playing) {
        return false;
    }

    // 时间戳有效性检查
    if (timestamp < 0) {
        emit error(tr("无效的查找时间戳: %1").arg(timestamp));
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
        emit error(tr("查找时间戳失败: %1").arg(timestamp));
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

/** @brief 跳转到指定书签位置 @param index 书签索引 @return true=跳转成功 */
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

/** @brief 添加书签(标记当前录制位置) @param label 书签标签 @param streamId 数据流标识 */
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
    ++m_totalBookmarks;  // 累计书签计数
    std::sort(m_bookmarks.begin(), m_bookmarks.end());
    emit bookmarksChanged();
}

/** @brief 获取所有书签 @return DataBookmark列表 */
QVector<DataBookmark> DataLogger::bookmarks() const
{
    return m_bookmarks;
}
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

void DataLogger::resetStats()
{
    m_totalLogsWritten = 0;
    m_totalBookmarks = 0;
    m_totalRecords = 0;
    m_totalBytesRecorded = 0;
    m_totalPlaybacks = 0;
    m_totalErrors = 0;
}
