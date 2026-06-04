/**
 * @file DataExporterFormats.cpp
 * @brief 数据导出器 -- 全量导出格式方法实现(Plain/HexDump/CSV/Timestamped/Bin/Json)
 *
 * 本文件从 DataExporter.cpp 拆分而来，包含六种导出格式的批量导出方法:
 *   - exportPlain:       纯文本格式([时间] [方向] HEX | ASCII)
 *   - exportHexDump:     HEX转储格式(地址+HEX+ASCII，16字节/行)
 *   - exportCsv:         CSV格式(BOM头+可配置分隔符+时间戳+方向+数据)
 *   - exportTimestamped: 带时间戳格式(ISO时间 [方向] 数据)
 *   - exportBin:         原始二进制格式(仅数据字节)
 *   - exportJson:        JSON格式(结构化数据数组)
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

// ============================================================================
// 全量导出方法
// ============================================================================

/** @brief 导出纯文本格式 @param path 文件路径 @param lines 行数据 @return 是否成功 */
bool DataExporter::exportPlain(const QString& path, const QVector<TerminalLine>& lines)
{
    QFile file(path);
    QTextStream out;
    if (!openTextFile(file, out, path)) return false;

    for (const TerminalLine& line : lines) {
        out << QString("[%1] [%2] %3 | %4\n")
                .arg(line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz"),
                     (line.direction == DataDirection::Rx) ? "RX" : "TX",
                     HexConverter::toHexString(line.data), toAsciiString(line.data));
    }
    return flushAndCheck(file, out, path);
}

/** @brief 导出HEX转储格式(地址+HEX+ASCII) @param path 文件路径 @param lines 行数据 @return 是否成功 */
bool DataExporter::exportHexDump(const QString& path, const QVector<TerminalLine>& lines)
{
    QFile file(path);
    QTextStream out;
    if (!openTextFile(file, out, path)) return false;

    QByteArray allData = concatData(lines);
    if (allData.isEmpty()) {
        emit exportError(path, tr("无有效数据可导出"));
        file.close();
        return false;
    }

    const int bytesPerLine = 16;
    for (int offset = 0; offset < allData.size(); offset += bytesPerLine) {
        out << formatHexDumpLine(allData.mid(offset, qMin(bytesPerLine, allData.size() - offset)),
                                 static_cast<quint64>(offset)) << '\n';
    }
    return flushAndCheck(file, out, path);
}

/** @brief 导出CSV格式(BOM头+可配置分隔符+时间戳+方向+数据) @param path 文件路径 @param lines 行数据 @return 是否成功 */
bool DataExporter::exportCsv(const QString& path, const QVector<TerminalLine>& lines)
{
    QFile file(path);
    QTextStream out;
    if (!openTextFile(file, out, path)) return false;

    // UTF-8 BOM: 确保Excel中文环境下正确识别编码(可通过setCsvBomEnabled关闭)
    if (!writeCsvBom(file, path)) {
        file.close();
        return false;
    }

    out << csvHeader() << '\n';
    for (const TerminalLine& line : lines) {
        out << line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz") << m_csvDelimiter
            << ((line.direction == DataDirection::Rx) ? "RX" : "TX") << m_csvDelimiter
            << HexConverter::toHexString(line.data) << m_csvDelimiter
            << escapeCsvField(toAsciiString(line.data)) << '\n';
    }
    return flushAndCheck(file, out, path);
}

/** @brief 导出带时间戳格式(ISO时间 [方向] 数据) @param path 文件路径 @param lines 行数据 @return 是否成功 */
bool DataExporter::exportTimestamped(const QString& path, const QVector<TerminalLine>& lines)
{
    QFile file(path);
    QTextStream out;
    if (!openTextFile(file, out, path)) return false;

    for (const TerminalLine& line : lines) {
        out << QString("[%1] %2\n")
                .arg(line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz"),
                     HexConverter::toHexString(line.data));
    }
    return flushAndCheck(file, out, path);
}

/** @brief 导出原始二进制格式(仅数据字节，无时间戳) @param path 文件路径 @param lines 行数据 @return 是否成功 */
bool DataExporter::exportBin(const QString& path, const QVector<TerminalLine>& lines)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        emit exportError(path, tr("无法打开文件: %1").arg(file.errorString()));
        return false;
    }
    for (const TerminalLine& line : lines) {
        if (file.write(line.data) != line.data.size()) {
            emit exportError(path, tr("写入文件失败: %1").arg(file.errorString()));
            file.close();
            return false;
        }
    }
    file.close();
    return true;
}

/** @brief 导出JSON格式(结构化数据数组，含export_time/total_lines/lines字段) @param path 文件路径 @param lines 行数据 @return 是否成功 */
bool DataExporter::exportJson(const QString& path, const QVector<TerminalLine>& lines)
{
    QJsonObject root;
    root["export_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["total_lines"] = lines.size();

    QJsonArray linesArray;
    for (const TerminalLine& line : lines) {
        QJsonObject lineObj;
        lineObj["timestamp"] = line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz");
        lineObj["direction"] = (line.direction == DataDirection::Rx) ? "RX" : "TX";
        lineObj["hex"] = HexConverter::toHexString(line.data);
        lineObj["ascii"] = toAsciiString(line.data);
        linesArray.append(lineObj);
    }
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
