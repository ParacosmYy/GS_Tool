/**
 * @file DataStreamRecorder.cpp
 * @brief 数据流记录器实现 - 原始数据流捕获/标注/多格式保存
 *
 * 3种格式: Raw(纯二进制) / HexDump(16字节HEX+ASCII) / TimestampedCsv(时间戳CSV)
 * 自动分割: 超过maxFileSizeMB自动切换新文件。标注系统嵌入输出流。
 */

#include "core/recording/DataStreamRecorder.h"

#include <QDateTime>
#include <QFileInfo>

// ============================================================
// 构造 / 析构
// ============================================================

/** @brief 构造数据流记录器，初始化所有统计计数器 @param parent 父对象 */
DataStreamRecorder::DataStreamRecorder(QObject* parent)
    : QObject(parent)
    , m_stream(&m_file)
{
}

/** @brief 析构时若仍在录制则自动停止，确保文件正确关闭 */
DataStreamRecorder::~DataStreamRecorder()
{
    if (m_recording) {
        stop();
    }
}

// ============================================================
// 录制控制
// ============================================================

/** @brief 开始录制 @param config 录制配置 @return true=成功打开文件 */
bool DataStreamRecorder::start(const RecordingConfig& config)
{
    if (m_recording) {
        emit errorOccurred(tr("已在录制中，请先停止当前录制"));
        return false;
    }

    /* 验证配置: 文件路径不能为空 */
    if (config.filePath.isEmpty()) {
        emit errorOccurred(tr("录制文件路径不能为空"));
        return false;
    }

    m_config = config;
    m_fileIndex = 0;
    m_sessionBytes = 0;
    m_sessionPackets = 0;

    /* 重置会话统计 */
    m_recordingStats = RecordingStats{};

    /* 生成第一个文件路径并打开 */
    QString path = generateFilePath();
    m_file.setFileName(path);

    /* 根据格式选择打开模式 */
    QIODevice::OpenMode mode = QIODevice::WriteOnly;
    if (m_config.format != FileFormat::Raw) {
        mode |= QIODevice::Text;        /* 文本格式需要换行转换 */
    }

    if (!m_file.open(mode)) {
        emit errorOccurred(tr("无法打开录制文件: %1 (%2)")
            .arg(path, m_file.errorString()));
        return false;
    }

    /* 写入文件头 */
    writeHeader();

    /* 启动计时器 */
    m_recordingTimer.start();
    m_recording = true;

    /* 更新统计 */
    ++m_recordingStats.totalStarts;
    ++m_stats.totalRecordingSessions;
    ++m_stats.totalFilesCreated;

    emit recordingStarted(path);
    return true;
}

/** @brief 停止录制，刷新关闭文件，更新累计统计 */
void DataStreamRecorder::stop()
{
    if (!m_recording) return;

    m_recording = false;

    /* 计算会话时长和吞吐量 */
    double elapsedMs = static_cast<double>(m_recordingTimer.elapsed());
    double elapsedSec = elapsedMs / 1000.0;
    m_recordingStats.recordingDurationSec = elapsedSec;

    if (elapsedSec > 0.0) {
        m_recordingStats.avgBytesPerSec = static_cast<double>(m_sessionBytes) / elapsedSec;
    }

    /* 更新累计统计 */
    m_stats.totalBytesWritten += static_cast<quint64>(m_sessionBytes);
    m_stats.totalRecordingTimeSec += elapsedSec;

    if (m_recordingStats.avgBytesPerSec > m_stats.peakThroughputBytesPerSec) {
        m_stats.peakThroughputBytesPerSec = m_recordingStats.avgBytesPerSec;
    }

    ++m_recordingStats.totalStops;

    /* 关闭文件 */
    m_file.flush();
    m_file.close();

    emit recordingStopped(m_sessionBytes);
}

/** @brief 查询是否正在录制 @return true=录制中 */
bool DataStreamRecorder::isRecording() const
{
    return m_recording;
}

// ============================================================
// 数据写入
// ============================================================

