/**
 * @file DataExporter.cpp
 * @brief 数据导出器实现 - Plain/HexDump/CSV/Timestamped/Bin 五种格式
 *
 * HexDump 格式将所有行数据拼接后按经典16字节/行输出。
 * 时间范围过滤仅在 exportToFile 模式下支持。
 * 所有写入操作均检查 QFile 错误状态，失败时发射 exportError 信号通知调用方。
 */

#include "utils/DataExporter.h"
#include "utils/HexConverter.h"
#include "core/Constants.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>

// ---- 构造函数 ----

DataExporter::DataExporter(QObject* parent) : QObject(parent) {}

// ---- 全量导出入口 ----

/** @brief 全量导出 - 输入校验 → 时间过滤 → 格式分发 */
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
    }
    return false;
}

// ---- 流式导出入口 ----

/** @brief 流式导出 - 分批拉取数据，不支持时间过滤 */
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
    }
    return false;
}

// ---- 文件写入错误检查 ----

/**
 * @brief 刷新文本流并检查文件写入错误
 * @param file 文件对象（已打开）
 * @param out  文本流
 * @param path 文件路径（用于错误信号）
 * @return true=写入成功，false=写入失败（已发射 exportError 信号）
 */
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

// ---- 时间范围过滤 ----

/** @brief from/to 均可选，都无效时返回原始数据 */
QVector<TerminalLine> DataExporter::filterByTime(
    const QVector<TerminalLine>& lines, const QDateTime& from, const QDateTime& to) const
{
    bool hasFrom = from.isValid();
    bool hasTo = to.isValid();
    if (!hasFrom && !hasTo) return lines; // 避免不必要的拷贝

    QVector<TerminalLine> result;
    result.reserve(lines.size());
    for (const TerminalLine& line : lines) {
        if (hasFrom && line.timestamp < from) continue;
        if (hasTo && line.timestamp > to) continue;
        result.append(line);
    }
    return result;
}

// ============================================================
// 全量导出方法
// ============================================================

/** @brief 纯文本导出: [时间戳] [方向] HEX | ASCII */
bool DataExporter::exportPlain(const QString& path, const QVector<TerminalLine>& lines)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit exportError(path, tr("无法打开文件: %1").arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    for (const TerminalLine& line : lines) {
        QString timeStr = line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz");
        QString dirStr = (line.direction == DataDirection::Rx) ? "RX" : "TX";
        out << QString("[%1] [%2] %3 | %4\n")
                .arg(timeStr, dirStr, HexConverter::toHexString(line.data), toAsciiString(line.data));
    }
    return flushAndCheck(file, out, path);
}

/** @brief 十六进制转储 - 经典格式: 地址 | HEX(16字节/行) | ASCII */
bool DataExporter::exportHexDump(const QString& path, const QVector<TerminalLine>& lines)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit exportError(path, tr("无法打开文件: %1").arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    QByteArray allData = concatData(lines);
    if (allData.isEmpty()) {
        emit exportError(path, tr("无有效数据可导出"));
        file.close();
        return false;
    }

    const int bytesPerLine = 16;
    for (int offset = 0; offset < allData.size(); offset += bytesPerLine) {
        QByteArray chunk = allData.mid(offset, qMin(bytesPerLine, allData.size() - offset));
        out << formatHexDumpLine(chunk, static_cast<quint64>(offset)) << '\n';
    }
    return flushAndCheck(file, out, path);
}

/** @brief CSV导出: 表头 + 逗号分隔，ASCII字段用双引号包裹 */
bool DataExporter::exportCsv(const QString& path, const QVector<TerminalLine>& lines)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit exportError(path, tr("无法打开文件: %1").arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << "timestamp,direction,data_hex,data_ascii\n";

    for (const TerminalLine& line : lines) {
        QString timeStr = line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz");
        QString dirStr = (line.direction == DataDirection::Rx) ? "RX" : "TX";
        out << timeStr << ',' << dirStr << ',' << HexConverter::toHexString(line.data)
            << ",\"" << toAsciiString(line.data) << "\"\n";
    }
    return flushAndCheck(file, out, path);
}

