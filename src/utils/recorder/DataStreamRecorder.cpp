/**
 * @file DataStreamRecorder.cpp
 * @brief 数据流录制器实现 -- 微秒精度双向数据流捕获/标记/触发/导出
 *
 * 内存缓冲模式: 数据保存在 QList 中，支持时间范围/方向/模式搜索。
 * 文件流式模式: 数据直接写入磁盘，内存占用恒定。
 * 触发器: 检测到匹配字节后自动 start/stop。
 * 导出: JSON / CSV / Binary 三种格式。
 */

#include "utils/recorder/DataStreamRecorder.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>
#include <QTextStream>

// ============================================================
// 构造 / 析构
// ============================================================

/** @brief 构造数据流录制器，初始化内部状态 @param parent 父对象 */
DataStreamRecorder::DataStreamRecorder(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("DataStreamRecorder"));
}

/** @brief 析构时若仍在录制则自动停止 */
DataStreamRecorder::~DataStreamRecorder()
{
    if (m_recording) {
        stop();
    }
}

// ============================================================
// 录制控制
// ============================================================

/** @brief 开始内存缓冲模式录制 @param maxBufferSize 最大缓冲条目数, 0=无限制 */
void DataStreamRecorder::start(quint64 maxBufferSize)
{
    if (m_recording) return;

    m_streamingMode = false;
    m_maxBufferSize = maxBufferSize;

    /* 清空缓冲区和标记 */
    {
        QMutexLocker locker(&m_bufferMutex);
        m_buffer.clear();
    }
    m_markers.clear();

    /* 启动计时器 */
    m_elapsedTimer.start();
    m_statsTimer.start();
    m_statsWindowBytes = 0;
    m_recording = true;

    emit recordingStarted();
}

/** @brief 开始文件流式模式录制 @param filePath 输出文件路径 */
void DataStreamRecorder::startStreaming(const QString& filePath)
{
    if (m_recording) return;

    m_streamingMode = true;

    /* 清空缓冲区和标记 */
    {
        QMutexLocker locker(&m_bufferMutex);
        m_buffer.clear();
    }
    m_markers.clear();

    /* 打开输出文件 */
    m_streamFile.setFileName(filePath);
    if (!m_streamFile.open(QIODevice::WriteOnly)) {
        return;
    }

    /* 启动计时器 */
    m_elapsedTimer.start();
    m_statsTimer.start();
    m_statsWindowBytes = 0;
    m_recording = true;

    emit recordingStarted();
}

/** @brief 停止录制，刷新并关闭文件(流式模式)，更新统计 */
void DataStreamRecorder::stop()
{
    if (!m_recording) return;

    m_recording = false;

    /* 计算录制时长 */
    double durationMs = static_cast<double>(m_elapsedTimer.elapsed());
    m_stats.recordingDurationMs = durationMs;

    /* 更新平均记录大小 */
    if (m_stats.totalRecords > 0) {
        m_stats.avgRecordSize = m_stats.totalBytesRecorded / m_stats.totalRecords;
    }

    /* 流式模式: 关闭文件 */
    if (m_streamingMode && m_streamFile.isOpen()) {
        m_streamFile.flush();
        m_streamFile.close();
    }

    emit recordingStopped();
}

/** @brief 查询是否正在录制 @return true=录制中 */
bool DataStreamRecorder::isRecording() const
{
    return m_recording;
}

// ============================================================
// 数据记录
// ============================================================