/** @brief 录制一帧数据 @param data 原始字节 @param isReceived true=RX false=TX */
void DataStreamRecorder::recordData(const QByteArray& data, bool isReceived)
{
    if (!m_recording || data.isEmpty()) return;

    /* 根据格式分发 */
    switch (m_config.format) {
    case FileFormat::Raw:
        writeRaw(data, isReceived);
        break;
    case FileFormat::HexDump:
        writeHexDump(data, isReceived);
        break;
    case FileFormat::TimestampedCsv:
        writeTimestamped(data, isReceived);
        break;
    }

    /* 更新统计 */
    m_sessionBytes += data.size();
    ++m_sessionPackets;
    m_recordingStats.totalBytesRecorded += static_cast<quint64>(data.size());
    ++m_recordingStats.totalPacketsRecorded;
    m_recordingStats.currentFileSize = static_cast<quint64>(m_file.size());

    emit dataRecorded(data.size());

    /* 检查文件大小限制，自动分割 */
    if (m_config.autoSplit && m_config.maxFileSizeMB > 0) {
        qint64 maxBytes = static_cast<qint64>(m_config.maxFileSizeMB) * 1024 * 1024;
        if (m_file.size() >= maxBytes) {
            splitFile();
        }
    }
}

/** @brief 添加文本标注，在输出流中插入注释行 @param text 标注内容 */
void DataStreamRecorder::addAnnotation(const QString& text)
{
    if (!m_recording) return;

    QString line;
    switch (m_config.format) {
    case FileFormat::Raw:
        /* 原始格式不支持内联标注，写入独立的标注标记 */
        line = QStringLiteral("[ANNOTATION] %1").arg(text);
        m_file.write(line.toUtf8());
        m_file.write("\n");
        break;
    case FileFormat::HexDump:
        line = QStringLiteral("; @ANNOTATION %1 -- %2")
            .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs), text);
        m_file.write(line.toUtf8());
        m_file.write("\n");
        break;
    case FileFormat::TimestampedCsv:
        line = QStringLiteral("\"%1\",\"ANNOTATION\",\"0\",\"%2\"")
            .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs), text);
        m_file.write(line.toUtf8());
        m_file.write("\n");
        break;
    }

    ++m_recordingStats.totalAnnotations;
    emit annotationAdded(text);
}

/** @brief 添加位置标记(带时间戳书签)，便于事后快速定位 @param label 标记标签 */
void DataStreamRecorder::addMarker(const QString& label)
{
    if (!m_recording) return;

    QString line;
    switch (m_config.format) {
    case FileFormat::Raw:
        line = QStringLiteral("[MARKER] %1 @ byte %2")
            .arg(label).arg(m_sessionBytes);
        m_file.write(line.toUtf8());
        m_file.write("\n");
        break;
    case FileFormat::HexDump:
        line = QStringLiteral("; @MARKER %1 @ offset 0x%2 @ %3")
            .arg(label)
            .arg(m_sessionBytes, 0, 16)
            .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs));
        m_file.write(line.toUtf8());
        m_file.write("\n");
        break;
    case FileFormat::TimestampedCsv:
        line = QStringLiteral("\"%1\",\"MARKER\",\"0\",\"%2 @ byte %3\"")
            .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs))
            .arg(label)
            .arg(m_sessionBytes);
        m_file.write(line.toUtf8());
        m_file.write("\n");
        break;
    }

    ++m_recordingStats.totalAnnotations;
    emit annotationAdded(label);
}

// ============================================================
// 配置/状态查询
// ============================================================

/** @brief 获取当前录制配置 @return 配置副本 */
DataStreamRecorder::RecordingConfig DataStreamRecorder::config() const
{
    return m_config;
}

/** @brief 获取当前录制会话统计 @return 统计快照 */
DataStreamRecorder::RecordingStats DataStreamRecorder::recordingStats() const
{
    RecordingStats stats = m_recordingStats;
    /* 实时更新时长和吞吐量 */
    if (m_recording) {
        double elapsedMs = static_cast<double>(m_recordingTimer.elapsed());
        stats.recordingDurationSec = elapsedMs / 1000.0;
        if (stats.recordingDurationSec > 0.0) {
            stats.avgBytesPerSec = static_cast<double>(m_sessionBytes) / stats.recordingDurationSec;
        }
    }
    return stats;
}