/** @brief 带时间戳导出: [时间戳] HEX */
bool DataExporter::exportTimestamped(const QString& path, const QVector<TerminalLine>& lines)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit exportError(path, tr("无法打开文件: %1").arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    for (const TerminalLine& line : lines) {
        out << QString("[%1] %2\n")
                .arg(line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz"),
                     HexConverter::toHexString(line.data));
    }
    return flushAndCheck(file, out, path);
}

/** @brief 二进制导出: 仅写入原始字节，逐行检查 write() 返回值 */
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

// ============================================================
// 静态辅助方法
// ============================================================

/** @brief 不可打印字符(0x00~0x1F, 0x7F~0xFF)替换为 '.' */
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

/** @brief 拼接所有行数据为连续字节数组（预计算大小减少重新分配） */
QByteArray DataExporter::concatData(const QVector<TerminalLine>& lines)
{
    qsizetype totalSize = 0;
    for (const TerminalLine& line : lines) totalSize += line.data.size();

    QByteArray result;
    result.reserve(totalSize);
    for (const TerminalLine& line : lines) result.append(line.data);
    return result;
}

/**
 * @brief 格式化单行 HexDump
 * 格式: "XXXXXXXX | XX XX ... XX | ................"，每8字节间加额外空格
 */
QString DataExporter::formatHexDumpLine(const QByteArray& data, quint64 address)
{
    const int bytesPerLine = 16;
    QString addrStr = QString("%1").arg(address, 8, 16, QChar('0')).toUpper();

    QString hexPart;
    hexPart.reserve(bytesPerLine * 3 + 2);
    for (int i = 0; i < bytesPerLine; ++i) {
        if (i > 0) hexPart += ' ';
        if (i == 8) hexPart += ' '; // 第8字节后额外空格
        if (i < data.size()) {
            hexPart += QString("%1").arg(static_cast<unsigned char>(data[i]), 2, 16, QChar('0')).toUpper();
        } else {
            hexPart += "  "; // 不足16字节空格补齐
        }
    }
    return QString("%1 | %2 | %3").arg(addrStr, hexPart, toAsciiString(data));
}

// ============================================================
// 流式导出方法
// ============================================================

/** @brief 流式纯文本导出 */
bool DataExporter::exportStreamedPlain(const QString& path, LineProvider provider,
                                        int totalLines, int batchSize)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit exportError(path, tr("无法打开文件: %1").arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

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

/**
 * @brief 流式HexDump导出 - 维护全局地址偏移和跨批次残余缓冲区
 * 每批拼接后按16字节宽度格式化，不足部分留到下一批。
 */
bool DataExporter::exportStreamedHexDump(const QString& path, LineProvider provider,
                                          int totalLines, int batchSize)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit exportError(path, tr("无法打开文件: %1").arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

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

/** @brief 流式CSV导出 - 先写表头再分批写入数据行 */
bool DataExporter::exportStreamedCsv(const QString& path, LineProvider provider,
                                      int totalLines, int batchSize)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit exportError(path, tr("无法打开文件: %1").arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << "timestamp,direction,data_hex,data_ascii\n";

    int offset = 0;
    while (offset < totalLines) {
        QVector<TerminalLine> batch = provider(offset, qMin(batchSize, totalLines - offset));
        if (batch.isEmpty()) break;
        for (const TerminalLine& line : batch) {
            out << line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz") << ','
                << ((line.direction == DataDirection::Rx) ? "RX" : "TX") << ','
                << HexConverter::toHexString(line.data) << ",\"" << toAsciiString(line.data) << "\"\n";
        }
        offset += batch.size();
    }
    return flushAndCheck(file, out, path);
}

/** @brief 流式带时间戳导出 */
bool DataExporter::exportStreamedTimestamped(const QString& path, LineProvider provider,
                                              int totalLines, int batchSize)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit exportError(path, tr("无法打开文件: %1").arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

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

/** @brief 流式二进制导出 - 分批写入原始字节，逐行检查 write() 返回值 */
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
