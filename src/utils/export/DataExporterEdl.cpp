/**
 * @file DataExporterEdl.cpp
 * @brief 数据导出器 - EDL范围导出方法实现
 *
 * 从EDL二进制日志文件中按时间范围读取记录并导出为指定格式。
 * EDL记录格式(BigEndian): timestamp(8B) + direction(1B) + length(4B) + data(length B)。
 *
 * EDL范围导出同样记录耗时统计，并在成功后发射exportCompleted信号。
 */

#include "utils/export/DataExporter.h"
#include "shared/AppConstants.h"

#include <QFile>
#include <QDataStream>
#include <QDateTime>
#include <QTimeZone>

// ---- EDL范围导出 ----

/** @brief 从EDL日志文件导出指定时间范围的数据，记录耗时和统计 @param edlPath 日志文件路径 @param format 导出格式 @param outPath 输出文件路径 @param fromMs 起始时间(ms) @param toMs 结束时间(ms) @return 是否成功 */
bool DataExporter::exportRange(const QString& edlPath, Format format,
                                const QString& outPath,
                                qint64 fromMs, qint64 toMs)
{
    if (edlPath.isEmpty() || outPath.isEmpty()) return false;

    ++m_totalExports;
    if (fromMs >= 0 && toMs >= 0 && fromMs > toMs) return false;

    // 开始计时
    m_exportTimer.start();

    QVector<TerminalLine> lines = readEdlRange(edlPath, fromMs, toMs);
    m_lastExportRangeCount = lines.size();
    if (lines.isEmpty()) return false;

    bool ok = false;
    switch (format) {
    case Plain:       ok = exportPlain(outPath, lines); break;
    case HexDump:     ok = exportHexDump(outPath, lines); ++m_totalHexDumpExports; break;
    case Csv:         ok = exportCsv(outPath, lines); ++m_totalCsvExports; break;
    case Timestamped: ok = exportTimestamped(outPath, lines); break;
    case Bin:         ok = exportBin(outPath, lines); ++m_totalBinExports; break;
    case Json:        ok = exportJson(outPath, lines); ++m_totalJsonExports; break;
    default:
        ++m_totalErrors;
        emit exportError(outPath, tr("不支持的导出格式: %1").arg(static_cast<int>(format)));
        return false;
    }

    // 计算耗时
    qint64 durationMs = m_exportTimer.elapsed();
    m_lastExportDurationMs = durationMs;
    m_totalExportDurationMs += durationMs;

    if (ok) {
        quint64 byteCount = 0;
        for (const auto& line : lines) {
            byteCount += static_cast<quint64>(line.data.size());
        }
        m_totalBytesExported += byteCount;
        m_totalRowsExported += static_cast<quint64>(lines.size());
        m_lastExportRowCount = static_cast<quint64>(lines.size());
        m_lastExportByteCount = byteCount;
        emit exportCompleted(outPath, format,
                             m_lastExportRowCount, byteCount, durationMs);
    } else {
        ++m_totalErrors;
        m_lastExportRowCount = 0;
        m_lastExportByteCount = 0;
    }
    return ok;
}

/** @brief 返回上次exportRange调用实际导出的行数 @return 导出行数 */
int DataExporter::lastExportRangeCount() const { return m_lastExportRangeCount; }

/** @brief 从EDL二进制文件中读取指定时间范围的记录，EDL记录(BigEndian): timestamp(8B) + direction(1B) + length(4B) + data(length B) @param edlPath EDL日志文件路径 @param fromMs 起始时间戳(ms)，-1表示不限制 @param toMs 结束时间戳(ms)，-1表示不限制 @return 符合时间范围的TerminalLine列表 */
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
    stream.setVersion(QDataStream::Qt_6_8);
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
