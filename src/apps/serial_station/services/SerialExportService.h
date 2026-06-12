#ifndef SERIAL_EXPORT_SERVICE_H
#define SERIAL_EXPORT_SERVICE_H

#include <QtCore/QString>
#include <QtCore/QVector>

#include "apps/serial_station/services/SerialLogService.h"

namespace serial_station {

/**
 * @brief Serial Station 日志导出格式。
 */
enum class SerialExportFormat {
    PlainText,
    JsonLines,
    Csv
};

/**
 * @brief Serial Station 日志导出请求。
 */
struct SerialExportRequest {
    QString filePath;
    SerialExportFormat format = SerialExportFormat::PlainText;
    bool writeUtf8Bom = false;
};

/**
 * @brief Serial Station 日志导出结果。
 */
struct SerialExportResult {
    bool ok = false;
    QString filePath;
    QString format;
    qint64 bytesWritten = 0;
    QString errorMessage;
};

/**
 * @brief Serial Station 服务层日志导出器。
 */
class SerialExportService {
public:
    /**
     * @brief 根据格式生成导出文本。
     */
    QString formatRecords(const QVector<SerialLogRecord>& records,
                          SerialExportFormat format) const;

    /**
     * @brief 将日志记录导出到文件。
     */
    SerialExportResult exportRecords(const QVector<SerialLogRecord>& records,
                                     const SerialExportRequest& request) const;

    /**
     * @brief 返回格式稳定名称。
     */
    QString formatName(SerialExportFormat format) const;

    /**
     * @brief 返回格式默认扩展名。
     */
    QString defaultSuffix(SerialExportFormat format) const;

private:
    QString formatPlainText(const QVector<SerialLogRecord>& records) const;
    QString formatJsonLines(const QVector<SerialLogRecord>& records) const;
    QString formatCsv(const QVector<SerialLogRecord>& records) const;
    QString csvLine(const SerialLogRecord& record) const;
    QString csvEscape(const QString& value) const;
    QByteArray encodedPayload(const QString& text, bool writeUtf8Bom) const;
    SerialExportResult makeFailure(const SerialExportRequest& request,
                                   const QString& message) const;
    SerialExportResult makeSuccess(const SerialExportRequest& request,
                                   qint64 bytesWritten) const;
    bool isSupportedFormat(SerialExportFormat format) const;
};

} // namespace serial_station

#endif // SERIAL_EXPORT_SERVICE_H
