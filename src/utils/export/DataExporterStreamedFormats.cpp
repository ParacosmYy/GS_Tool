/**
 * @file DataExporterStreamedFormats.cpp
 * @brief 数据导出器 - 流式导出变体格式实现(Bin/JSON/Timestamped)
 *
 * 从DataExporterStreamed.cpp拆分而来，包含三种流式导出格式:
 *   - exportStreamedTimestamped(): 带时间戳格式流式导出
 *   - exportStreamedBin():         原始二进制格式流式导出
 *   - exportStreamedJson():        JSON格式流式导出(分批构建JSON数组)
 *
 * 统一分发入口与其他格式(Plain/HexDump/CSV)见DataExporterStreamed.cpp。
 */

#include "utils/export/DataExporter.h"
#include "utils/crypto/HexConverter.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

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
    if (offset <= 0) {
        file.close();
        emit exportError(path, tr("没有数据可导出"));
        return false;
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
    if (offset <= 0) {
        file.close();
        emit exportError(path, tr("没有数据可导出"));
        return false;
    }
    file.close();
    return true;
}

/** @brief 流式导出JSON格式(分批构建JSON数组，适合大数据量场景) @param path 输出文件路径 @param provider 行数据回调 @param totalLines 数据总行数 @param batchSize 每批行数 @return 是否成功 */
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
    if (offset <= 0) {
        emit exportError(path, tr("没有数据可导出"));
        return false;
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
