/**
 * @file DataExporterStreamed.cpp
 * @brief 数据导出器 - 流式导出方法实现
 *
 * 流式导出通过 LineProvider 回调分批拉取数据，避免一次性将所有行加载到内存。
 * 支持 Plain/HexDump/CSV/Timestamped/Bin/Json 六种格式的流式变体，
 * 以及 exportStreamed() 统一分发入口。
 *
 * CSV流式导出使用可配置的分隔符(csvDelimiter)和BOM头设置(csvBomEnabled)。
 * 流式导出同样记录耗时和统计信息，并在成功后发射exportCompleted信号。
 */

#include "utils/export/DataExporter.h"
#include "utils/crypto/HexConverter.h"
#include "shared/AppConstants.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>

namespace {

enum class StreamedInputError {
    None,
    EmptyPath,
    MissingProvider,
    EmptyData
};

StreamedInputError validateStreamedExportInput(const QString& filePath,
                                               const DataExporter::LineProvider& lineProvider,
                                               int totalLines)
{
    if (filePath.isEmpty()) return StreamedInputError::EmptyPath;
    if (!lineProvider) return StreamedInputError::MissingProvider;
    if (totalLines <= 0) return StreamedInputError::EmptyData;
    return StreamedInputError::None;
}

} // namespace

// ---- 流式导出入口 ----

/** @brief 导出数据到文件(流式模式+进度回调)，记录耗时和统计 @param filePath 目标路径 @param format 格式 @param lineProvider 行数据提供回调 @param totalLines 总行数 @param batchSize 每批行数 @param progress 进度回调(返回false取消) @return 是否成功 */
bool DataExporter::exportStreamed(const QString& filePath, Format format,
                                   LineProvider lineProvider,
                                   int totalLines, int batchSize,
                                   ProgressCallback progress)
{
    switch (validateStreamedExportInput(filePath, lineProvider, totalLines)) {
    case StreamedInputError::None:
        break;
    case StreamedInputError::EmptyPath:
        ++m_totalErrors;
        emit exportError(filePath, tr("文件路径为空"));
        return false;
    case StreamedInputError::MissingProvider:
        ++m_totalErrors;
        emit exportError(filePath, tr("数据源不可用"));
        return false;
    case StreamedInputError::EmptyData:
        ++m_totalEmptySkips;
        emit exportError(filePath, tr("没有数据可导出"));
        return false;
    }

    ++m_totalExports;
    m_exportTimer.start();
    const int effectiveBatchSize = (batchSize > 0) ? batchSize : 1000;

    bool ok = false;
    int rowsExported = 0;
    switch (format) {
    case Plain: {
        QFile file(filePath);
        QTextStream out;
        if (!openTextFile(file, out, filePath)) break;
        int offset = 0;
        while (offset < totalLines) {
            QVector<TerminalLine> batch = lineProvider(offset, qMin(effectiveBatchSize, totalLines - offset));
            if (batch.isEmpty()) break;
            for (const TerminalLine& line : batch) {
                out << QString("[%1] [%2] %3 | %4\n")
                        .arg(line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz"),
                             (line.direction == DataDirection::Rx) ? "RX" : "TX",
                             HexConverter::toHexString(line.data), toAsciiString(line.data));
            }
            offset += batch.size();
            rowsExported = offset;
            if (!reportProgress(progress, filePath, rowsExported, totalLines)) {
                file.close();
                ++m_totalCancelled;
                emit exportCancelled(filePath);
                return false;
            }
        }
        if (rowsExported <= 0) {
            file.close();
            emit exportError(filePath, tr("没有数据可导出"));
            break;
        }
        ok = flushAndCheck(file, out, filePath);
        break;
    }
    case Csv: {
        QFile file(filePath);
        QTextStream out;
        if (!openTextFile(file, out, filePath)) break;
        if (!writeCsvBom(file, filePath)) { file.close(); break; }
        out << csvHeader() << '\n';
        int offset = 0;
        while (offset < totalLines) {
            QVector<TerminalLine> batch = lineProvider(offset, qMin(effectiveBatchSize, totalLines - offset));
            if (batch.isEmpty()) break;
            for (const TerminalLine& line : batch) {
                out << line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz") << m_csvDelimiter
                    << ((line.direction == DataDirection::Rx) ? "RX" : "TX") << m_csvDelimiter
                    << HexConverter::toHexString(line.data) << m_csvDelimiter
                    << escapeCsvField(toAsciiString(line.data)) << '\n';
            }
            offset += batch.size();
            rowsExported = offset;
            if (!reportProgress(progress, filePath, rowsExported, totalLines)) {
                file.close();
                ++m_totalCancelled;
                emit exportCancelled(filePath);
                return false;
            }
        }
        if (rowsExported <= 0) {
            file.close();
            emit exportError(filePath, tr("没有数据可导出"));
            break;
        }
        ok = flushAndCheck(file, out, filePath);
        break;
    }
    case HexDump:     ok = exportStreamedHexDump(filePath, lineProvider, totalLines, effectiveBatchSize); break;
    case Timestamped: ok = exportStreamedTimestamped(filePath, lineProvider, totalLines, effectiveBatchSize); break;
    case Bin:         ok = exportStreamedBin(filePath, lineProvider, totalLines, effectiveBatchSize); break;
    case Json:        ok = exportStreamedJson(filePath, lineProvider, totalLines, effectiveBatchSize); break;
    default:
        ++m_totalErrors;
        emit exportError(filePath, tr("不支持的导出格式: %1").arg(static_cast<int>(format)));
        return false;
    }

    qint64 durationMs = m_exportTimer.elapsed();
    m_lastExportDurationMs = durationMs;
    m_totalExportDurationMs += durationMs;

    if (ok) {
        switch (format) {
        case Plain:       ++m_totalPlainExports; break;
        case HexDump:     ++m_totalHexDumpExports; break;
        case Csv:         ++m_totalCsvExports; break;
        case Timestamped: ++m_totalTimestampedExports; break;
        case Bin:         ++m_totalBinExports; break;
        case Json:        ++m_totalJsonExports; break;
        default: break;
        }
        m_totalRowsExported += static_cast<quint64>(rowsExported > 0 ? rowsExported : totalLines);
        m_lastExportRowCount = static_cast<quint64>(rowsExported > 0 ? rowsExported : totalLines);
        m_lastExportByteCount = 0;
        emit exportCompleted(filePath, format,
                             m_lastExportRowCount, m_lastExportByteCount, durationMs);
    } else {
        ++m_totalErrors;
        m_lastExportRowCount = 0;
        m_lastExportByteCount = 0;
    }
    return ok;
}

