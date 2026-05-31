/**
 * @file DataExporter.cpp
 * @brief 数据导出器实现 - Plain/HexDump/CSV/Timestamped/Bin 五种格式
 *
 * HexDump 格式将所有行数据拼接后按经典16字节/行输出。
 * 时间范围过滤仅在 exportToFile 模式下支持。
 */

#include "utils/DataExporter.h"
#include "utils/HexConverter.h"
#include "core/Constants.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>

// ---- 构造函数 ----

DataExporter::DataExporter(QObject* parent)
    : QObject(parent)
{
}

// ---- 全量导出入口 ----

/**
 * @brief 全量导出 - 输入校验 → 时间过滤 → 格式分发
 * @return true 成功，false 失败（空数据/文件无法打开）
 */
bool DataExporter::exportToFile(const QString& filePath, Format format,
                                 const QVector<TerminalLine>& lines,
                                 const QDateTime& from, const QDateTime& to)
{
    if (lines.isEmpty() || filePath.isEmpty()) {
        return false;
    }

    QVector<TerminalLine> filtered = filterByTime(lines, from, to);
    if (filtered.isEmpty()) {
        return false;
    }

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

/**
 * @brief 流式导出 - 分批拉取数据，不支持时间过滤
 * @return true 成功，false 失败
 */
bool DataExporter::exportStreamed(const QString& filePath, Format format,
                                   LineProvider lineProvider,
                                   int totalLines, int batchSize)
{
    if (totalLines <= 0 || !lineProvider || filePath.isEmpty()) {
        return false;
    }

    switch (format) {
    case Plain:       return exportStreamedPlain(filePath, lineProvider, totalLines, batchSize);
    case HexDump:     return exportStreamedHexDump(filePath, lineProvider, totalLines, batchSize);
    case Csv:         return exportStreamedCsv(filePath, lineProvider, totalLines, batchSize);
    case Timestamped: return exportStreamedTimestamped(filePath, lineProvider, totalLines, batchSize);
    case Bin:         return exportStreamedBin(filePath, lineProvider, totalLines, batchSize);
    }
    return false;
}

// ---- 时间范围过滤 ----

/** @brief from/to 均可选，都无效时返回原始数据 */
QVector<TerminalLine> DataExporter::filterByTime(
    const QVector<TerminalLine>& lines,
    const QDateTime& from,
    const QDateTime& to) const
{
    bool hasFrom = from.isValid();
    bool hasTo = to.isValid();

    // 两者都无效时直接返回原始数据，避免不必要的拷贝
    if (!hasFrom && !hasTo) {
        return lines;
    }

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

/** @brief 纯文本: [时间戳] [方向] HEX | ASCII */
bool DataExporter::exportPlain(const QString& path,
                                const QVector<TerminalLine>& lines)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    for (const TerminalLine& line : lines) {
        QString timeStr = line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz");
        QString dirStr = (line.direction == DataDirection::Rx) ? "RX" : "TX";
        QString hex = HexConverter::toHexString(line.data);
        QString ascii = toAsciiString(line.data);
        out << QString("[%1] [%2] %3 | %4\n").arg(timeStr, dirStr, hex, ascii);
    }

    file.close();
    return true;
}

/**
 * @brief 十六进制转储 - 经典格式
 *
 * 输出示例:
 *   00000000 | AA BB CC DD EE FF 00 11  22 33 44 55 66 77 88 99 | ................
 *   00000010 | AA BB CC DD EE                                         | .....
 *
 * 左侧: 8位地址 | 中间: 16字节HEX(每8字节额外空格) | 右侧: ASCII
 */
bool DataExporter::exportHexDump(const QString& path,
                                  const QVector<TerminalLine>& lines)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    // 拼接所有行数据为连续字节流
    QByteArray allData = concatData(lines);
    if (allData.isEmpty()) {
        file.close();
        return false;
    }

    // 每16字节输出一行
    const int bytesPerLine = 16;
    int offset = 0;
    while (offset < allData.size()) {
        int chunkSize = qMin(bytesPerLine, allData.size() - offset);
        QByteArray chunk = allData.mid(offset, chunkSize);
        out << formatHexDumpLine(chunk, static_cast<quint64>(offset)) << '\n';
        offset += bytesPerLine;
    }

    file.close();
    return true;
}

