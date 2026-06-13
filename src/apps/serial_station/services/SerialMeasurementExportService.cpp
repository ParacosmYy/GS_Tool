#include "apps/serial_station/services/SerialMeasurementExportService.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QIODevice>
#include <QtCore/QSaveFile>
#include <QtCore/QStringList>

namespace serial_station {

QString SerialMeasurementExportService::formatSnapshot(
    const SerialMeasurementSnapshot& snapshot) const
{
    if (snapshot.recentFrames.isEmpty()) {
        return QString();
    }

    int channelCount = 0;
    for (const SerialMeasurementFrame& frame : snapshot.recentFrames) {
        channelCount = qMax(channelCount, frame.values.size());
    }

    QStringList rows;
    QStringList header;
    header.append(QStringLiteral("frame"));
    for (int index = 0; index < channelCount; ++index) {
        header.append(QStringLiteral("ch%1").arg(index + 1));
    }
    rows.append(header.join(QLatin1Char(',')));

    for (const SerialMeasurementFrame& frame : snapshot.recentFrames) {
        QStringList row;
        row.append(QString::number(frame.frameIndex));
        for (int index = 0; index < channelCount; ++index) {
            row.append(index < frame.values.size()
                           ? QString::number(frame.values.at(index), 'g', 6)
                           : QString());
        }
        rows.append(row.join(QLatin1Char(',')));
    }

    return rows.join(QLatin1Char('\n')) + QLatin1Char('\n');
}

SerialMeasurementExportResult SerialMeasurementExportService::exportSnapshot(
    const SerialMeasurementSnapshot& snapshot,
    const SerialMeasurementExportRequest& request) const
{
    if (request.filePath.trimmed().isEmpty()) {
        return makeFailure(request, QStringLiteral("测量导出路径为空"));
    }

    if (snapshot.recentFrames.isEmpty()) {
        return makeFailure(request, QStringLiteral("没有可导出的测量数据"));
    }

    const QFileInfo fileInfo(request.filePath);
    const QDir parentDir = fileInfo.absoluteDir();
    if (!parentDir.exists()) {
        return makeFailure(request,
                           QStringLiteral("测量导出目录不存在: %1")
                               .arg(parentDir.absolutePath()));
    }

    const QString text = formatSnapshot(snapshot);
    if (text.isEmpty()) {
        return makeFailure(request, QStringLiteral("测量导出内容为空"));
    }

    const QByteArray payload = encodedPayload(text, request.writeUtf8Bom);
    QSaveFile file(fileInfo.absoluteFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return makeFailure(request,
                           QStringLiteral("无法打开测量导出文件: %1")
                               .arg(file.errorString()));
    }

    const qint64 written = file.write(payload);
    if (written != payload.size()) {
        return makeFailure(request, QStringLiteral("测量导出文件写入不完整"));
    }

    if (!file.commit()) {
        return makeFailure(request,
                           QStringLiteral("测量导出文件提交失败: %1")
                               .arg(file.errorString()));
    }

    return makeSuccess(request, written);
}

QByteArray SerialMeasurementExportService::encodedPayload(
    const QString& text,
    bool writeUtf8Bom) const
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

SerialMeasurementExportResult SerialMeasurementExportService::makeFailure(
    const SerialMeasurementExportRequest& request,
    const QString& message) const
{
    SerialMeasurementExportResult result;
    result.filePath = request.filePath;
    result.format = QStringLiteral("csv");
    result.errorMessage = message;
    return result;
}

SerialMeasurementExportResult SerialMeasurementExportService::makeSuccess(
    const SerialMeasurementExportRequest& request,
    qint64 bytesWritten) const
{
    SerialMeasurementExportResult result;
    result.ok = true;
    result.filePath = QFileInfo(request.filePath).absoluteFilePath();
    result.format = QStringLiteral("csv");
    result.bytesWritten = bytesWritten;
    return result;
}

} // namespace serial_station
