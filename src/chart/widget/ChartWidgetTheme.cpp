/**
 * @file ChartWidgetTheme.cpp
 * @brief ChartWidget 主题切换、事件处理与Series管理
 *
 * 从 ChartWidget.cpp 拆分而来，包含主题颜色应用、窗口resize事件、
 * 数据线Series创建/移除和统计计数器重置。
 */

#include "chart/widget/ChartWidget.h"
#include "chart/widget/ChartColors.h"
#include "core/theme/ThemeManager.h"

#include <QLineSeries>
#include <QResizeEvent>

// ============================================================================
// 主题切换
// ============================================================================

/** @brief 主题切换回调，重绘所有图表视觉元素 */
void ChartWidget::onThemeChanged()
{
    applyThemeColors();
}

/** @brief 应用当前主题颜色到图表背景、坐标轴和series */
void ChartWidget::applyThemeColors()
{
    ++m_totalRedraws;
    auto& theme = ThemeManager::instance();
    bool isDark = theme.isSystemDarkMode() || theme.currentTheme().contains("dark");

    /* 图表背景色 */
    QColor bgColor = theme.color(ThemeManager::SemanticColor::BgPrimary);
    m_chart->setBackgroundBrush(QBrush(bgColor));
    m_chart->setPlotAreaBackgroundBrush(QBrush(bgColor));
    m_chart->setPlotAreaBackgroundVisible(true);

    /* 网格线和标签颜色 */
    QColor gridColor = theme.color(ThemeManager::SemanticColor::Border);
    QColor labelColor = theme.color(ThemeManager::SemanticColor::TextSecondary);

    m_xAxis->setGridLineColor(gridColor);
    m_xAxis->setLinePen(QPen(gridColor, 1));
    m_xAxis->setLabelsBrush(QBrush(labelColor));
    m_xAxis->setTitleBrush(labelColor);

    m_yAxisManager->applyThemeColors(gridColor, labelColor);

    /* 图例文字颜色 */
    if (m_chart->legend()) {
        m_chart->legend()->setLabelColor(labelColor);
    }

    /* 数据线颜色: 按通道索引重新分配调色板 */
    QVector<QColor> palette = ChartColors::colorsForTheme(isDark);
    if (palette.isEmpty()) return;
    int index = 0;
    for (auto it = m_seriesMap.begin(); it != m_seriesMap.end(); ++it, ++index) {
        QColor lineColor = palette[index % palette.size()];
        it.value()->setColor(lineColor);
    }
}

// ============================================================================
// 事件处理
// ============================================================================

/** @brief 窗口大小变化时同步游标叠加层尺寸 @param event 调整大小事件 */
void ChartWidget::resizeEvent(QResizeEvent* event)
{
    ++m_totalRedraws;
    QWidget::resizeEvent(event);
    if (m_cursorOverlay && m_chartView) {
        m_cursorOverlay->setGeometry(m_chartView->rect());
    }
}

// ============================================================================
// 内部方法 -- Series管理
// ============================================================================

/** @brief 为指定通道创建QLineSeries并添加到图表 @param name 通道名称 @param color 线条颜色 */
void ChartWidget::createSeries(const QString& name, const QColor& color)
{
    if (m_seriesMap.contains(name)) return;

    QColor chColor = color;
    if (!chColor.isValid()) {
        bool isDark = ThemeManager::instance().currentTheme().contains("dark");
        QVector<QColor> palette = ChartColors::colorsForTheme(isDark);
        if (!palette.isEmpty()) {
            chColor = palette[m_seriesMap.size() % palette.size()];
        } else {
            chColor = ThemeManager::instance().color(ThemeManager::SemanticColor::Accent);
        }
    }

    auto* series = new QLineSeries;
    series->setObjectName(QStringLiteral("chartSeries_%1").arg(name));
    series->setName(name);
    series->setColor(chColor);
    series->setUseOpenGL(true);

    m_chart->addSeries(series);
    series->attachAxis(m_xAxis);
    m_yAxisManager->attachSeries(name, series);

    m_seriesMap[name] = series;
}

/** @brief 移除指定通道的QLineSeries @param name 通道名称 */
void ChartWidget::removeSeries(const QString& name)
{
    auto it = m_seriesMap.find(name);
    if (it == m_seriesMap.end()) return;

    m_chart->removeSeries(it.value());
    it.value()->deleteLater();
    m_seriesMap.erase(it);
}

// ============================================================================
// 统计计数器
// ============================================================================

/** @brief 查询当前主题的网格线颜色，从ThemeManager获取Border语义色 @return 网格线QColor */
QColor ChartWidget::gridColor() const
{
    return ThemeManager::instance().color(ThemeManager::SemanticColor::Border);
}

// resetChartWidgetStatistics() 定义在 ChartWidget.cpp 中（含完整字段重置）