/** @brief 获取累计全局统计 @return 统计常量引用 */
const DataStreamRecorder::Stats& DataStreamRecorder::stats() const
{
    return m_stats;
}

// ============================================================
// 内部写入方法
// ============================================================

/** @brief 写入文件头(格式标识/创建时间/配置元数据) */
void DataStreamRecorder::writeHeader()
{
    QString now = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);

    switch (m_config.format) {
    case FileFormat::Raw:
        /* 原始二进制: 写入一个轻量头部标识 */
        {
            QByteArray header;
            header.append("[EmbedDebug Raw Stream]\n");
            header.append(QStringLiteral("Created: %1\n").arg(now).toUtf8());
            header.append(QStringLiteral("Format: Raw Binary\n").arg(now).toUtf8());
            m_file.write(header);
        }
        break;
    case FileFormat::HexDump:
        {
            QString header;
            header.append(QStringLiteral("; EmbedDebug HEX Dump\n"));
            header.append(QStringLiteral("; Created: %1\n").arg(now));
            header.append(QStringLiteral("; Columns: Offset | Hex (16 bytes) | ASCII\n"));
            header.append(QStringLiteral("; Direction: TX=send RX=recv\n"));
            if (m_config.includeDirection) {
                header.append(QStringLiteral("; Dir markers: [TX] / [RX]\n"));
            }
            header.append(";\n");
            m_file.write(header.toUtf8());
        }
        break;
    case FileFormat::TimestampedCsv:
        {
            QString header;
            header.append(QStringLiteral("# EmbedDebug Timestamped CSV\n"));
            header.append(QStringLiteral("# Created: %1\n").arg(now));
            header.append(QStringLiteral("# Fields: timestamp,direction,size,hex_data\n"));
            header.append("timestamp,direction,size,hex_data\n");
            m_file.write(header.toUtf8());
        }
        break;
    }
}

/** @brief 原始二进制写入，方向标记0x00=TX/0x01=RX+2字节长度前缀 */
void DataStreamRecorder::writeRaw(const QByteArray& data, bool isReceived)
{
    if (m_config.includeDirection) {
        /* 方向标记: 0x00=TX, 0x01=RX, 后跟2字节长度(小端)，然后数据 */
        char dir = isReceived ? 0x01 : 0x00;
        m_file.write(&dir, 1);

        /* 2字节长度前缀(小端序) */
        quint16 len = static_cast<quint16>(qMin(data.size(), 65535));
        char lenBuf[2] = { static_cast<char>(len & 0xFF), static_cast<char>((len >> 8) & 0xFF) };
        m_file.write(lenBuf, 2);
    }

    if (m_config.includeTimestamps) {
        /* 8字节微秒时间戳(相对于录制开始的偏移量) */
        qint64 usec = m_recordingTimer.elapsed() * 1000;
        m_file.write(reinterpret_cast<const char*>(&usec), sizeof(usec));
    }

    m_file.write(data);
}

/** @brief 时间戳CSV写入: "ISO时间戳","TX/RX",字节数,"HEX字符串" */
void DataStreamRecorder::writeTimestamped(const QByteArray& data, bool isReceived)
{
    QString line;

    /* 时间戳 */
    if (m_config.includeTimestamps) {
        line.append(QStringLiteral("\"%1\"")
            .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs)));
    } else {
        line.append("\"\"");
    }

    /* 方向 */
    if (m_config.includeDirection) {
        line.append(QStringLiteral(",\"%1\"").arg(isReceived ? "RX" : "TX"));
    } else {
        line.append(",\"\"");
    }

    /* 大小 */
    line.append(QStringLiteral(",%1").arg(data.size()));

    /* HEX数据 */
    QString hex;
    hex.reserve(data.size() * 3);
    for (int i = 0; i < data.size(); ++i) {
        if (i > 0) hex.append(' ');
        hex.append(QStringLiteral("%1").arg(
            static_cast<unsigned char>(data.at(i)), 2, 16, QChar('0')));
    }
    line.append(QStringLiteral(",\"%1\"\n").arg(hex));

    m_file.write(line.toUtf8());
}

