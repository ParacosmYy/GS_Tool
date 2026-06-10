/**
 * @file DataExporter.cpp
 * @brief 数据导出器实现 - Plain/HexDump/CSV/Timestamped/Bin/Json 六种格式
 *
 * HexDump 格式将所有行数据拼接后按经典16字节/行输出。
 * 时间范围过滤仅在 exportToFile 模式下支持。
 * 所有写入操作均检查 QFile 错误状态，失败时发射 exportError 信号。
 *
 * CSV增强:
 * - 可配置列分隔符(setCsvDelimiter)，默认逗号
 * - 可配置BOM头(setCsvBomEnabled)，默认启用，确保Excel中文兼容
 *
 * 统计增强:
 * - 每次导出记录耗时(totalExportDurationMs/lastExportDurationMs)
 * - 每次导出记录行数和字节数(lastExportRowCount/lastExportByteCount)
 * - 成功后发射exportCompleted信号携带完整统计摘要
 *
 * 流式导出方法见 DataExporterStreamed.cpp
 * EDL范围导出方法见 DataExporterEdl.cpp
 */

#include "utils/export/DataExporter.h"
#include "utils/crypto/HexConverter.h"
#include "shared/AppConstants.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

/** @brief 构造数据导出器 @param parent 父对象 */
DataExporter::DataExporter(QObject* parent) : QObject(parent) {}

// ---- CSV配置 ----

/** @brief 设置CSV列分隔符 @param delim 分隔符字符 */
void DataExporter::setCsvDelimiter(QChar delim) {
    m_csvDelimiter = delim;
}

/** @brief 获取当前CSV列分隔符 @return 分隔符字符 */
QChar DataExporter::csvDelimiter() const {
    return m_csvDelimiter;
}

/** @brief 设置CSV是否写入UTF-8 BOM头 @param enable true=写入BOM */
void DataExporter::setCsvBomEnabled(bool enable) {
    m_csvBomEnabled = enable;
}

/** @brief 获取CSV是否写入BOM头 @return true=启用BOM */
bool DataExporter::isCsvBomEnabled() const {
    return m_csvBomEnabled;
}

// ---- 公共入口 ----

