/**
 * @file ChartWidgetExport.cpp
 * @brief ChartWidget 导出/快照与统计重置接口实现
 * 从 ChartWidget.cpp 拆分而来，包含:
 *   - exportScreenshot()   图表截图导出(PNG)
 *   - resetChartWidgetStatistics() 统计计数器重置
 */

#include "chart/widget/ChartWidget.h"

// ============================================================================
// 截图导出 + 统计重置
// ============================================================================

/** @brief 导出当前图表为截图(PNG) @param filePath 目标文件路径 @return true=导出成功 */
bool ChartWidget::exportScreenshot(const QString& filePath)
{
    if (!m_chartView) return false;

    QPixmap pixmap = m_chartView->grab();
    ++m_totalScreenshots;
    return pixmap.save(filePath, "PNG");
}

/** @brief 重置波形图统计计数器 */
void ChartWidget::resetChartWidgetStatistics()
{
    m_totalDataUpdates = 0;
    m_totalRenders = 0;
    m_totalRedraws = 0;
    m_totalInteractions = 0;
    m_totalZoomEvents = 0;
    m_totalPanEvents = 0;
    m_totalChannelToggles = 0;
    m_totalScreenshots = 0;
    m_totalSamplesAppended = 0;
    m_totalAutoScales = 0;
    m_totalManualZooms = 0;
    m_lastRenderTimeMs = 0;
    m_renderFps = 0.0;
}