/** @brief 导出数据到文件(流式模式，无进度回调)，记录耗时和统计 @param filePath 目标路径 @param format 格式 @param lineProvider 行数据提供回调 @param totalLines 总行数 @param batchSize 每批行数 @return 是否成功 */
bool DataExporter::exportStreamed(const QString& filePath, Format format,
                                   LineProvider lineProvider,
                                   int totalLines, int batchSize)
{
    switch (validateStreamedExportInput(filePath, lineProvider, totalLines)) {
    case StreamedInputError::None:
        break;
    case StreamedInputError::EmptyPath:
        ++m_totalErrors;
        emit exportError(filePath, tr("文件路径为空"));
        return false;
    case StreamedInputError::MissingProvider:
        ++m_totalErrors;
        emit exportError(filePath, tr("数据源不可用"));
        return false;
    case StreamedInputError::EmptyData:
        ++m_totalEmptySkips;
        emit exportError(filePath, tr("没有数据可导出"));
        return false;
    }

    ++m_totalExports;
    m_exportTimer.start();
    const int effectiveBatchSize = (batchSize > 0) ? batchSize : 1000;

    bool ok = false;
    switch (format) {
    case Plain:       ok = exportStreamedPlain(filePath, lineProvider, totalLines, effectiveBatchSize); break;
    case HexDump:     ok = exportStreamedHexDump(filePath, lineProvider, totalLines, effectiveBatchSize); break;
    case Csv:         ok = exportStreamedCsv(filePath, lineProvider, totalLines, effectiveBatchSize); break;
    case Timestamped: ok = exportStreamedTimestamped(filePath, lineProvider, totalLines, effectiveBatchSize); break;
    case Bin:         ok = exportStreamedBin(filePath, lineProvider, totalLines, effectiveBatchSize); break;
    case Json:        ok = exportStreamedJson(filePath, lineProvider, totalLines, effectiveBatchSize); break;
    default:
        ++m_totalErrors;
        emit exportError(filePath, tr("不支持的导出格式: %1").arg(static_cast<int>(format)));
        return false;
    }

    // 计算耗时
    qint64 durationMs = m_exportTimer.elapsed();
    m_lastExportDurationMs = durationMs;
    m_totalExportDurationMs += durationMs;

    if (ok) {
        switch (format) {
        case Plain:       ++m_totalPlainExports; break;
        case HexDump:     ++m_totalHexDumpExports; break;
        case Csv:         ++m_totalCsvExports; break;
        case Timestamped: ++m_totalTimestampedExports; break;
        case Bin:         ++m_totalBinExports; break;
        case Json:        ++m_totalJsonExports; break;
        default: break;
        }
        // 流式模式下无法精确统计字节数，用totalLines估算
        m_totalRowsExported += static_cast<quint64>(totalLines);
        m_lastExportRowCount = static_cast<quint64>(totalLines);
        m_lastExportByteCount = 0; // 流式模式无法精确统计，设为0
        emit exportCompleted(filePath, format,
                             m_lastExportRowCount, m_lastExportByteCount, durationMs);
    } else {
        ++m_totalErrors;
        m_lastExportRowCount = 0;
        m_lastExportByteCount = 0;
    }
    return ok;
}

