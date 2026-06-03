/**
 * @file ChartExporter.cpp
 * @brief 图表导出器实现
 *
 * 实现 CSV/PNG/SVG/JSON 四种格式的图表数据导出。
 * CSV 和 JSON 使用多通道数据列表接口，PNG/SVG 使用 QWidget 渲染。
 */

#include "utils/export/ChartExporter.h"
#include "chart/model/ChartModel.h"

#include <QFile>
#include <QTextStream>
#include <QPainter>
#include <QPixmap>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QWidget>
#include <QSvgGenerator>

// ---------------------------------------------------------------------------
// 构造函数
// ---------------------------------------------------------------------------

ChartExporter::ChartExporter(QObject* parent)
    : QObject(parent)
{
}

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
    if (channelNames.isEmpty() || data.isEmpty()) {
        ++m_totalErrors;
        emit exportFailed(tr("导出数据为空"));
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        ++m_totalErrors;
        emit exportFailed(tr("无法打开文件: %1").arg(filePath));
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
    m_totalCsvRows += static_cast<quint64>(maxRows);
    emit exportCompleted(filePath);
    return true;
}

// ---------------------------------------------------------------------------
// PNG 导出
// ---------------------------------------------------------------------------

/**
 * @brief 将控件截图保存为 PNG 文件
 *
 * 使用 QWidget::grab() 获取控件像素图，调用 QPixmap::save() 写入。
 *
 * @param filePath 目标文件路径
 * @param widget   待截图的控件
 * @return true 保存成功
 */
bool ChartExporter::exportToPng(const QString& filePath, QWidget* widget)
{
    if (!widget) {
        ++m_totalErrors;
        emit exportFailed(tr("控件指针为空，无法导出 PNG"));
        return false;
    }

    QPixmap pixmap = widget->grab();
    if (pixmap.isNull()) {
        ++m_totalErrors;
        emit exportFailed(tr("截图失败: 控件为空"));
        return false;
    }

    if (!pixmap.save(filePath, "PNG")) {
        ++m_totalErrors;
        emit exportFailed(tr("PNG 保存失败: %1").arg(filePath));
        return false;
    }

    ++m_totalExports;
    ++m_totalChartImages;
    emit exportCompleted(filePath);
    return true;
}

// ---------------------------------------------------------------------------
// SVG 导出
// ---------------------------------------------------------------------------

/**
 * @brief 将控件渲染为 SVG 矢量图
 *
 * 使用 QSvgGenerator 生成真正的 SVG 矢量文件，
 * 而非降级为 PNG 位图。保留控件的所有绘制细节。
 *
 * @param filePath 目标文件路径（应以 .svg 结尾）
 * @param widget   待渲染的控件
 * @return true 渲染成功
 */
bool ChartExporter::exportToSvg(const QString& filePath, QWidget* widget)
{
    if (!widget) {
        ++m_totalErrors;
        emit exportFailed(tr("控件指针为空，无法导出 SVG"));
        return false;
    }

    /* 配置 SVG 生成器 */
    QSvgGenerator generator;
    generator.setFileName(filePath);
    generator.setSize(widget->size());
    generator.setViewBox(QRect(0, 0, widget->width(), widget->height()));
    generator.setTitle(tr("EmbedDebug 图表导出"));
    generator.setDescription(tr("由 EmbedDebug 自动生成的 SVG 矢量图"));

    /* 使用 QPainter 渲染到 SVG */
    QPainter painter(&generator);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    widget->render(&painter);
    painter.end();

    ++m_totalExports;
    ++m_totalChartImages;
    emit exportCompleted(filePath);
    return true;
}

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
    if (channelNames.isEmpty() || data.isEmpty()) {
        ++m_totalErrors;
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
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        ++m_totalErrors;
        emit exportFailed(tr("无法打开文件: %1").arg(filePath));
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    ++m_totalExports;
    m_totalCsvRows += static_cast<quint64>(maxRows);
    emit exportCompleted(filePath);
    return true;
}

// ---------------------------------------------------------------------------
// 统计计数
// ---------------------------------------------------------------------------

/** @brief 获取累计导出操作总次数 @return 导出次数 */
quint64 ChartExporter::totalExports() const
{
    return m_totalExports;
}

/** @brief 获取累计图表图片导出次数(PNG+SVG) @return 图片导出次数 */
quint64 ChartExporter::totalChartImages() const
{
    return m_totalChartImages;
}

/** @brief 获取累计CSV导出的数据行总数 @return CSV行数 */
quint64 ChartExporter::totalCsvRows() const
{
    return m_totalCsvRows;
}

/** @brief 获取累计导出失败次数 @return 失败次数 */
quint64 ChartExporter::totalErrors() const
{
    return m_totalErrors;
}

/** @brief 重置所有导出统计计数器(导出次数/图片次数/CSV行数/错误次数) */
void ChartExporter::resetExportStatistics()
{
    m_totalExports = 0;
    m_totalChartImages = 0;
    m_totalCsvRows = 0;
    m_totalErrors = 0;
}