/** @brief 导出数据到文件(批量模式+进度回调)，记录耗时和统计 @param filePath 目标文件路径 @param format 导出格式 @param lines 终端行数据 @param progress 进度回调(返回false取消) @param from 起始时间过滤 @param to 结束时间过滤 @return 是否成功 */
bool DataExporter::exportToFile(const QString& filePath, Format format,
                                 const QVector<TerminalLine>& lines,
                                 ProgressCallback progress,
                                 const QDateTime& from, const QDateTime& to)
{
    const QString normalizedPath = filePath.trimmed();
    if (normalizedPath.isEmpty()) {
        ++m_totalErrors;
        emit exportError(normalizedPath, tr("文件路径为空"));
        return false;
    }
    if (lines.isEmpty()) {
        ++m_totalEmptySkips;
        emit exportError(normalizedPath, tr("没有数据可导出"));
        return false;
    }

    QVector<TerminalLine> filtered = filterByTime(lines, from, to);
    if (filtered.isEmpty()) {
        ++m_totalEmptySkips;
        emit exportError(normalizedPath, tr("时间范围内没有数据可导出"));
        return false;
    }
    m_totalFilteredRows += static_cast<quint64>(lines.size() - filtered.size());

    m_exportTimer.start();

    bool ok = false;
    const int totalRows = filtered.size();
    switch (format) {
    case Plain: {
        QFile file(normalizedPath);
        QTextStream out;
        if (!openTextFile(file, out, normalizedPath)) break;
        for (int i = 0; i < totalRows; ++i) {
            const TerminalLine& line = filtered[i];
            out << QString("[%1] [%2] %3 | %4\n")
                    .arg(line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz"),
                         (line.direction == DataDirection::Rx) ? "RX" : "TX",
                         HexConverter::toHexString(line.data), toAsciiString(line.data));
            if (!reportProgress(progress, normalizedPath, i + 1, totalRows)) {
                file.close();
                ++m_totalCancelled;
                emit exportCancelled(normalizedPath);
                return false;
            }
        }
        ok = flushAndCheck(file, out, normalizedPath);
        break;
    }
    case Csv: {
        QFile file(normalizedPath);
        QTextStream out;
        if (!openTextFile(file, out, normalizedPath)) break;
        if (!writeCsvBom(file, normalizedPath)) { file.close(); break; }
        out << csvHeader() << '\n';
        for (int i = 0; i < totalRows; ++i) {
            const TerminalLine& line = filtered[i];
            out << line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz") << m_csvDelimiter
                << ((line.direction == DataDirection::Rx) ? "RX" : "TX") << m_csvDelimiter
                << HexConverter::toHexString(line.data) << m_csvDelimiter
                << escapeCsvField(toAsciiString(line.data)) << '\n';
            if (!reportProgress(progress, normalizedPath, i + 1, totalRows)) {
                file.close();
                ++m_totalCancelled;
                emit exportCancelled(normalizedPath);
                return false;
            }
        }
        ok = flushAndCheck(file, out, normalizedPath);
        break;
    }
    case Json: {
        QJsonObject root;
        root["export_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        root["total_lines"] = totalRows;
        QJsonArray linesArray;
        for (int i = 0; i < totalRows; ++i) {
            const TerminalLine& line = filtered[i];
            QJsonObject lineObj;
            lineObj["timestamp"] = line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz");
            lineObj["direction"] = (line.direction == DataDirection::Rx) ? "RX" : "TX";
            lineObj["hex"] = HexConverter::toHexString(line.data);
            lineObj["ascii"] = toAsciiString(line.data);
            linesArray.append(lineObj);
            if (!reportProgress(progress, normalizedPath, i + 1, totalRows)) {
                ++m_totalCancelled;
                emit exportCancelled(normalizedPath);
                return false;
            }
        }
        root["lines"] = linesArray;
        QFile file(normalizedPath);
        if (!file.open(QIODevice::WriteOnly)) {
            emit exportError(normalizedPath, tr("无法打开文件: %1").arg(file.errorString()));
            break;
        }
        QJsonDocument doc(root);
        if (file.write(doc.toJson(QJsonDocument::Indented)) == -1) {
            emit exportError(normalizedPath, tr("写入文件失败: %1").arg(file.errorString()));
            file.close(); break;
        }
        file.close();
        ok = true;
        break;
    }
    case HexDump:     ok = exportHexDump(normalizedPath, filtered); break;
    case Timestamped: ok = exportTimestamped(normalizedPath, filtered); break;
    case Bin:         ok = exportBin(normalizedPath, filtered); break;
    default:
        ++m_totalErrors;
        emit exportError(normalizedPath, tr("不支持的导出格式: %1").arg(static_cast<int>(format)));
        return false;
    }

    qint64 durationMs = m_exportTimer.elapsed();
    m_lastExportDurationMs = durationMs;
    m_totalExportDurationMs += durationMs;

    if (ok) {
        ++m_totalExports;
        switch (format) {
        case Plain:       ++m_totalPlainExports; break;
        case HexDump:     ++m_totalHexDumpExports; break;
        case Csv:         ++m_totalCsvExports; break;
        case Timestamped: ++m_totalTimestampedExports; break;
        case Bin:         ++m_totalBinExports; break;
        case Json:        ++m_totalJsonExports; break;
        default: break;
        }
        quint64 byteCount = 0;
        for (const auto& line : filtered) {
            byteCount += static_cast<quint64>(line.data.size());
        }
        m_totalBytesExported += byteCount;
        m_totalRowsExported += static_cast<quint64>(filtered.size());
        m_lastExportRowCount = static_cast<quint64>(filtered.size());
        m_lastExportByteCount = byteCount;
        emit exportCompleted(normalizedPath, format,
                             m_lastExportRowCount, byteCount, durationMs);
    } else {
        ++m_totalErrors;
        m_lastExportRowCount = 0;
        m_lastExportByteCount = 0;
    }
    return ok;
}

/** @brief 报告导出进度并检查是否应取消 @param progress 进度回调(可空) @param filePath 文件路径 @param current 当前行索引 @param total 总行数 @return true=继续，false=用户取消 */
bool DataExporter::reportProgress(ProgressCallback& progress, const QString& filePath, int current, int total)
{
    if (total <= 0) return true;
    int percent = qMin(100, static_cast<int>(static_cast<qint64>(current) * 100 / total));
    emit exportProgress(filePath, percent);
    if (progress) {
        return progress(percent);
    }
    return true;
}

/** @brief 导出数据到文件(批量模式)，记录耗时和统计 @param filePath 目标文件路径 @param format 导出格式 @param lines 终端行数据 @param from 起始时间过滤 @param to 结束时间过滤 @return 是否成功 */
bool DataExporter::exportToFile(const QString& filePath, Format format,
                                 const QVector<TerminalLine>& lines,
                                 const QDateTime& from, const QDateTime& to)
{
    const QString normalizedPath = filePath.trimmed();
    if (normalizedPath.isEmpty()) {
        ++m_totalErrors;
        emit exportError(normalizedPath, tr("文件路径为空"));
        return false;
    }
    if (lines.isEmpty()) {
        ++m_totalEmptySkips;
        emit exportError(normalizedPath, tr("没有数据可导出"));
        return false;
    }

    QVector<TerminalLine> filtered = filterByTime(lines, from, to);
    if (filtered.isEmpty()) {
        ++m_totalEmptySkips;
        emit exportError(normalizedPath, tr("时间范围内没有数据可导出"));
        return false;
    }
    m_totalFilteredRows += static_cast<quint64>(lines.size() - filtered.size());

    // 开始计时
    m_exportTimer.start();

    bool ok = false;
    switch (format) {
    case Plain:       ok = exportPlain(normalizedPath, filtered); break;
    case HexDump:     ok = exportHexDump(normalizedPath, filtered); break;
    case Csv:         ok = exportCsv(normalizedPath, filtered); break;
    case Timestamped: ok = exportTimestamped(normalizedPath, filtered); break;
    case Bin:         ok = exportBin(normalizedPath, filtered); break;
    case Json:        ok = exportJson(normalizedPath, filtered); break;
    default:
        ++m_totalErrors;
        emit exportError(normalizedPath, tr("不支持的导出格式: %1").arg(static_cast<int>(format)));
        return false;
    }

    // 计算耗时
    qint64 durationMs = m_exportTimer.elapsed();
    m_lastExportDurationMs = durationMs;
    m_totalExportDurationMs += durationMs;

    if (ok) {
        ++m_totalExports;
        switch (format) {
        case Plain:       ++m_totalPlainExports; break;
        case HexDump:     ++m_totalHexDumpExports; break;
        case Csv:         ++m_totalCsvExports; break;
        case Timestamped: ++m_totalTimestampedExports; break;
        case Bin:         ++m_totalBinExports; break;
        case Json:        ++m_totalJsonExports; break;
        default: break;
        }
        quint64 byteCount = 0;
        for (const auto& line : filtered) {
            byteCount += static_cast<quint64>(line.data.size());
        }
        m_totalBytesExported += byteCount;
        m_totalRowsExported += static_cast<quint64>(filtered.size());
        m_lastExportRowCount = static_cast<quint64>(filtered.size());
        m_lastExportByteCount = byteCount;
        emit exportCompleted(normalizedPath, format,
                             m_lastExportRowCount, byteCount, durationMs);
    } else {
        ++m_totalErrors;
        m_lastExportRowCount = 0;
        m_lastExportByteCount = 0;
    }
    return ok;
}

// ---- 辅助方法 ----

/** @brief 打开文本文件用于写入并设置UTF8编码，失败时发射exportError @param file 文件对象 @param out 文本流 @param path 文件路径 @return 是否成功 */
bool DataExporter::openTextFile(QFile& file, QTextStream& out, const QString& path)
{
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit exportError(path, tr("无法打开文件: %1").arg(file.errorString()));
        return false;
    }
    out.setDevice(&file);
    out.setEncoding(QStringConverter::Utf8);
    return true;
}