// ---- 流式导出方法 ----

/** @brief 流式导出纯文本格式 @param path 文件路径 @param provider 行数据提供回调 @param totalLines 总行数 @param batchSize 每批行数 @return 是否成功 */
bool DataExporter::exportStreamedPlain(const QString& path, LineProvider provider,
                                        int totalLines, int batchSize)
{
    QFile file(path);
    QTextStream out;
    if (!openTextFile(file, out, path)) return false;

    int offset = 0;
    while (offset < totalLines) {
        QVector<TerminalLine> batch = provider(offset, qMin(batchSize, totalLines - offset));
        if (batch.isEmpty()) break;
        for (const TerminalLine& line : batch) {
            out << QString("[%1] [%2] %3 | %4\n")
                    .arg(line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz"),
                         (line.direction == DataDirection::Rx) ? "RX" : "TX",
                         HexConverter::toHexString(line.data), toAsciiString(line.data));
        }
        offset += batch.size();
    }
    if (offset <= 0) {
        file.close();
        emit exportError(path, tr("没有数据可导出"));
        return false;
    }
    return flushAndCheck(file, out, path);
}

/** @brief 流式导出HEX转储格式，维护全局地址偏移和跨批次残余缓冲区 @param path 文件路径 @param provider 行数据提供回调 @param totalLines 总行数 @param batchSize 每批行数 @return 是否成功 */
bool DataExporter::exportStreamedHexDump(const QString& path, LineProvider provider,
                                          int totalLines, int batchSize)
{
    QFile file(path);
    QTextStream out;
    if (!openTextFile(file, out, path)) return false;

    const int bytesPerLine = 16;
    QByteArray residual;
    quint64 globalAddr = 0;
    int offset = 0;

    while (offset < totalLines) {
        QVector<TerminalLine> batch = provider(offset, qMin(batchSize, totalLines - offset));
        if (batch.isEmpty()) break;
        QByteArray batchData = residual + concatData(batch);

        int pos = 0;
        while (pos + bytesPerLine <= batchData.size()) {
            out << formatHexDumpLine(batchData.mid(pos, bytesPerLine), globalAddr) << '\n';
            pos += bytesPerLine;
            globalAddr += bytesPerLine;
        }
        residual = batchData.mid(pos);
        offset += batch.size();
    }
    if (offset <= 0) {
        file.close();
        emit exportError(path, tr("没有数据可导出"));
        return false;
    }
    if (!residual.isEmpty())
        out << formatHexDumpLine(residual, globalAddr) << '\n';

    return flushAndCheck(file, out, path);
}

/** @brief 流式导出CSV格式(可配置BOM头和分隔符) @param path 文件路径 @param provider 行数据提供回调 @param totalLines 总行数 @param batchSize 每批行数 @return 是否成功 */
bool DataExporter::exportStreamedCsv(const QString& path, LineProvider provider,
                                       int totalLines, int batchSize)
{
    QFile file(path);
    QTextStream out;
    if (!openTextFile(file, out, path)) return false;

    // UTF-8 BOM: 根据配置决定是否写入，确保Excel中文环境下正确识别编码
    if (!writeCsvBom(file, path)) {
        file.close();
        return false;
    }

    out << csvHeader() << '\n';
    int offset = 0;
    while (offset < totalLines) {
        QVector<TerminalLine> batch = provider(offset, qMin(batchSize, totalLines - offset));
        if (batch.isEmpty()) break;
        for (const TerminalLine& line : batch) {
            out << line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz") << m_csvDelimiter
                << ((line.direction == DataDirection::Rx) ? "RX" : "TX") << m_csvDelimiter
                << HexConverter::toHexString(line.data) << m_csvDelimiter
                << escapeCsvField(toAsciiString(line.data)) << '\n';
        }
        offset += batch.size();
    }
    if (offset <= 0) {
        file.close();
        emit exportError(path, tr("没有数据可导出"));
        return false;
    }
    return flushAndCheck(file, out, path);
}

// 流式导出变体格式(Timestamped/Bin/Json)见 DataExporterStreamedFormats.cpp
