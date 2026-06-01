/**
 * @file DataExporter.cpp
 * @brief 数据导出器实现 - Plain/HexDump/CSV/Timestamped/Bin 五种格式
 *
 * HexDump 格式将所有行数据拼接后按经典16字节/行输出。
 * 时间范围过滤仅在 exportToFile 模式下支持。
 * 所有写入操作均检查 QFile 错误状态，失败时发射 exportError 信号。
 */

#include "utils/DataExporter.h"
#include "utils/HexConverter.h"
#include "core/Constants.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDataStream>
#include <QTimeZone>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

DataExporter::DataExporter(QObject* parent) : QObject(parent) {}

// ---- 公共入口 ----

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
    }
    return false;
}

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
    }
    return false;
}

// ---- 辅助方法 ----

/** @brief 打开文本文件并设置UTF8编码，失败时发射exportError */
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

bool DataExporter::exportCsv(const QString& path, const QVector<TerminalLine>& lines)
{
    QFile file(path);
    QTextStream out;
    if (!openTextFile(file, out, path)) return false;

    // UTF-8 BOM: 确保Excel中文环境下正确识别编码
    file.write("\xEF\xBB\xBF");

    out << "timestamp,direction,data_hex,data_ascii\n";
    for (const TerminalLine& line : lines) {
        out << line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz") << ','
            << ((line.direction == DataDirection::Rx) ? "RX" : "TX") << ','
            << HexConverter::toHexString(line.data) << ','
            << escapeCsvField(toAsciiString(line.data)) << '\n';
    }
    return flushAndCheck(file, out, path);
}
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

QByteArray DataExporter::concatData(const QVector<TerminalLine>& lines)
{
    qsizetype totalSize = 0;
    for (const TerminalLine& line : lines) totalSize += line.data.size();

    QByteArray result;
    result.reserve(totalSize);
    for (const TerminalLine& line : lines) result.append(line.data);
    return result;
}

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

// ---- 流式导出方法 ----

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

bool DataExporter::exportStreamedCsv(const QString& path, LineProvider provider,
                                       int totalLines, int batchSize)
{
    QFile file(path);
    QTextStream out;
    if (!openTextFile(file, out, path)) return false;

    // UTF-8 BOM: 确保Excel中文环境下正确识别编码
    file.write("\xEF\xBB\xBF");

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

// ---- EDL范围导出 ----

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