/** @brief 记录一帧数据，带微秒精度时间戳 @param data 原始字节 @param direction 数据方向 */
void DataStreamRecorder::recordData(const QByteArray& data, Direction direction)
{
    if (!m_recording || data.isEmpty()) return;

    /* 获取微秒时间戳 */
    quint64 timestampUs = static_cast<quint64>(m_elapsedTimer.nsecsElapsed() / 1000);

    /* 检查触发规则(录制前检查，触发器可改变录制状态) */
    if (m_triggerActive) {
        checkTrigger(data);
    }

    /* 如果触发器刚刚停止了录制，直接返回 */
    if (!m_recording) return;

    /* 构建记录条目 */
    RecordEntry entry;
    entry.timestampUs = timestampUs;
    entry.direction = direction;
    entry.data = data;

    if (m_streamingMode) {
        /* 流式模式: 写入文件，格式为 Binary 紧凑格式 */
        /* [8字节时间戳][1字节方向][4字节数据长度][N字节数据] */
        m_streamFile.write(reinterpret_cast<const char*>(&timestampUs), sizeof(quint64));
        char dir = (direction == Direction::RX) ? 0x01 : 0x00;
        m_streamFile.write(&dir, 1);
        quint32 len = static_cast<quint32>(data.size());
        m_streamFile.write(reinterpret_cast<const char*>(&len), sizeof(quint32));
        m_streamFile.write(data);
    } else {
        /* 缓冲模式: 追加到内存 */
        QMutexLocker locker(&m_bufferMutex);
        m_buffer.append(entry);

        /* 检查缓冲区限制 */
        if (m_maxBufferSize > 0 && static_cast<quint64>(m_buffer.size()) > m_maxBufferSize) {
            m_buffer.removeFirst();
        }

        locker.unlock();
        emitBufferUsage();
    }

    /* 更新统计 */
    updateLiveStats(data.size());

    /* 方向统计 */
    if (direction == Direction::RX) {
        ++m_stats.totalRxRecords;
    } else {
        ++m_stats.totalTxRecords;
    }
    m_stats.totalBytesRecorded += static_cast<quint64>(data.size());
    ++m_stats.totalRecords;

    /* 更新平均记录大小 */
    if (m_stats.totalRecords > 0) {
        m_stats.avgRecordSize = m_stats.totalBytesRecorded / m_stats.totalRecords;
    }

    emit recordAdded(timestampUs);
}

// ============================================================
// 标记/书签
// ============================================================

/** @brief 在当前录制位置插入命名标记 @param name 标记名称 */
void DataStreamRecorder::insertMarker(const QString& name)
{
    if (!m_recording) return;

    Marker marker;
    marker.timestampUs = static_cast<quint64>(m_elapsedTimer.nsecsElapsed() / 1000);
    marker.name = name;

    m_markers.append(marker);
    ++m_stats.totalMarkers;

    emit markerInserted(name);
}

/** @brief 获取所有标记 @return 标记列表 */
QList<DataStreamRecorder::Marker> DataStreamRecorder::markers() const
{
    return m_markers;
}

// ============================================================
// 查询/提取
// ============================================================

/** @brief 获取时间范围内的记录 @param startUs 起始微秒(含) @param endUs 结束微秒(含) */
QList<DataStreamRecorder::RecordEntry> DataStreamRecorder::getRecordsByTimeRange(
    quint64 startUs, quint64 endUs) const
{
    QMutexLocker locker(&m_bufferMutex);
    QList<RecordEntry> result;
    result.reserve(m_buffer.size() / 4); /* 预估 25% 命中率 */

    for (const auto& entry : m_buffer) {
        if (entry.timestampUs >= startUs && entry.timestampUs <= endUs) {
            result.append(entry);
        }
    }
    return result;
}

/** @brief 获取指定方向的记录 @param direction 数据方向 */
QList<DataStreamRecorder::RecordEntry> DataStreamRecorder::getRecordsByDirection(
    Direction direction) const
{
    QMutexLocker locker(&m_bufferMutex);
    QList<RecordEntry> result;
    result.reserve(m_buffer.size() / 2); /* 预估 50% 命中率 */

    for (const auto& entry : m_buffer) {
        if (entry.direction == direction) {
            result.append(entry);
        }
    }
    return result;
}

/** @brief 获取所有内存缓冲区中的记录 @return 记录列表的副本 */
QList<DataStreamRecorder::RecordEntry> DataStreamRecorder::allRecords() const
{
    QMutexLocker locker(&m_bufferMutex);
    return m_buffer;
}

/** @brief 获取当前缓冲区中的记录条数 */
quint64 DataStreamRecorder::recordCount() const
{
    QMutexLocker locker(&m_bufferMutex);
    return static_cast<quint64>(m_buffer.size());
}

// ============================================================
// 搜索
// ============================================================

/** @brief 搜索包含指定字节模式的记录 @param pattern 要搜索的字节模式 */
QList<DataStreamRecorder::RecordEntry> DataStreamRecorder::search(const QByteArray& pattern) const
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

