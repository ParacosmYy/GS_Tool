/**
 * @file ChartExporter.cpp
 * @brief 图表导出器实现 —— 构造与统计
 *
 * 本文件仅包含 ChartExporter 的构造/析构及导出统计计数方法。
 * 各格式导出方法见 ChartExporterFormats.cpp。
 */

#include "utils/export/ChartExporter.h"

// ---------------------------------------------------------------------------
// 构造函数
// ---------------------------------------------------------------------------

ChartExporter::ChartExporter(QObject* parent)
    : QObject(parent)
{
}

// ---------------------------------------------------------------------------
// 统计计数
// ---------------------------------------------------------------------------

/** @brief 获取累计导出操作总次数 @return 导出次数 */
quint64 ChartExporter::totalExports() const
{
    return m_totalExports;
}

/** @brief 获取累计PNG导出次数 @return PNG导出计数 */
quint64 ChartExporter::totalExportsPng() const
{
    return m_totalExportsPng;
}

/** @brief 获取累计CSV导出次数 @return CSV导出计数 */
quint64 ChartExporter::totalExportsCsv() const
{
    return m_totalExportsCsv;
}

/** @brief 获取累计PDF导出次数(预留) @return PDF导出计数 */
quint64 ChartExporter::totalExportsPdf() const
{
    return m_totalExportsPdf;
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

/** @brief 获取累计导出错误次数 @return 导出错误计数 */
quint64 ChartExporter::totalExportErrors() const
{
    return m_totalExportErrors;
}

/** @brief 获取累计导出失败次数(兼容旧接口) @return 失败次数 */
quint64 ChartExporter::totalErrors() const
{
    return m_totalErrors;
}

/** @brief 获取累计导出字节总数(所有格式文件大小之和) @return 字节数 */
quint64 ChartExporter::totalBytesExported() const
{
    return m_totalBytesExported;
}

/** @brief 重置所有导出统计计数器(导出次数/图片次数/CSV行数/错误次数) */
void ChartExporter::resetExportStatistics()
{
    m_totalExports = 0;
    m_totalExportsPng = 0;
    m_totalExportsCsv = 0;
    m_totalExportsPdf = 0;
    m_totalChartImages = 0;
    m_totalCsvRows = 0;
    m_totalExportErrors = 0;
    m_totalErrors = 0;
    m_totalBytesExported = 0;
}
