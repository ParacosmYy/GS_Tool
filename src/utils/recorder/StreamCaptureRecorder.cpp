/**
 * @file StreamCaptureRecorder.cpp
 * @brief 流捕获录制器实现 -- 微秒精度双向数据流捕获/标记/触发
 *
 * 内存缓冲模式: 数据保存在 QList 中，支持时间范围/方向/模式搜索。
 * 文件流式模式: 数据直接写入磁盘，内存占用恒定。
 * 触发器: 检测到匹配字节后自动 start/stop。
 * 导出方法在 StreamCaptureRecorderExport.cpp 中。
 */

#include "utils/recorder/StreamCaptureRecorder.h"

#include <QMutexLocker>

// ============================================================
// 构造 / 析构
// ============================================================

/** @brief 构造流捕获录制器 @param parent 父对象 */
StreamCaptureRecorder::StreamCaptureRecorder(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("StreamCaptureRecorder"));
}

/** @brief 析构时若仍在录制则自动停止 */
StreamCaptureRecorder::~StreamCaptureRecorder()
{
    if (m_recording) {
        stop();
    }
}

// ============================================================
// 录制控制
// ============================================================

/** @brief 开始内存缓冲模式录制 @param maxBufferSize 最大缓冲条目数, 0=无限制 */
void StreamCaptureRecorder::start(quint64 maxBufferSize)
{
    if (m_recording) return;

    m_streamingMode = false;
    m_maxBufferSize = maxBufferSize;

    {
        QMutexLocker locker(&m_bufferMutex);
        m_buffer.clear();
    }
    m_markers.clear();

    m_elapsedTimer.start();
    m_statsTimer.start();
    m_statsWindowBytes = 0;
    m_recording = true;

    emit recordingStarted();
}

/** @brief 开始文件流式模式录制 @param filePath 输出文件路径 */
void StreamCaptureRecorder::startStreaming(const QString& filePath)
{
    if (m_recording) return;

    m_streamingMode = true;

    {
        QMutexLocker locker(&m_bufferMutex);
        m_buffer.clear();
    }
    m_markers.clear();

    m_streamFile.setFileName(filePath);
    if (!m_streamFile.open(QIODevice::WriteOnly)) {
        return;
    }

    m_elapsedTimer.start();
    m_statsTimer.start();
    m_statsWindowBytes = 0;
    m_recording = true;

    emit recordingStarted();
}

/** @brief 停止录制 */
void StreamCaptureRecorder::stop()
{
    if (!m_recording) return;

    m_recording = false;

    m_stats.recordingDurationMs = static_cast<double>(m_elapsedTimer.elapsed());
    if (m_stats.totalRecords > 0) {
        m_stats.avgRecordSize = m_stats.totalBytesRecorded / m_stats.totalRecords;
    }

    if (m_streamingMode && m_streamFile.isOpen()) {
        m_streamFile.flush();
        m_streamFile.close();
    }

    emit recordingStopped();
}

/** @brief 是否正在录制 */
bool StreamCaptureRecorder::isRecording() const
{
    return m_recording;
}

// ============================================================
// 数据记录
// ============================================================

/** @brief 记录一帧数据 @param data 原始字节 @param direction 方向 */
void StreamCaptureRecorder::recordData(const QByteArray& data, Direction direction)
{
    if (!m_recording || data.isEmpty()) return;

    quint64 timestampUs = static_cast<quint64>(m_elapsedTimer.nsecsElapsed() / 1000);

    /* 检查触发器 */
    if (m_triggerActive) {
        checkTrigger(data);
    }
    if (!m_recording) return;

    RecordEntry entry;
    entry.timestampUs = timestampUs;
    entry.direction = direction;
    entry.data = data;

    if (m_streamingMode) {
        /* 流式: [8B时间戳][1B方向][4B长度][NB数据] */
        m_streamFile.write(reinterpret_cast<const char*>(&timestampUs), sizeof(quint64));
        char dir = (direction == Direction::RX) ? 0x01 : 0x00;
        m_streamFile.write(&dir, 1);
        quint32 len = static_cast<quint32>(data.size());
        m_streamFile.write(reinterpret_cast<const char*>(&len), sizeof(quint32));
        m_streamFile.write(data);
    } else {
        QMutexLocker locker(&m_bufferMutex);
        m_buffer.append(entry);
        if (m_maxBufferSize > 0 && static_cast<quint64>(m_buffer.size()) > m_maxBufferSize) {
            m_buffer.removeFirst();
        }
        locker.unlock();
        emitBufferUsage();
    }

    updateLiveStats(data.size());

    if (direction == Direction::RX) {
        ++m_stats.totalRxRecords;
    } else {
        ++m_stats.totalTxRecords;
    }
    m_stats.totalBytesRecorded += static_cast<quint64>(data.size());
    ++m_stats.totalRecords;
    if (m_stats.totalRecords > 0) {
        m_stats.avgRecordSize = m_stats.totalBytesRecorded / m_stats.totalRecords;
    }

    emit recordAdded(timestampUs);
}

// ============================================================
// 标记
// ============================================================

/** @brief 插入命名标记 @param name 标记名称 */
void StreamCaptureRecorder::insertMarker(const QString& name)
{
    if (!m_recording) return;

    Marker marker;
    marker.timestampUs = static_cast<quint64>(m_elapsedTimer.nsecsElapsed() / 1000);
    marker.name = name;
    m_markers.append(marker);
    ++m_stats.totalMarkers;

    emit markerInserted(name);
}