/** @brief 搜索包含指定文本(正则)的记录 @param regex 正则表达式 */
QList<DataStreamRecorder::RecordEntry> DataStreamRecorder::searchByText(
    const QRegularExpression& regex) const
{
    QMutexLocker locker(&m_bufferMutex);
    QList<RecordEntry> result;

    if (!regex.isValid()) return result;

    for (const auto& entry : m_buffer) {
        QString text = QString::fromUtf8(entry.data);
        if (regex.match(text).hasMatch()) {
            result.append(entry);
        }
    }
    return result;
}

// ============================================================
// 触发器
// ============================================================

/** @brief 设置触发规则(自动录制) @param rule 触发规则 */
void DataStreamRecorder::setTriggerRule(const TriggerRule& rule)
{
    m_triggerRule = rule;
    m_triggerActive = !rule.pattern.isEmpty();
}

/** @brief 清除触发规则 */
void DataStreamRecorder::clearTriggerRule()
{
    m_triggerActive = false;
    m_triggerRule = TriggerRule{};
}

/** @brief 检查触发规则是否匹配 @param data 待检查的数据 */
void DataStreamRecorder::checkTrigger(const QByteArray& data)
{
    if (!m_triggerActive) return;

    if (data.contains(m_triggerRule.pattern)) {
        if (m_triggerRule.startOnMatch) {
            /* 匹配后自动开始 — 此处仅记录，实际 start 由外部调用 */
            /* 未来可在此处添加自动 start 逻辑 */
        } else {
            /* 匹配后自动停止录制 */
            stop();
        }
    }
}

// ============================================================
// 导出
// ============================================================

/** @brief 将缓冲区记录导出到文件 @param filePath 目标路径 @param format 导出格式 */
bool DataStreamRecorder::exportToFile(const QString& filePath, ExportFormat format) const
{
    switch (format) {
    case ExportFormat::Json:
        return exportJson(filePath);
    case ExportFormat::Csv:
        return exportCsv(filePath);
    case ExportFormat::Binary:
        return exportBinary(filePath);
    }
    return false;
}

/** @brief JSON格式导出: 每条记录一个JSON对象，整体为数组 */
bool DataStreamRecorder::exportJson(const QString& filePath) const
{
    QMutexLocker locker(&m_bufferMutex);

    QJsonArray recordsArray;
    for (const auto& entry : m_buffer) {
        QJsonObject obj;
        obj[QStringLiteral("timestamp_us")] = static_cast<qint64>(entry.timestampUs);
        obj[QStringLiteral("direction")] = (entry.direction == Direction::RX)
            ? QStringLiteral("RX") : QStringLiteral("TX");

        /* 将数据转为HEX字符串 */
        QStringList hexBytes;
        hexBytes.reserve(entry.data.size());
        for (int i = 0; i < entry.data.size(); ++i) {
            hexBytes.append(QStringLiteral("%1")
                .arg(static_cast<unsigned char>(entry.data.at(i)), 2, 16, QChar('0')));
        }
        obj[QStringLiteral("data_hex")] = hexBytes.join(' ');
        obj[QStringLiteral("size")] = entry.data.size();

        recordsArray.append(obj);
    }

    /* 标记也导出 */
    QJsonArray markersArray;
    for (const auto& marker : m_markers) {
        QJsonObject obj;
        obj[QStringLiteral("timestamp_us")] = static_cast<qint64>(marker.timestampUs);
        obj[QStringLiteral("name")] = marker.name;
        markersArray.append(obj);
    }

    QJsonObject root;
    root[QStringLiteral("records")] = recordsArray;
    root[QStringLiteral("markers")] = markersArray;
    root[QStringLiteral("export_time")] = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);

    QJsonDocument doc(root);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

/** @brief CSV格式导出: timestamp_us,direction,size,hex_data */
bool DataStreamRecorder::exportCsv(const QString& filePath) const
{
    QMutexLocker locker(&m_bufferMutex);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream stream(&file);

    /* 文件头 */
    stream << QStringLiteral("# EmbedDebug DataStreamRecorder Export\n");
    stream << QStringLiteral("# Created: %1\n")
        .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs));
    stream << QStringLiteral("timestamp_us,direction,size,hex_data\n");

    /* 数据行 */
    for (const auto& entry : m_buffer) {
        stream << entry.timestampUs << ','
               << (entry.direction == Direction::RX ? "RX" : "TX") << ','
               << entry.data.size() << ',';

        /* HEX数据 */
        for (int i = 0; i < entry.data.size(); ++i) {
            if (i > 0) stream << ' ';
            stream << QStringLiteral("%1")
                .arg(static_cast<unsigned char>(entry.data.at(i)), 2, 16, QChar('0'));
        }
        stream << '\n';
    }

    /* 标记行 */
    stream << QStringLiteral("\n# Markers\n");
    for (const auto& marker : m_markers) {
        stream << QStringLiteral("# MARKER: %1 @ %2 us\n")
            .arg(marker.name)
            .arg(marker.timestampUs);
    }

    stream.flush();
    file.close();
    return true;
}