/** @brief HEX转储写入: 偏移量|16字节HEX列|ASCII列，方向标记以注释行开头 */
void DataStreamRecorder::writeHexDump(const QByteArray& data, bool isReceived)
{
    /* 方向标记 */
    if (m_config.includeDirection) {
        QString dirLine = QStringLiteral("; [%1] %2 bytes @ %3\n")
            .arg(isReceived ? "RX" : "TX")
            .arg(data.size())
            .arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"));
        m_file.write(dirLine.toUtf8());
    }

    /* 时间戳标记 */
    if (m_config.includeTimestamps && !m_config.includeDirection) {
        QString tsLine = QStringLiteral("; @ %1\n")
            .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs));
        m_file.write(tsLine.toUtf8());
    }

    /* HEX转储: 每行16字节 */
    static const int bytesPerLine = 16;
    for (int offset = 0; offset < data.size(); offset += bytesPerLine) {
        int chunkSize = qMin(bytesPerLine, data.size() - offset);

        /* 偏移量 */
        QString line = QStringLiteral("%1  ").arg(offset, 8, 16, QChar('0'));

        /* HEX列 */
        for (int i = 0; i < bytesPerLine; ++i) {
            if (i < chunkSize) {
                line.append(QStringLiteral("%1 ")
                    .arg(static_cast<unsigned char>(data.at(offset + i)), 2, 16, QChar('0')));
            } else {
                line.append("   ");
            }
            if (i == 7) line.append(' ');
        }

        /* ASCII列 */
        line.append(" |");
        for (int i = 0; i < chunkSize; ++i) {
            unsigned char ch = static_cast<unsigned char>(data.at(offset + i));
            line.append((ch >= 0x20 && ch <= 0x7E) ? QChar(ch) : QChar('.'));
        }
        line.append("|\n");

        m_file.write(line.toUtf8());
    }
}

/** @brief 分割到新文件: 关闭当前→递增序号→创建新文件→写入文件头 @return true=成功 */
bool DataStreamRecorder::splitFile()
{
    QString oldPath = m_file.fileName();

    /* 关闭当前文件 */
    m_file.flush();
    m_file.close();

    /* 递增序号并生成新路径 */
    ++m_fileIndex;
    QString newPath = generateFilePath();

    /* 重新打开新文件 */
    m_file.setFileName(newPath);
    QIODevice::OpenMode mode = QIODevice::WriteOnly;
    if (m_config.format != FileFormat::Raw) {
        mode |= QIODevice::Text;
    }

    if (!m_file.open(mode)) {
        m_recording = false;
        emit errorOccurred(tr("分割文件失败，无法创建新文件: %1 (%2)")
            .arg(newPath, m_file.errorString()));
        return false;
    }

    /* 写入新文件的文件头 */
    writeHeader();

    /* 更新统计 */
    ++m_recordingStats.totalSplits;
    ++m_stats.totalFilesCreated;
    m_recordingStats.currentFileSize = 0;

    emit fileSizeLimitReached(oldPath);
    return true;
}

/** @brief 生成带序号的文件路径: base_0001.ext → base_0002.ext */
QString DataStreamRecorder::generateFilePath()
{
    /* 获取基础路径(不含扩展名) */
    QString base = m_config.filePath;

    /* 根据格式确定扩展名 */
    QString ext;
    switch (m_config.format) {
    case FileFormat::Raw:
        ext = ".bin";
        break;
    case FileFormat::HexDump:
        ext = ".txt";
        break;
    case FileFormat::TimestampedCsv:
        ext = ".csv";
        break;
    }

    /* 如果基础路径已有扩展名则去掉 */
    QFileInfo info(base);
    if (!info.suffix().isEmpty()) {
        base = base.left(base.size() - info.suffix().size() - 1);
    }

    /* 生成带序号的路径: base_0001.ext */
    return QStringLiteral("%1_%2%3")
        .arg(base)
        .arg(m_fileIndex, 4, 10, QChar('0'))
        .arg(ext);
}
