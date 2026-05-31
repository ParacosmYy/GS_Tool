#include "utils/DataExporter.h"
#include "utils/HexConverter.h"
#include "core/Constants.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>

DataExporter::DataExporter(QObject* parent)
    : QObject(parent)
{
}

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
    case Txt:  return exportTxt(filePath, filtered);
    case Csv:  return exportCsv(filePath, filtered);
    case Bin:  return exportBin(filePath, filtered);
    }
    return false;
}

QVector<TerminalLine> DataExporter::filterByTime(
    const QVector<TerminalLine>& lines,
    const QDateTime& from,
    const QDateTime& to) const
{
    bool hasFrom = from.isValid();
    bool hasTo = to.isValid();

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

bool DataExporter::exportTxt(const QString& path,
                              const QVector<TerminalLine>& lines)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    for (const TerminalLine& line : lines) {
        QString timeStr = line.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz");
        QString dirStr = (line.direction == DataDirection::Rx) ? "RX" : "TX";
        // 复用 HexConverter 公共组件，不重复实现
        QString hex = HexConverter::toHexString(line.data);
        QString ascii = toAsciiString(line.data);

        out << QString("[%1] [%2] %3 | %4\n").arg(timeStr, dirStr, hex, ascii);
    }

    file.close();
    return true;
}

bool DataExporter::exportCsv(const QString& path,
                              const QVector<TerminalLine>& lines)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

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

bool DataExporter::exportBin(const QString& path,
                              const QVector<TerminalLine>& lines)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    for (const TerminalLine& line : lines) {
        file.write(line.data);
    }

    file.close();
    return true;
}

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

// ---- 批量流式导出 ----

bool DataExporter::exportStreamed(const QString& filePath, Format format,
                                   LineProvider lineProvider,
                                   int totalLines, int batchSize)
{
    if (totalLines <= 0 || !lineProvider || filePath.isEmpty()) {
        return false;
    }

    switch (format) {
    case Txt:  return exportStreamedTxt(filePath, lineProvider, totalLines, batchSize);
    case Csv:  return exportStreamedCsv(filePath, lineProvider, totalLines, batchSize);
    case Bin:  return exportStreamedBin(filePath, lineProvider, totalLines, batchSize);
    }
    return false;
}

bool DataExporter::exportStreamedTxt(const QString& path, LineProvider provider,
                                      int totalLines, int batchSize)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

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

bool DataExporter::exportStreamedCsv(const QString& path, LineProvider provider,
                                      int totalLines, int batchSize)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    // 写入 CSV 表头
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

bool DataExporter::exportStreamedBin(const QString& path, LineProvider provider,
                                      int totalLines, int batchSize)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

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
