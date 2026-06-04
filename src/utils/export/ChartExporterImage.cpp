/**
 * @file ChartExporterImage.cpp
 * @brief 图表导出器 —— 图片导出方法实现
 *
 * 包含 PNG 位图导出和 SVG 矢量图导出的具体逻辑。
 * 从 ChartExporterFormats.cpp 拆分而来，将图片类导出与数据类导出分离，
 * 便于按导出类型独立维护。
 */

#include "utils/export/ChartExporter.h"

#include <QPainter>
#include <QPixmap>
#include <QSvgGenerator>
#include <QWidget>
#include <QFileInfo>

// ---------------------------------------------------------------------------
// PNG 导出
// ---------------------------------------------------------------------------

/**
 * @brief 将控件截图保存为 PNG 文件
 *
 * 使用 QWidget::grab() 获取控件像素图，调用 QPixmap::save() 写入。
 * 导出成功后更新 PNG 计数器和字节统计，并发射 exportCompleted 信号。
 *
 * @param filePath 目标文件路径
 * @param widget   待截图的控件
 * @return true 保存成功
 */
bool ChartExporter::exportToPng(const QString& filePath, QWidget* widget)
{
    if (!widget) {
        ++m_totalErrors;
        ++m_totalExportErrors;
        emit exportFailed(tr("控件指针为空，无法导出 PNG"));
        return false;
    }

    QPixmap pixmap = widget->grab();
    if (pixmap.isNull()) {
        ++m_totalErrors;
        ++m_totalExportErrors;
        emit exportFailed(tr("截图失败: 控件为空"));
        return false;
    }

    if (!pixmap.save(filePath, "PNG")) {
        ++m_totalErrors;
        ++m_totalExportErrors;
        emit exportFailed(tr("PNG 保存失败: %1").arg(filePath));
        return false;
    }

    ++m_totalExports;
    ++m_totalExportsPng;
    ++m_totalChartImages;
    // PNG文件大小估算(像素宽×高×4字节RGBA近似)
    QFileInfo fi(filePath);
    if (fi.exists()) {
        m_totalBytesExported += static_cast<quint64>(fi.size());
    }
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
 * 导出成功后更新图表图片计数器和字节统计，并发射 exportCompleted 信号。
 *
 * @param filePath 目标文件路径（应以 .svg 结尾）
 * @param widget   待渲染的控件
 * @return true 渲染成功
 */
bool ChartExporter::exportToSvg(const QString& filePath, QWidget* widget)
{
    if (!widget) {
        ++m_totalErrors;
        ++m_totalExportErrors;
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
    QFileInfo svgFi(filePath);
    if (svgFi.exists()) {
        m_totalBytesExported += static_cast<quint64>(svgFi.size());
    }
    emit exportCompleted(filePath);
    return true;
}
