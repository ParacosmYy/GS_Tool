/**
 * @file ScatterWidgetStats.cpp
 * @brief ScatterWidget 槽函数、主题样式和统计接口
 *
 * 从 ScatterWidget.cpp 拆分而来，包含通道变更/数据更新等槽函数、
 * 主题颜色应用、以及散点图统计计数器接口。
 */

#include "chart/stats/ScatterWidget.h"
#include "chart/model/ChartModel.h"
#include "core/theme/ThemeManager.h"

// ============================================================
// 槽函数
// ============================================================

/** @brief X轴通道变更时自动刷新 @param index 下拉框索引(未使用) */
void ScatterWidget::onXChannelChanged(int /*index*/)
{
    if (m_autoRefresh) {
        refreshPlot();
    }
}

/** @brief Y轴通道变更时自动刷新 @param index 下拉框索引(未使用) */
void ScatterWidget::onYChannelChanged(int /*index*/)
{
    if (m_autoRefresh) {
        refreshPlot();
    }
}

/** @brief 自动刷新开关切换 @param checked 是否开启自动刷新 */
void ScatterWidget::onAutoRefreshToggled(bool checked)
{
    m_autoRefresh = checked;
}

/** @brief ChartModel数据更新回调，自动刷新模式下仅当X或Y通道有更新时触发重绘 @param updatedChannels 更新的通道名称列表 */
void ScatterWidget::onDataUpdated(const QStringList& updatedChannels)
{
    if (!m_autoRefresh) return;

    QString xName = m_xChannelCombo->currentText();
    QString yName = m_yChannelCombo->currentText();
    if (updatedChannels.contains(xName) || updatedChannels.contains(yName)) {
        refreshPlot();
    }
}

/** @brief 通道列表变更时重建X/Y下拉框，保存之前的选择并尽量恢复 */
void ScatterWidget::onChannelsChanged()
{
    if (!m_model) return;

    QString prevX = m_xChannelCombo->currentText();
    QString prevY = m_yChannelCombo->currentText();

    // 重建X通道下拉框
    m_xChannelCombo->blockSignals(true);
    m_xChannelCombo->clear();
    QStringList names = m_model->channelNames();
    for (const auto& name : names) {
        m_xChannelCombo->addItem(name);
    }
    int xIdx = m_xChannelCombo->findText(prevX);
    if (xIdx >= 0) {
        m_xChannelCombo->setCurrentIndex(xIdx);
    } else if (m_xChannelCombo->count() > 0) {
        m_xChannelCombo->setCurrentIndex(0);
    }
    m_xChannelCombo->blockSignals(false);

    // 重建Y通道下拉框
    m_yChannelCombo->blockSignals(true);
    m_yChannelCombo->clear();
    for (const auto& name : names) {
        m_yChannelCombo->addItem(name);
    }
    int yIdx = m_yChannelCombo->findText(prevY);
    if (yIdx >= 0) {
        m_yChannelCombo->setCurrentIndex(yIdx);
    } else if (m_yChannelCombo->count() > 0) {
        m_yChannelCombo->setCurrentIndex(qMin(1, m_yChannelCombo->count() - 1));
    }
    m_yChannelCombo->blockSignals(false);

    if (m_autoRefresh && m_xChannelCombo->count() > 0) {
        refreshPlot();
    }
}

/** @brief 主题切换响应 */
void ScatterWidget::onThemeChanged()
{
    applyThemeColors();
}

// ============================================================
// 主题样式
// ============================================================

/** @brief 应用当前主题颜色到图表背景、网格线、坐标轴标签和散点颜色 */
void ScatterWidget::applyThemeColors()
{
    auto& theme = ThemeManager::instance();

    QColor bgColor = theme.color(ThemeManager::SemanticColor::BgPrimary);
    m_chart->setBackgroundBrush(bgColor);

    QColor gridColor = theme.color(ThemeManager::SemanticColor::Border);
    QColor labelColor = theme.color(ThemeManager::SemanticColor::TextSecondary);

    m_xAxis->setLinePen(QPen(gridColor, 1));
    m_xAxis->setGridLinePen(QPen(gridColor, 1, Qt::DashLine));
    m_xAxis->setLabelsBrush(labelColor);
    m_xAxis->setTitleBrush(labelColor);

    m_yAxis->setLinePen(QPen(gridColor, 1));
    m_yAxis->setGridLinePen(QPen(gridColor, 1, Qt::DashLine));
    m_yAxis->setLabelsBrush(labelColor);
    m_yAxis->setTitleBrush(labelColor);

    bool isDark = (theme.currentTheme().startsWith(QStringLiteral("dark")));
    const auto& colors = ChartColors::colorsForTheme(isDark);
    QColor dotColor = colors.isEmpty() ? QColor("#89b4fa") : colors.first();
    m_series->setColor(dotColor);
    m_series->setBorderColor(dotColor);

    m_chart->setPlotAreaBackgroundBrush(QBrush(bgColor));
    m_chart->setPlotAreaBackgroundVisible(true);
}

// ============================================================
// 统计接口
// ============================================================

/** @brief 获取累计绘制点数 @return 绘制点数 */
quint64 ScatterWidget::totalPointsPlotted() const { return m_totalPointsPlotted; }

/** @brief 获取累计更新次数 @return 更新次数 */
quint64 ScatterWidget::totalUpdates() const { return m_totalUpdates; }

/** @brief 获取累计清除次数 @return 清除次数 */
quint64 ScatterWidget::totalClears() const { return m_totalClears; }

/** @brief 获取最近一次刷新的点密度网格最大值 @return 密度最大值 */
int ScatterWidget::pointDensityMax() const { return m_pointDensityMax; }

/** @brief 获取最近一次刷新的所有散点Y值平均 @return Y值平均 */
double ScatterWidget::averageValue() const { return m_averageValue; }

/** @brief 重置所有散点图统计计数器 */
void ScatterWidget::resetScatterStatistics()
{
    m_totalPointsPlotted = 0;
    m_totalUpdates = 0;
    m_totalClears = 0;
    m_pointDensityMax = 0;
    m_averageValue = 0.0;
}