/** @brief 获取所有标记 */
QList<StreamCaptureRecorder::Marker> StreamCaptureRecorder::markers() const
{
    return m_markers;
}

// ============================================================
// 查询/提取
// ============================================================

/** @brief 按时间范围获取记录 [startUs, endUs] */
QList<StreamCaptureRecorder::RecordEntry> StreamCaptureRecorder::getRecordsByTimeRange(
    quint64 startUs, quint64 endUs) const
{
    QMutexLocker locker(&m_bufferMutex);
    QList<RecordEntry> result;
    result.reserve(m_buffer.size() / 4);

    for (const auto& entry : m_buffer) {
        if (entry.timestampUs >= startUs && entry.timestampUs <= endUs) {
            result.append(entry);
        }
    }
    return result;
}

/** @brief 按方向获取记录 */
QList<StreamCaptureRecorder::RecordEntry> StreamCaptureRecorder::getRecordsByDirection(
    Direction direction) const
{
    QMutexLocker locker(&m_bufferMutex);
    QList<RecordEntry> result;
    result.reserve(m_buffer.size() / 2);

    for (const auto& entry : m_buffer) {
        if (entry.direction == direction) {
            result.append(entry);
        }
    }
    return result;
}

/** @brief 获取所有内存记录的副本 */
QList<StreamCaptureRecorder::RecordEntry> StreamCaptureRecorder::allRecords() const
{
    QMutexLocker locker(&m_bufferMutex);
    return m_buffer;
}

/** @brief 缓冲区记录条数 */
quint64 StreamCaptureRecorder::recordCount() const
{
    QMutexLocker locker(&m_bufferMutex);
    return static_cast<quint64>(m_buffer.size());
}

// ============================================================
// 搜索
// ============================================================

/** @brief 搜索包含指定字节模式的记录 */
QList<StreamCaptureRecorder::RecordEntry> StreamCaptureRecorder::search(
    const QByteArray& pattern) const
{
    QMutexLocker locker(&m_bufferMutex);
    QList<RecordEntry> result;
    if (pattern.isEmpty()) return result;

    for (const auto& entry : m_buffer) {
        if (entry.data.contains(pattern)) {
            result.append(entry);
        }
    }
    return result;
}

/** @brief 搜索包含指定正则文本的记录 */
QList<StreamCaptureRecorder::RecordEntry> StreamCaptureRecorder::searchByText(
    const QRegularExpression& regex) const
{
    QMutexLocker locker(&m_bufferMutex);
    QList<RecordEntry> result;
    if (!regex.isValid()) return result;

    for (const auto& entry : m_buffer) {
        if (regex.match(QString::fromUtf8(entry.data)).hasMatch()) {
            result.append(entry);
        }
    }
    return result;
}

// ============================================================
// 触发器
// ============================================================

/** @brief 设置触发规则 */
void StreamCaptureRecorder::setTriggerRule(const TriggerRule& rule)
{
    m_triggerRule = rule;
    m_triggerActive = !rule.pattern.isEmpty();
}

/** @brief 清除触发规则 */
void StreamCaptureRecorder::clearTriggerRule()
{
    m_triggerActive = false;
    m_triggerRule = TriggerRule{};
}

/** @brief 检查触发规则 */
void StreamCaptureRecorder::checkTrigger(const QByteArray& data)
{
    if (!m_triggerActive) return;

    if (data.contains(m_triggerRule.pattern)) {
        if (!m_triggerRule.startOnMatch) {
            stop();
        }
    }
}

// ============================================================
// 统计
// ============================================================

/** @brief 获取统计快照 */
StreamCaptureRecorder::Stats StreamCaptureRecorder::stats() const
{
    Stats s = m_stats;
    if (m_recording) {
        s.recordingDurationMs = static_cast<double>(m_elapsedTimer.elapsed());
    }
    return s;
}

/** @brief 重置所有统计计数器 */
void StreamCaptureRecorder::resetStatistics()
{
    m_stats = Stats{};
    m_statsWindowBytes = 0;
}

// ============================================================
// 内部辅助
// ============================================================

/** @brief 更新实时统计(峰值吞吐量) */
void StreamCaptureRecorder::updateLiveStats(int dataSize)
{
    m_statsWindowBytes += static_cast<quint64>(dataSize);

    qint64 elapsedMs = m_statsTimer.elapsed();
    if (elapsedMs >= 500) {
        double sec = static_cast<double>(elapsedMs) / 1000.0;
        double bytesPerSec = static_cast<double>(m_statsWindowBytes) / sec;
        if (bytesPerSec > m_stats.peakBytesPerSec) {
            m_stats.peakBytesPerSec = bytesPerSec;
        }
        m_statsWindowBytes = 0;
        m_statsTimer.restart();
    }
}

/** @brief 计算并发射缓冲区使用率 */
void StreamCaptureRecorder::emitBufferUsage()
{
    if (m_maxBufferSize == 0) return;

    QMutexLocker locker(&m_bufferMutex);
    double percentage = (static_cast<double>(m_buffer.size()) /
                         static_cast<double>(m_maxBufferSize)) * 100.0;
    locker.unlock();

    emit bufferUsageChanged(percentage);
}
