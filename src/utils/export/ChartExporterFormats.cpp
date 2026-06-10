/**
 * @file ChartExporterFormats.cpp
 * @brief 图表导出器 —— CSV数据格式导出方法实现
 *
 * 包含 CSV 格式的具体导出逻辑。
 * JSON 导出见 ChartExporterJson.cpp。
 * PNG / SVG 图片导出见 ChartExporterImage.cpp。
 */

#include "utils/export/ChartExporter.h"
#include "chart/model/ChartModel.h"

#include <QFile>
#include <QTextStream>

// ---------------------------------------------------------------------------
// CSV 导出（ChartModel 版本）
// ---------------------------------------------------------------------------

/**
 * @brief 导出 ChartModel 数据为 CSV 文件
 *
 * 从 ChartModel 提取所有通道名称和数据点，
 * 委托给 exportToCsv(filePath, channelNames, data) 重载执行实际写入。
 * X轴使用帧索引作为时间戳。
 *
 * @param filePath 目标文件路径
 * @param model    图表数据模型指针
 * @return true 导出成功
 */
bool ChartExporter::exportToCsv(const QString& filePath, ChartModel* model)
{
    if (!model) {
        emit exportFailed(tr("图表模型指针为空，无法导出 CSV"));
        return false;
    }

    const QStringList names = model->channelNames();
    if (names.isEmpty()) {
        emit exportFailed(tr("图表模型中无通道数据"));
        return false;
    }

    /* 收集各通道数据，转为 QList<QList<double>> 格式 */
    QList<QList<double>> channelData;
    channelData.reserve(names.size());

    int maxRows = 0;
    for (const QString& chName : names) {
        const QVector<QPointF> points = model->channelData(chName);
        QList<double> values;
        values.reserve(points.size());
        for (const QPointF& pt : points) {
            values.append(pt.y());
        }
        maxRows = qMax(maxRows, values.size());
        channelData.append(std::move(values));
    }

    if (maxRows == 0) {
        emit exportFailed(tr("图表模型中无采样数据"));
        return false;
    }

    return exportToCsv(filePath, names, channelData);
}

// ---------------------------------------------------------------------------
// CSV 导出（多通道数据）
// ---------------------------------------------------------------------------

/**
 * @brief 导出多通道数据为 CSV 文件
 *
 * 输出格式：首行为表头 "Timestamp,CH1,CH2,..."，
 * 后续每行为 "时间戳(ms),通道1值,通道2值,..."。
 * 时间戳从 0 开始按行递增（单位 ms）。
 *
 * @param filePath   目标文件路径
 * @param channelNames 通道名称列表
 * @param data       各通道采样数据（外层=通道，内层=采样点）
 * @return true 写入成功
 */
bool ChartExporter::exportToCsv(const QString& filePath,
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

    QFile file(normalizedPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        ++m_totalErrors;
        ++m_totalExportErrors;
        emit exportFailed(tr("无法打开文件: %1").arg(normalizedPath));
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    /* --- 写表头 --- */
    out << "Timestamp(ms)";
    for (const auto& name : channelNames) {
        out << "," << name;
    }
    out << "\n";

    /* --- 确定最大行数 --- */
    int maxRows = 0;
    for (const auto& ch : data) {
        maxRows = qMax(maxRows, ch.size());
    }

    /* --- 逐行写数据 --- */
    for (int row = 0; row < maxRows; ++row) {
        out << row;  // 时间戳 = 行索引（ms）

        for (int ch = 0; ch < data.size(); ++ch) {
            out << ",";
            if (row < data[ch].size()) {
                out << data[ch][row];
            }
            // 超出该通道长度的位置留空
        }
        out << "\n";
    }

    file.close();
    ++m_totalExports;
    ++m_totalExportsCsv;
    m_totalCsvRows += static_cast<quint64>(maxRows);
    m_totalBytesExported += static_cast<quint64>(file.size());
    emit exportCompleted(normalizedPath);
    return true;
}

// JSON 导出见 ChartExporterJson.cpp