/** @brief CSV表格: 表头 + 逗号分隔，ASCII字段用双引号包裹 */
bool DataExporter::exportCsv(const QString& path,
                              const QVector<TerminalLine>& lines)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    // CSV 表头
    out << "timestamp,direction,data_hex,data_ascii\n";

    for (const TerminalLine& line : lines) {
        QString timeStr = line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz");
        QString dirStr = (line.direction == DataDirection::Rx) ? "RX" : "TX";
        QString hex = HexConverter::toHexString(line.data);
        QString ascii = toAsciiString(line.data);

        out << timeStr << ',' << dirStr << ',' << hex << ','
            << '"' << ascii << '"' << '\n';
    }

    file.close();
    return true;
}

/** @brief 带时间戳: [时间戳] HEX，不包含方向标识和ASCII列 */
bool DataExporter::exportTimestamped(const QString& path,
                                      const QVector<TerminalLine>& lines)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    for (const TerminalLine& line : lines) {
        QString timeStr = line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz");
        QString hex = HexConverter::toHexString(line.data);
        out << QString("[%1] %2\n").arg(timeStr, hex);
    }

    file.close();
    return true;
}

/** @brief 二进制: 仅写入原始字节 */
bool DataExporter::exportBin(const QString& path,
                              const QVector<TerminalLine>& lines)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return false;

    for (const TerminalLine& line : lines) {
        file.write(line.data);
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
    for (const TerminalLine& line : lines) {
        totalSize += line.data.size();
    }

    QByteArray result;
    result.reserve(totalSize);

    for (const TerminalLine& line : lines) {
        result.append(line.data);
    }

    return result;
}

/**
 * @brief 格式化单行 HexDump
 *
 * 格式: "XXXXXXXX | XX XX XX XX XX XX XX XX  XX XX XX XX XX XX XX XX | ................"
 * 每8字节间加额外空格增强可读性，不足16字节用空格补齐
 */
QString DataExporter::formatHexDumpLine(const QByteArray& data, quint64 address)
{
    const int bytesPerLine = 16;

    // 地址: 8位十六进制
    QString addrStr = QString("%1").arg(address, 8, 16, QChar('0')).toUpper();

    // HEX 部分: 每8字节间加额外空格
    QString hexPart;
    hexPart.reserve(bytesPerLine * 3 + 2);
    for (int i = 0; i < bytesPerLine; ++i) {
        if (i > 0) {
            hexPart += ' ';
            if (i == 8) hexPart += ' ';  // 第8字节后额外空格
        }
        if (i < data.size()) {
            unsigned char byte = static_cast<unsigned char>(data[i]);
            hexPart += QString("%1").arg(byte, 2, 16, QChar('0')).toUpper();
        } else {
            hexPart += "  ";  // 不足16字节用空格补齐
        }
    }

    // ASCII 部分
    QString asciiPart = toAsciiString(data);

    return QString("%1 | %2 | %3").arg(addrStr, hexPart, asciiPart);
}

// ============================================================
// 流式导出方法
// ============================================================

/** @brief 流式纯文本导出 */
bool DataExporter::exportStreamedPlain(const QString& path, LineProvider provider,
                                        int totalLines, int batchSize)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    int offset = 0;
    while (offset < totalLines) {
        int count = qMin(batchSize, totalLines - offset);
        QVector<TerminalLine> batch = provider(offset, count);
        if (batch.isEmpty()) break;

        for (const TerminalLine& line : batch) {
            QString timeStr = line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz");
            QString dirStr = (line.direction == DataDirection::Rx) ? "RX" : "TX";
            QString hex = HexConverter::toHexString(line.data);
            QString ascii = toAsciiString(line.data);
            out << QString("[%1] [%2] %3 | %4\n").arg(timeStr, dirStr, hex, ascii);
        }

        offset += batch.size();
    }

    file.close();
    return true;
}

