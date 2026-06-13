#include "apps/serial_station/SerialStationController.h"

#include <QtCore/QDateTime>

namespace serial_station {

SerialLogService& SerialStationController::logService()
{
    return m_logService;
}

const SerialLogService& SerialStationController::logService() const
{
    return m_logService;
}

QVector<SerialLogRecord> SerialStationController::logRecords() const
{
    return m_logService.records();
}

QVector<SerialLogRecord> SerialStationController::logRecords(const SerialLogFilter& filter) const
{
    return m_logService.records(filter);
}

QString SerialStationController::logPlainText() const
{
    return m_logService.toPlainText();
}

QString SerialStationController::logPlainText(const SerialLogFilter& filter) const
{
    return m_logService.toPlainText(filter);
}

QString SerialStationController::logJsonLines() const
{
    return m_logService.toJsonLines();
}

QString SerialStationController::logJsonLines(const SerialLogFilter& filter) const
{
    return m_logService.toJsonLines(filter);
}

SerialExportResult SerialStationController::exportLogRecords(const SerialExportRequest& request)
{
    const SerialExportResult result = m_exportService.exportRecords(m_logService.records(), request);
    if (result.ok) {
        logSystem(tr("日志已导出: %1 (%2, %3 bytes)")
                      .arg(result.filePath, result.format, QString::number(result.bytesWritten)),
                  {{QStringLiteral("filePath"), result.filePath},
                   {QStringLiteral("format"), result.format},
                   {QStringLiteral("bytesWritten"), result.bytesWritten}});
        return result;
    }

    logError(tr("日志导出失败: %1").arg(result.errorMessage),
             {{QStringLiteral("filePath"), result.filePath},
              {QStringLiteral("format"), result.format},
              {QStringLiteral("message"), result.errorMessage}});
    return result;
}

SerialProfileWriteResult SerialStationController::saveProfileToFile(
    const SerialStationProfile& profile,
    const QString& filePath)
{
    const SerialProfileWriteResult result = m_profileService.saveToFile(profile, filePath);
    if (result.ok) {
        logSystem(QStringLiteral("已保存配置档案: %1").arg(result.filePath),
                  {{QStringLiteral("filePath"), result.filePath}});
    } else {
        logError(QStringLiteral("保存配置档案失败: %1").arg(result.errorMessage),
                 {{QStringLiteral("filePath"), filePath}});
    }
    return result;
}

SerialProfileResult SerialStationController::loadProfileFromFile(const QString& filePath)
{
    const SerialProfileResult result = m_profileService.loadFromFile(filePath);
    if (result.ok) {
        logSystem(QStringLiteral("已加载配置档案: %1").arg(result.profile.name),
                  {{QStringLiteral("filePath"), filePath},
                   {QStringLiteral("profileName"), result.profile.name}});
    } else {
        logError(QStringLiteral("加载配置档案失败: %1").arg(result.errorMessage),
                 {{QStringLiteral("filePath"), filePath}});
    }
    return result;
}

QString SerialStationController::suggestedExportFileName(SerialExportFormat format) const
{
    const QString timestamp =
        QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss"));
    const QString suffix = m_exportService.defaultSuffix(format);
    if (suffix.isEmpty()) {
        return QStringLiteral("serial-log-%1.log").arg(timestamp);
    }

    return QStringLiteral("serial-log-%1.%2").arg(timestamp, suffix);
}

} // namespace serial_station
