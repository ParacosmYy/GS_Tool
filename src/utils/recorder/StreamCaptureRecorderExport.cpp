/**
 * @file StreamCaptureRecorderExport.cpp
 * @brief 流捕获录制器 -- 导出方法实现(JSON/CSV/Binary)
 *
 * 从 StreamCaptureRecorder.cpp 拆分而来，包含:
 *   - exportToFile() 格式分发
 *   - exportJson() JSON数组导出
 *   - exportCsv() CSV表格导出
 *   - exportBinary() 二进制紧凑导出
 *
 * 录制/查询/统计逻辑保留在 StreamCaptureRecorder.cpp。
 */

#include "utils/recorder/StreamCaptureRecorder.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>
#include <QTextStream>

// ============================================================
// 导出入口
// ============================================================

/** @brief 导出缓冲区记录到文件 @param filePath 目标路径 @param format 导出格式 */
bool StreamCaptureRecorder::exportToFile(const QString& filePath, ExportFormat format) const
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

// ============================================================
// JSON 导出
// ============================================================

/** @brief JSON格式导出: 每条记录一个JSON对象，整体为数组 */
bool StreamCaptureRecorder::exportJson(const QString& filePath) const
{
    QMutexLocker locker(&m_bufferMutex);

    QJsonArray recordsArray;
    for (const auto& entry : m_buffer) {
        QJsonObject obj;
        obj[QStringLiteral("timestamp_us")] = static_cast<qint64>(entry.timestampUs);
        obj[QStringLiteral("direction")] = (entry.direction == Direction::RX)
            ? QStringLiteral("RX") : QStringLiteral("TX");

        /* 数据转HEX字符串 */
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

    /* 标记导出 */
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

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

// ============================================================
// CSV 导出
// ============================================================

/** @brief CSV格式导出: timestamp_us,direction,size,hex_data */
bool StreamCaptureRecorder::exportCsv(const QString& filePath) const
{
    QMutexLocker locker(&m_bufferMutex);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream stream(&file);

    /* 文件头 */
    stream << QStringLiteral("# EmbedDebug StreamCaptureRecorder Export\n");
    stream << QStringLiteral("# Created: %1\n")
        .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs));
    stream << QStringLiteral("timestamp_us,direction,size,hex_data\n");

    /* 数据行 */
    for (const auto& entry : m_buffer) {
        stream << entry.timestampUs << ','
               << (entry.direction == Direction::RX ? "RX" : "TX") << ','
               << entry.data.size() << ',';

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
            .arg(marker.name).arg(marker.timestampUs);
    }

    stream.flush();
    file.close();
    return true;
}

// ============================================================
// Binary 导出
// ============================================================

/** @brief 二进制格式: 魔数+文件头+紧凑记录+标记 */
bool StreamCaptureRecorder::exportBinary(const QString& filePath) const
{
    QMutexLocker locker(&m_bufferMutex);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;

    /* 文件头: 魔数(4B) + 版本(2B) + 记录数(8B) + 标记数(8B) */
    const char magic[] = "EDSR"; /* EmbedDebug Stream Recorder */
    file.write(magic, 4);

    quint16 version = 1;
    file.write(reinterpret_cast<const char*>(&version), sizeof(quint16));

    quint64 recordCount = static_cast<quint64>(m_buffer.size());
    file.write(reinterpret_cast<const char*>(&recordCount), sizeof(quint64));

    quint64 markerCount = static_cast<quint64>(m_markers.size());
    file.write(reinterpret_cast<const char*>(&markerCount), sizeof(quint64));

    /* 记录: [8B时间戳][1B方向][4B长度][NB数据] */
    for (const auto& entry : m_buffer) {
        file.write(reinterpret_cast<const char*>(&entry.timestampUs), sizeof(quint64));
        char dir = (entry.direction == Direction::RX) ? 0x01 : 0x00;
        file.write(&dir, 1);
        quint32 len = static_cast<quint32>(entry.data.size());
        file.write(reinterpret_cast<const char*>(&len), sizeof(quint32));
        file.write(entry.data);
    }

    /* 标记: [8B时间戳][2B名称长度][NB名称UTF-8] */
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
