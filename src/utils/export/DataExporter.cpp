/**
 * @file DataExporter.cpp
 * @brief 数据导出器实现 - Plain/HexDump/CSV/Timestamped/Bin/Json 五种格式
 *
 * HexDump 格式将所有行数据拼接后按经典16字节/行输出。
 * 时间范围过滤仅在 exportToFile 模式下支持。
 * 所有写入操作均检查 QFile 错误状态，失败时发射 exportError 信号。
 *
 * 流式导出方法见 DataExporterStreamed.cpp
 * EDL范围导出方法见 DataExporterEdl.cpp
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

/** @brief 构造数据导出器 @param parent 父对象 */
DataExporter::DataExporter(QObject* parent) : QObject(parent) {}

// ---- 公共入口 ----

/** @brief 导出数据到文件(批量模式) @param filePath 目标文件路径 @param format 导出格式 @param lines 终端行数据 @param from 起始时间过滤 @param to 结束时间过滤 @return 是否成功 */
bool DataExporter::exportToFile(const QString& filePath, Format format,
                                 const QVector<TerminalLine>& lines,
                                 const QDateTime& from, const QDateTime& to)
{
    if (lines.isEmpty() || filePath.isEmpty()) return false;

    QVector<TerminalLine> filtered = filterByTime(lines, from, to);
    if (filtered.isEmpty()) return false;

    switch (format) {
    case Plain:       return exportPlain(filePath, filtered);
    case HexDump:     return exportHexDump(filePath, filtered);
    case Csv:         return exportCsv(filePath, filtered);
    case Timestamped: return exportTimestamped(filePath, filtered);
    case Bin:         return exportBin(filePath, filtered);
    case Json:        return exportJson(filePath, filtered);
    default:
        emit exportError(filePath, tr("不支持的导出格式: %1").arg(static_cast<int>(format)));
        return false;
    }
    return false;
}

// ---- 辅助方法 ----

/** @brief 打开文本文件并设置UTF8编码，失败时发射exportError */
/** @brief 打开文本文件用于写入 @param file 文件对象 @param out 文本流 @param path 文件路径 @return 是否成功 */
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

// ---- 全量导出方法 ----

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

/** @brief 导出CSV格式(时间戳,方向,数据) @param path 文件路径 @param lines 行数据 @return 是否成功 */
bool DataExporter::exportCsv(const QString& path, const QVector<TerminalLine>& lines)
{
    QFile file(path);
    QTextStream out;
    if (!openTextFile(file, out, path)) return false;

    // UTF-8 BOM: 确保Excel中文环境下正确识别编码
    if (file.write("\xEF\xBB\xBF") != 3) {
        emit exportError(path, tr("写入BOM失败"));
        file.close();
        return false;
    }

    out << "timestamp,direction,data_hex,data_ascii\n";
    for (const TerminalLine& line : lines) {
        out << line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz") << ','
            << ((line.direction == DataDirection::Rx) ? "RX" : "TX") << ','
            << HexConverter::toHexString(line.data) << ','
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

/**
 * @brief JSON全量导出 - 生成结构化JSON文件
 *
 * 输出格式:
 * {
 *   "export_time": "2026-06-01T12:00:00",
 *   "total_lines": 100,
 *   "lines": [
 *     {"timestamp": "2026-06-01 12:00:00.123", "direction": "RX", "hex": "48656C6C6F", "ascii": "Hello"},
 *     ...
 *   ]
 * }
 *
 * @param path 输出文件路径
 * @param lines 过滤后的行数据
 * @return true 成功，false 失败
 */
/** @brief 导出JSON格式(结构化数据数组) @param path 文件路径 @param lines 行数据 @return 是否成功 */
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

// ---- 静态辅助方法 ----

/** @brief 将字节数据转换为可打印ASCII字符串(不可打印字符替换为'.') @param data 原始字节 @return ASCII字符串 */
QString DataExporter::toAsciiString(const QByteArray& data)
{
    if (data.isEmpty()) return QString();
    QString result;
    result.reserve(data.size());
    const char* ptr = data.constData();
    for (int i = 0; i < data.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(ptr[i]);
        result += (ch >= 0x20 && ch <= 0x7E) ? QLatin1Char(ch) : QLatin1Char('.');
    }
    return result;
}

/** @brief 转义CSV字段中的特殊字符(逗号、引号、换行) @param field 原始字段 @return 转义后的字段 */
QString DataExporter::escapeCsvField(const QString& field)
{
    // RFC 4180: 字段含逗号、双引号或换行时，用双引号包裹，内部双引号翻倍
    if (!field.contains(QLatin1Char(',')) &&
        !field.contains(QLatin1Char('"')) &&
        !field.contains(QLatin1Char('\n'))) {
        return field;
    }
    QString escaped = field;
    escaped.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    return QLatin1Char('"') + escaped + QLatin1Char('"');
}

/** @brief 将多行数据拼接为单个QByteArray(预分配总大小避免反复重分配) @param lines 终端行列表 @return 拼接后的原始字节 */
QByteArray DataExporter::concatData(const QVector<TerminalLine>& lines)
{
    qsizetype totalSize = 0;
    for (const TerminalLine& line : lines) totalSize += line.data.size();

    QByteArray result;
    result.reserve(totalSize);
    for (const TerminalLine& line : lines) result.append(line.data);
    return result;
}

/** @brief 格式化单行HEX转储(地址+HEX+ASCII) @param data 原始字节 @param address 起始地址 @return 格式化的HEX转储行 */
QString DataExporter::formatHexDumpLine(const QByteArray& data, quint64 address)
{
    const int bytesPerLine = 16;
    QString addrStr = QString("%1").arg(address, 8, 16, QChar('0')).toUpper();

    QString hexPart;
    hexPart.reserve(bytesPerLine * 3 + 2);
    for (int i = 0; i < bytesPerLine; ++i) {
        if (i > 0) hexPart += ' ';
        if (i == 8) hexPart += ' ';
        if (i < data.size()) {
            hexPart += QString("%1").arg(static_cast<unsigned char>(data[i]), 2, 16, QChar('0')).toUpper();
        } else {
            hexPart += "  ";
        }
    }
    return QString("%1 | %2 | %3").arg(addrStr, hexPart, toAsciiString(data));
}