/**
 * @brief 流式HexDump导出 - 维护全局地址偏移量和跨批次残余缓冲区
 *
 * 每批数据拼接后按16字节宽度格式化，不足部分留到下一批拼接。
 */
bool DataExporter::exportStreamedHexDump(const QString& path, LineProvider provider,
                                          int totalLines, int batchSize)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    const int bytesPerLine = 16;
    QByteArray residual;       // 跨批次残余数据
    quint64 globalAddress = 0; // 全局地址偏移

    int offset = 0;
    while (offset < totalLines) {
        int count = qMin(batchSize, totalLines - offset);
        QVector<TerminalLine> batch = provider(offset, count);
        if (batch.isEmpty()) break;

        // 拼接残余数据和本批次数据
        QByteArray batchData = residual + concatData(batch);

        // 按16字节宽度输出完整的行
        int pos = 0;
        while (pos + bytesPerLine <= batchData.size()) {
            QByteArray chunk = batchData.mid(pos, bytesPerLine);
            out << formatHexDumpLine(chunk, globalAddress) << '\n';
            pos += bytesPerLine;
            globalAddress += bytesPerLine;
        }

        // 保留不足16字节的残余数据
        residual = batchData.mid(pos);
        offset += batch.size();
    }

    // 输出最后的残余行
    if (!residual.isEmpty()) {
        out << formatHexDumpLine(residual, globalAddress) << '\n';
    }

    file.close();
    return true;
}

/** @brief 流式CSV导出 - 先写表头再分批写入数据行 */
bool DataExporter::exportStreamedCsv(const QString& path, LineProvider provider,
                                      int totalLines, int batchSize)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    // CSV 表头
    out << "timestamp,direction,data_hex,data_ascii\n";

    int offset = 0;
    while (offset < totalLines) {
        int count = qMin(batchSize, totalLines - offset);
        QVector<TerminalLine> batch = provider(offset, count);
        if (batch.isEmpty()) break;

        for (const TerminalLine& line : batch) {
            QString timeStr = line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz");
            QString dirStr = (line.direction == DataDirection::Rx) ? "RX" : "TX";
            QString hex = HexConverter::toHexString(line.data);
            QString ascii = toAsciiString(line.data);

            out << timeStr << ',' << dirStr << ',' << hex << ','
                << '"' << ascii << '"' << '\n';
        }

        offset += batch.size();
    }

    file.close();
    return true;
}

/** @brief 流式带时间戳导出 */
bool DataExporter::exportStreamedTimestamped(const QString& path, LineProvider provider,
                                              int totalLines, int batchSize)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    int offset = 0;
    while (offset < totalLines) {
        int count = qMin(batchSize, totalLines - offset);
        QVector<TerminalLine> batch = provider(offset, count);
        if (batch.isEmpty()) break;

        for (const TerminalLine& line : batch) {
            QString timeStr = line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz");
            QString hex = HexConverter::toHexString(line.data);
            out << QString("[%1] %2\n").arg(timeStr, hex);
        }

        offset += batch.size();
    }

    file.close();
    return true;
}

/** @brief 流式二进制导出 - 分批写入原始字节 */
bool DataExporter::exportStreamedBin(const QString& path, LineProvider provider,
                                      int totalLines, int batchSize)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return false;

    int offset = 0;
    while (offset < totalLines) {
        int count = qMin(batchSize, totalLines - offset);
        QVector<TerminalLine> batch = provider(offset, count);
        if (batch.isEmpty()) break;

        for (const TerminalLine& line : batch) {
            file.write(line.data);
        }

        offset += batch.size();
    }

    file.close();
    return true;
}
