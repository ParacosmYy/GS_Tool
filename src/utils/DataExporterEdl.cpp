/**
 * @file DataExporterEdl.cpp
 * @brief 数据导出器 - EDL范围导出方法实现
 *
 * 从EDL二进制日志文件中按时间范围读取记录并导出为指定格式。
 * EDL记录格式(BigEndian): timestamp(8B) + direction(1B) + length(4B) + data(length B)。
 */

#include "utils/DataExporter.h"
#include "core/Constants.h"

#include <QFile>
#include <QDataStream>
#include <QDateTime>
#include <QTimeZone>

// ---- EDL范围导出 ----

/** @brief 从EDL日志文件导出指定时间范围的数据 @param edlPath 日志文件路径 @param format 导出格式 @param outPath 输出文件路径 @param fromMs 起始时间(ms) @param toMs 结束时间(ms) @return 是否成功 */
bool DataExporter::exportRange(const QString& edlPath, Format format,
                                const QString& outPath,
                                qint64 fromMs, qint64 toMs)
{
    if (edlPath.isEmpty() || outPath.isEmpty()) return false;
    if (fromMs >= 0 && toMs >= 0 && fromMs > toMs) return false;

    QVector<TerminalLine> lines = readEdlRange(edlPath, fromMs, toMs);
    m_lastExportRangeCount = lines.size();
    if (lines.isEmpty()) return false;

    switch (format) {
    case Plain:       return exportPlain(outPath, lines);
    case HexDump:     return exportHexDump(outPath, lines);
    case Csv:         return exportCsv(outPath, lines);
    case Timestamped: return exportTimestamped(outPath, lines);
    case Bin:         return exportBin(outPath, lines);
    case Json:        return exportJson(outPath, lines);
    }
    return false;
}

/** @brief 返回上次exportRange调用实际导出的行数 @return 导出行数 */
int DataExporter::lastExportRangeCount() const { return m_lastExportRangeCount; }

/**
 * @brief 从EDL二进制文件中读取指定时间范围的记录
 * EDL记录(BigEndian): timestamp(8B) + direction(1B) + length(4B) + data(length B)
 */
QVector<TerminalLine> DataExporter::readEdlRange(const QString& edlPath,
                                                  qint64 fromMs, qint64 toMs)
{
    QVector<TerminalLine> result;
    QFile file(edlPath);
    if (!file.open(QIODevice::ReadOnly)) return result;

    // 验证文件头: magic(3B) + version(1B) + padding(4B)
    QByteArray magic = file.read(3);
    if (magic.size() != 3 || magic != kEdlMagic) return result;
    quint8 version = 0;
    if (file.read(reinterpret_cast<char*>(&version), 1) != 1 || version != kEdlVersion)
        return result;
    if (!file.seek(kEdlHeaderSize)) return result;

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::BigEndian);
    const QDateTime baseTime = QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0), QTimeZone::UTC);

    while (!file.atEnd()) {
        quint64 timestamp = 0;
        stream >> timestamp;
        if (stream.status() != QDataStream::Ok) break;
        quint8 direction = 0;
        stream >> direction;
        if (stream.status() != QDataStream::Ok) break;
        quint32 length = 0;
        stream >> length;
        if (stream.status() != QDataStream::Ok) break;
        if (length > kEdlMaxRecordSize) break;

        QByteArray data(static_cast<int>(length), Qt::Uninitialized);
        if (stream.readRawData(data.data(), static_cast<int>(length)) != static_cast<int>(length))
            break;

        qint64 tsMs = static_cast<qint64>(timestamp);
        if (toMs >= 0 && tsMs > toMs) break;
        if (fromMs >= 0 && tsMs < fromMs) continue;

        TerminalLine line;
        line.data = data;
        line.direction = (direction == 0) ? DataDirection::Rx : DataDirection::Tx;
        line.timestamp = baseTime.addMSecs(tsMs);
        result.append(line);
    }
    return result;
}
