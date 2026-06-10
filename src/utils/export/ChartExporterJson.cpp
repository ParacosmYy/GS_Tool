/**
 * @file ChartExporterJson.cpp
 * @brief 图表导出器 —— JSON数据格式导出方法实现
 *
 * 从 ChartExporterFormats.cpp 拆分而来，包含 JSON 格式的数据导出逻辑。
 * CSV 导出见 ChartExporterFormats.cpp。
 * PNG / SVG 图片导出见 ChartExporterImage.cpp。
 */

#include "utils/export/ChartExporter.h"
#include "chart/model/ChartModel.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

// ---------------------------------------------------------------------------
// JSON 导出
// ---------------------------------------------------------------------------

/**
 * @brief 导出多通道数据为 JSON 文件
 *
 * 输出格式为 JSON 数组，每个元素为对象：
 * {"timestamp": 0, "CH1": 1.23, "CH2": 4.56, ...}
 *
 * @param filePath     目标文件路径
 * @param channelNames 通道名称列表
 * @param data         各通道采样数据
 * @return true 写入成功
 */
bool ChartExporter::exportToJson(const QString& filePath,
                                 const QStringList& channelNames,
                                 const QList<QList<double>>& data)
{
    const QString normalizedPath = filePath.trimmed();
    if (normalizedPath.isEmpty()) {
        ++m_totalErrors;
        ++m_totalExportErrors;
        emit exportFailed(tr("文件路径为空"));
        return false;
    }

    if (channelNames.isEmpty() || data.isEmpty()) {
        ++m_totalErrors;
        ++m_totalExportErrors;
        emit exportFailed(tr("导出数据为空"));
        return false;
    }

    /* --- 确定最大行数 --- */
    int maxRows = 0;
    for (const auto& ch : data) {
        maxRows = qMax(maxRows, ch.size());
    }

    /* --- 构建 JSON 数组 --- */
    QJsonArray rootArray;
    for (int row = 0; row < maxRows; ++row) {
        QJsonObject rowObj;
        rowObj["timestamp"] = row;

        for (int ch = 0; ch < channelNames.size(); ++ch) {
            if (row < data[ch].size()) {
                rowObj[channelNames[ch]] = data[ch][row];
            } else {
                rowObj[channelNames[ch]] = QJsonValue::Null;
            }
        }
        rootArray.append(rowObj);
    }

    /* --- 写文件 --- */
    QJsonDocument doc(rootArray);
    QFile file(normalizedPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        ++m_totalErrors;
        ++m_totalExportErrors;
        emit exportFailed(tr("无法打开文件: %1").arg(normalizedPath));
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    ++m_totalExports;
    ++m_totalExportsJson;
    m_totalCsvRows += static_cast<quint64>(maxRows);
    m_totalBytesExported += static_cast<quint64>(file.size());
    emit exportCompleted(normalizedPath);
    return true;
}
