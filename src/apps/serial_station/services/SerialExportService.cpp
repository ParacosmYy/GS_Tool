#include "apps/serial_station/services/SerialExportService.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QIODevice>
#include <QtCore/QSaveFile>
#include <QtCore/QStringList>

namespace serial_station {

namespace {

QString timestampText(const QDateTime& timestamp)
{
    return timestamp.toString(Qt::ISODateWithMs);
}

QString joinedLines(const QStringList& lines)
{
    if (lines.isEmpty()) {
        return QString();
    }

    return lines.join(QLatin1Char('\n'));
}

} // namespace

QString SerialExportService::formatRecords(const QVector<SerialLogRecord>& records,
                                           SerialExportFormat format) const
{
    if (format == SerialExportFormat::PlainText) {
        return formatPlainText(records);
    }

    if (format == SerialExportFormat::JsonLines) {
        return formatJsonLines(records);
    }

    if (format == SerialExportFormat::Csv) {
        return formatCsv(records);
    }

    return QString();
}

SerialExportResult SerialExportService::exportRecords(const QVector<SerialLogRecord>& records,
                                                      const SerialExportRequest& request) const
{
    if (!isSupportedFormat(request.format)) {
        return makeFailure(request, QStringLiteral("导出格式不受支持"));
    }

    if (request.filePath.trimmed().isEmpty()) {
        return makeFailure(request, QStringLiteral("导出路径为空"));
    }

    if (records.isEmpty()) {
        return makeFailure(request, QStringLiteral("没有可导出的日志记录"));
    }

    const QFileInfo fileInfo(request.filePath);
    const QDir parentDir = fileInfo.absoluteDir();
    if (!parentDir.exists()) {
        return makeFailure(request, QStringLiteral("导出目录不存在: %1").arg(parentDir.absolutePath()));
    }

    const QString text = formatRecords(records, request.format);
    if (text.isEmpty()) {
        return makeFailure(request, QStringLiteral("导出内容为空"));
    }

    const QByteArray payload = encodedPayload(text, request.writeUtf8Bom);
    QSaveFile file(fileInfo.absoluteFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return makeFailure(request, QStringLiteral("无法打开导出文件: %1").arg(file.errorString()));
    }

    const qint64 written = file.write(payload);
    if (written != payload.size()) {
        return makeFailure(request, QStringLiteral("导出文件写入不完整"));
    }

    if (!file.commit()) {
        return makeFailure(request, QStringLiteral("导出文件提交失败: %1").arg(file.errorString()));
    }

    return makeSuccess(request, written);
}

QString SerialExportService::formatName(SerialExportFormat format) const
{
    if (format == SerialExportFormat::PlainText) {
        return QStringLiteral("plain_text");
    }

    if (format == SerialExportFormat::JsonLines) {
        return QStringLiteral("json_lines");
    }

    if (format == SerialExportFormat::Csv) {
        return QStringLiteral("csv");
    }

    return QStringLiteral("unsupported");
}

QString SerialExportService::defaultSuffix(SerialExportFormat format) const
{
    if (format == SerialExportFormat::PlainText) {
        return QStringLiteral("txt");
    }

    if (format == SerialExportFormat::JsonLines) {
        return QStringLiteral("jsonl");
    }

    if (format == SerialExportFormat::Csv) {
        return QStringLiteral("csv");
    }

    return QString();
}

QString SerialExportService::formatPlainText(const QVector<SerialLogRecord>& records) const
{
    SerialLogService logService(records.count());
    for (const SerialLogRecord& record : records) {
        logService.append(record);
    }

    return logService.toPlainText();
}

QString SerialExportService::formatJsonLines(const QVector<SerialLogRecord>& records) const
{
    SerialLogService logService(records.count());
    for (const SerialLogRecord& record : records) {
        logService.append(record);
    }

    return logService.toJsonLines();
}

QString SerialExportService::formatCsv(const QVector<SerialLogRecord>& records) const
{
    QStringList lines;
    lines.reserve(records.count() + 1);
    lines.append(QStringLiteral("timestamp,direction,source,text,payloadHex"));

    for (const SerialLogRecord& record : records) {
        lines.append(csvLine(record));
    }

    return joinedLines(lines);
}

QString SerialExportService::csvLine(const SerialLogRecord& record) const
{
    QStringList columns;
    columns.reserve(5);
    columns.append(csvEscape(timestampText(record.timestamp)));
    columns.append(csvEscape(SerialLogService::directionName(record.direction)));
    columns.append(csvEscape(record.source));
    columns.append(csvEscape(record.text));
    columns.append(csvEscape(SerialLogService::payloadHex(record.payload)));
    return columns.join(QLatin1Char(','));
}

QString SerialExportService::csvEscape(const QString& value) const
{
    QString escaped = value;
    escaped.replace(QStringLiteral("\""), QStringLiteral("\"\""));

    const bool needsQuotes = escaped.contains(QLatin1Char(',')) ||
        escaped.contains(QLatin1Char('"')) ||
        escaped.contains(QLatin1Char('\n')) ||
        escaped.contains(QLatin1Char('\r'));

    if (!needsQuotes) {
        return escaped;
    }

    return QStringLiteral("\"%1\"").arg(escaped);
}

QByteArray SerialExportService::encodedPayload(const QString& text, bool writeUtf8Bom) const
{
    QByteArray payload;
    if (writeUtf8Bom) {
        payload.append(char(0xEF));
        payload.append(char(0xBB));
        payload.append(char(0xBF));
    }

    payload.append(text.toUtf8());
    return payload;
}

SerialExportResult SerialExportService::makeFailure(const SerialExportRequest& request,
                                                    const QString& message) const
{
    SerialExportResult result;
    result.filePath = request.filePath;
    result.format = formatName(request.format);
    result.errorMessage = message;
    return result;
}

SerialExportResult SerialExportService::makeSuccess(const SerialExportRequest& request,
                                                    qint64 bytesWritten) const
{
    SerialExportResult result;
    result.ok = true;
    result.filePath = QFileInfo(request.filePath).absoluteFilePath();
    result.format = formatName(request.format);
    result.bytesWritten = bytesWritten;
    return result;
}

bool SerialExportService::isSupportedFormat(SerialExportFormat format) const
{
    return format == SerialExportFormat::PlainText ||
        format == SerialExportFormat::JsonLines ||
        format == SerialExportFormat::Csv;
}

} // namespace serial_station