/** @brief 二进制格式导出: 文件头+紧凑记录 */
bool DataStreamRecorder::exportBinary(const QString& filePath) const
{
    QMutexLocker locker(&m_bufferMutex);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;

    /* 文件头: 魔数(4字节) + 版本(2字节) + 记录数(8字节) + 标记数(8字节) */
    const char magic[] = "EDSR"; /* EmbedDebug Stream Recorder */
    file.write(magic, 4);

    quint16 version = 1;
    file.write(reinterpret_cast<const char*>(&version), sizeof(quint16));

    quint64 recordCount = static_cast<quint64>(m_buffer.size());
    file.write(reinterpret_cast<const char*>(&recordCount), sizeof(quint64));

    quint64 markerCount = static_cast<quint64>(m_markers.size());
    file.write(reinterpret_cast<const char*>(&markerCount), sizeof(quint64));

    /* 记录数据: 每条 [8字节时间戳][1字节方向][4字节长度][N字节数据] */
    for (const auto& entry : m_buffer) {
        file.write(reinterpret_cast<const char*>(&entry.timestampUs), sizeof(quint64));

        char dir = (entry.direction == Direction::RX) ? 0x01 : 0x00;
        file.write(&dir, 1);

        quint32 len = static_cast<quint32>(entry.data.size());
        file.write(reinterpret_cast<const char*>(&len), sizeof(quint32));
        file.write(entry.data);
    }

    /* 标记数据: 每条 [8字节时间戳][2字节名称长度][N字节名称(UTF-8)] */
    for (const auto& marker : m_markers) {
        file.write(reinterpret_cast<const char*>(&marker.timestampUs), sizeof(quint64));

        QByteArray nameUtf8 = marker.name.toUtf8();
        quint16 nameLen = static_cast<quint16>(nameUtf8.size());
        file.write(reinterpret_cast<const char*>(&nameLen), sizeof(quint16));
        file.write(nameUtf8);
    }

    file.flush();
    file.close();
    return true;
}

// ============================================================
// 统计
// ============================================================

/** @brief 获取统计快照 @return Stats 结构体 */
DataStreamRecorder::Stats DataStreamRecorder::stats() const
{
    Stats s = m_stats;

    /* 实时更新录制时长 */
    if (m_recording) {
        s.recordingDurationMs = static_cast<double>(m_elapsedTimer.elapsed());
    }

    return s;
}

/** @brief 重置所有统计计数器 */
void DataStreamRecorder::resetStatistics()
{
    m_stats = Stats{};
    m_statsWindowBytes = 0;
}

// ============================================================
// 内部辅助方法
// ============================================================

/** @brief 更新实时统计(峰值吞吐量) @param dataSize 本条记录的字节数 */
void DataStreamRecorder::updateLiveStats(int dataSize)
{
    m_statsWindowBytes += static_cast<quint64>(dataSize);

    /* 每 500ms 计算一次吞吐量 */
    qint64 elapsedMs = m_statsTimer.elapsed();
    if (elapsedMs >= 500) {
        double sec = static_cast<double>(elapsedMs) / 1000.0;
        double bytesPerSec = static_cast<double>(m_statsWindowBytes) / sec;

        if (bytesPerSec > m_stats.peakBytesPerSec) {
            m_stats.peakBytesPerSec = bytesPerSec;
        }

        /* 重置窗口 */
        m_statsWindowBytes = 0;
        m_statsTimer.restart();
    }
}

/** @brief 计算并发射缓冲区使用率 */
void DataStreamRecorder::emitBufferUsage()
{
    if (m_maxBufferSize == 0) return; /* 无限制模式不报告使用率 */

    QMutexLocker locker(&m_bufferMutex);
    double percentage = (static_cast<double>(m_buffer.size()) /
                         static_cast<double>(m_maxBufferSize)) * 100.0;
    locker.unlock();

    emit bufferUsageChanged(percentage);
}
