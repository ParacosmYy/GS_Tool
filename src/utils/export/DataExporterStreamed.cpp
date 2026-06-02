/**
 * @file DataExporterStreamed.cpp
 * @brief 数据导出器 - 流式导出方法实现
 *
 * 流式导出通过 LineProvider 回调分批拉取数据，避免一次性将所有行加载到内存。
 * 支持 Plain/HexDump/CSV/Timestamped/Bin/Json 六种格式的流式变体，
 * 以及 exportStreamed() 统一分发入口。
 */

#include "utils/export/DataExporter.h"
#include "utils/crypto/HexConverter.h"
#include "core/theme/Constants.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

// ---- 流式导出入口 ----

/** @brief 导出数据到文件(流式模式，适合大数据量) @param filePath 目标路径 @param format 格式 @param lineProvider 行数据提供回调 @param totalLines 总行数 @param batchSize 每批行数 @return 是否成功 */
bool DataExporter::exportStreamed(const QString& filePath, Format format,
                                   LineProvider lineProvider,
                                   int totalLines, int batchSize)
{
    if (totalLines <= 0 || !lineProvider || filePath.isEmpty()) return false;

    switch (format) {
    case Plain:       return exportStreamedPlain(filePath, lineProvider, totalLines, batchSize);
    case HexDump:     return exportStreamedHexDump(filePath, lineProvider, totalLines, batchSize);
    case Csv:         return exportStreamedCsv(filePath, lineProvider, totalLines, batchSize);
    case Timestamped: return exportStreamedTimestamped(filePath, lineProvider, totalLines, batchSize);
    case Bin:         return exportStreamedBin(filePath, lineProvider, totalLines, batchSize);
    case Json:        return exportStreamedJson(filePath, lineProvider, totalLines, batchSize);
    default:
        emit exportError(filePath, tr("不支持的导出格式: %1").arg(static_cast<int>(format)));
        return false;
    }
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
    return flushAndCheck(file, out, path);
}

/** @brief 流式HexDump - 维护全局地址偏移和跨批次残余缓冲区 */
/** @brief 流式导出HEX转储格式 @param path 文件路径 @param provider 行数据提供回调 @param totalLines 总行数 @param batchSize 每批行数 @return 是否成功 */
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
    if (!residual.isEmpty())
        out << formatHexDumpLine(residual, globalAddr) << '\n';

    return flushAndCheck(file, out, path);
}

/** @brief 流式导出CSV格式 @param path 文件路径 @param provider 行数据提供回调 @param totalLines 总行数 @param batchSize 每批行数 @return 是否成功 */
bool DataExporter::exportStreamedCsv(const QString& path, LineProvider provider,
                                       int totalLines, int batchSize)
{
    QFile file(path);
    QTextStream out;
    if (!openTextFile(file, out, path)) return false;

    // UTF-8 BOM: 确保Excel中文环境下正确识别编码
    if (file.write("\xEF\xBB\xBF") != 3) {
        emit exportError(path, tr("写入BOM失败"));
        return false;
    }

    out << "timestamp,direction,data_hex,data_ascii\n";
    int offset = 0;
    while (offset < totalLines) {
        QVector<TerminalLine> batch = provider(offset, qMin(batchSize, totalLines - offset));
        if (batch.isEmpty()) break;
        for (const TerminalLine& line : batch) {
            out << line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz") << ','
                << ((line.direction == DataDirection::Rx) ? "RX" : "TX") << ','
                << HexConverter::toHexString(line.data) << ','
                << escapeCsvField(toAsciiString(line.data)) << '\n';
        }
        offset += batch.size();
    }
    return flushAndCheck(file, out, path);
}

/** @brief 流式导出带时间戳格式 @param path 文件路径 @param provider 行数据提供回调 @param totalLines 总行数 @param batchSize 每批行数 @return 是否成功 */
bool DataExporter::exportStreamedTimestamped(const QString& path, LineProvider provider,
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
            out << QString("[%1] %2\n")
                    .arg(line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz"),
                         HexConverter::toHexString(line.data));
        }
        offset += batch.size();
    }
    return flushAndCheck(file, out, path);
}

/** @brief 流式导出原始二进制格式 @param path 文件路径 @param provider 行数据提供回调 @param totalLines 总行数 @param batchSize 每批行数 @return 是否成功 */
bool DataExporter::exportStreamedBin(const QString& path, LineProvider provider,
                                       int totalLines, int batchSize)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        emit exportError(path, tr("无法打开文件: %1").arg(file.errorString()));
        return false;
    }
    int offset = 0;
    while (offset < totalLines) {
        QVector<TerminalLine> batch = provider(offset, qMin(batchSize, totalLines - offset));
        if (batch.isEmpty()) break;
        for (const TerminalLine& line : batch) {
            if (file.write(line.data) != line.data.size()) {
                emit exportError(path, tr("写入文件失败: %1").arg(file.errorString()));
                file.close();
                return false;
            }
        }
        offset += batch.size();
    }
    file.close();
    return true;
}

/**
 * @brief 流式JSON导出 - 分批构建JSON数组，适合大数据量场景
 *
 * 与exportJson输出格式相同，但通过LineProvider分批拉取数据，
 * 避免一次性将所有行加载到内存中。
 *
 * @param path 输出文件路径
 * @param provider 行数据回调
 * @param totalLines 数据总行数
 * @param batchSize 每批行数
 * @return true 成功，false 失败
 */
/** @brief 流式导出JSON格式(结构化数组) @param path 文件路径 @param provider 行数据提供回调 @param totalLines 总行数 @param batchSize 每批行数 @return 是否成功 */
bool DataExporter::exportStreamedJson(const QString& path, LineProvider provider,
                                       int totalLines, int batchSize)
{
    QJsonArray linesArray;
    int offset = 0;
    while (offset < totalLines) {
        QVector<TerminalLine> batch = provider(offset, qMin(batchSize, totalLines - offset));
        if (batch.isEmpty()) break;
        for (const TerminalLine& line : batch) {
            QJsonObject lineObj;
            lineObj["timestamp"] = line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz");
            lineObj["direction"] = (line.direction == DataDirection::Rx) ? "RX" : "TX";
            lineObj["hex"] = HexConverter::toHexString(line.data);
            lineObj["ascii"] = toAsciiString(line.data);
            linesArray.append(lineObj);
        }
        offset += batch.size();
    }

    QJsonObject root;
    root["export_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["total_lines"] = totalLines;
    root["lines"] = linesArray;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        emit exportError(path, tr("无法打开文件: %1").arg(file.errorString()));
        return false;
    }
    QJsonDocument doc(root);
    if (file.write(doc.toJson(QJsonDocument::Indented)) == -1) {
        emit exportError(path, tr("写入文件失败: %1").arg(file.errorString()));
        file.close();
        return false;
    }
    file.close();
    return true;
}