/** @brief 刷新流并检查写入错误 @param file 文件对象 @param out 文本流 @param path 文件路径(用于错误消息) @return 是否成功 */
bool DataExporter::flushAndCheck(QFile& file, QTextStream& out, const QString& path)
{
    out.flush();
    if (file.error() != QFile::NoError) {
        emit exportError(path, tr("写入文件失败: %1").arg(file.errorString()));
        file.close();
        return false;
    }
    file.close();
    return true;
}

/** @brief 按时间范围过滤终端行(支持单端/双端/无过滤) @param lines 原始行列表 @param from 起始时间(无效=无下界) @param to 结束时间(无效=无上界) @return 过滤后的行列表 */
QVector<TerminalLine> DataExporter::filterByTime(
    const QVector<TerminalLine>& lines, const QDateTime& from, const QDateTime& to) const
{
    bool hasFrom = from.isValid();
    bool hasTo = to.isValid();
    if (!hasFrom && !hasTo) return lines;

    QVector<TerminalLine> result;
    result.reserve(lines.size());
    for (const TerminalLine& line : lines) {
        if (hasFrom && line.timestamp < from) continue;
        if (hasTo && line.timestamp > to) continue;
        result.append(line);
    }
    return result;
}

/** @brief 写入CSV BOM头(如果启用) @param file 已打开的文件对象 @param path 文件路径(用于错误报告) @return true=BOM写入成功或不需要BOM */
bool DataExporter::writeCsvBom(QFile& file, const QString& path)
{
    if (!m_csvBomEnabled) return true;
    if (file.write("\xEF\xBB\xBF") != 3) {
        emit exportError(path, tr("写入BOM失败"));
        return false;
    }
    return true;
}

/** @brief 生成CSV表头行(使用当前配置的分隔符) @return 表头字符串(不含尾随换行) */
QString DataExporter::csvHeader() const
{
    return QString("timestamp%1direction%2data_hex%3data_ascii")
        .arg(m_csvDelimiter, m_csvDelimiter, m_csvDelimiter);
}

// 全量导出格式方法见 DataExporterFormats.cpp
// 静态辅助方法(toAsciiString/escapeCsvField/concatData/formatHexDumpLine)见 DataExporterUtils.cpp
// 会话统计查询方法见 DataExporterStats.cpp
