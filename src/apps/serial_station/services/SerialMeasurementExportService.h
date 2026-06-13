#ifndef SERIAL_MEASUREMENT_EXPORT_SERVICE_H
#define SERIAL_MEASUREMENT_EXPORT_SERVICE_H

#include <QtCore/QString>

#include "apps/serial_station/services/SerialMeasurementService.h"

namespace serial_station {

/** @brief 测量数据 CSV 导出请求。 */
struct SerialMeasurementExportRequest {
    QString filePath;        ///< 目标 CSV 文件路径
    bool writeUtf8Bom = false; ///< 是否写入 UTF-8 BOM
};

/** @brief 测量数据 CSV 导出结果。 */
struct SerialMeasurementExportResult {
    bool ok = false;         ///< 是否成功写入
    QString filePath;        ///< 规范化后的目标路径
    QString format;          ///< 稳定格式名
    qint64 bytesWritten = 0; ///< 实际写入字节数
    QString errorMessage;    ///< 失败原因
};

/**
 * @brief Serial Station 测量数据导出服务。
 *
 * 只接收测量快照并写出 CSV，不接触 QWidget、串口线程或协议解析。
 */
class SerialMeasurementExportService {
public:
    /** @brief 将测量快照格式化为 CSV 文本。 */
    QString formatSnapshot(const SerialMeasurementSnapshot& snapshot) const;

    /** @brief 将测量快照以 CSV 写入文件。 */
    SerialMeasurementExportResult exportSnapshot(
        const SerialMeasurementSnapshot& snapshot,
        const SerialMeasurementExportRequest& request) const;

private:
    QByteArray encodedPayload(const QString& text, bool writeUtf8Bom) const;
    SerialMeasurementExportResult makeFailure(
        const SerialMeasurementExportRequest& request,
        const QString& message) const;
    SerialMeasurementExportResult makeSuccess(
        const SerialMeasurementExportRequest& request,
        qint64 bytesWritten) const;
};

} // namespace serial_station

#endif // SERIAL_MEASUREMENT_EXPORT_SERVICE_H
