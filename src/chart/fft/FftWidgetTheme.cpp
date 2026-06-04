/**
 * @file FftWidgetTheme.cpp
 * @brief FFT频谱控件 -- 主题样式应用实现
 *
 * 从 FftWidgetSetup.cpp 拆分而来，集中管理FftWidget的主题感知样式刷新逻辑:
 *   - applyThemeColors(): 主题感知样式刷新（背景、网格、标签、线条颜色）
 *
 * UI搭建见 FftWidgetSetup.cpp。
 * 槽函数与统计接口见 FftWidgetSlots.cpp。
 *
 * 所有方法均为FftWidget的private成员，声明见FftWidget.h。
 */

#include "chart/fft/FftWidget.h"
#include "chart/model/ChartModel.h"
#include "core/theme/ThemeManager.h"

#include <QtCharts>

// ============================================================
// 主题样式应用
// ============================================================

/** @brief 应用当前主题颜色到图表背景、网格线、坐标轴标签和频谱曲线 */
void FftWidget::applyThemeColors()
{
    auto& theme = ThemeManager::instance();

    // 图表背景
    QColor bgColor = theme.color(ThemeManager::SemanticColor::BgPrimary);
    m_chart->setBackgroundBrush(bgColor);

    // 网格线和坐标轴颜色
    QColor gridColor = theme.color(ThemeManager::SemanticColor::Border);
    QColor labelColor = theme.color(ThemeManager::SemanticColor::TextSecondary);

    // X轴样式
    m_xAxis->setLinePen(QPen(gridColor, 1));
    m_xAxis->setGridLinePen(QPen(gridColor, 1, Qt::DashLine));
    m_xAxis->setLabelsBrush(labelColor);
    m_xAxis->setTitleBrush(labelColor);

    // Y轴样式
    m_yAxis->setLinePen(QPen(gridColor, 1));
    m_yAxis->setGridLinePen(QPen(gridColor, 1, Qt::DashLine));
    m_yAxis->setLabelsBrush(labelColor);
    m_yAxis->setTitleBrush(labelColor);

    // 频谱线条颜色: 使用ChartColors调色板第一色（蓝色系）
    bool isDark = (theme.currentTheme().startsWith(QStringLiteral("dark")));
    const auto& colors = ChartColors::colorsForTheme(isDark);
    QColor spectrumColor = colors.isEmpty() ? QColor("#89b4fa") : colors.first();
    m_spectrumSeries->setPen(QPen(spectrumColor, 1.5));

    // 图表绘图区背景（与整体背景保持一致）
    QBrush plotAreaBrush(bgColor);
    m_chart->setPlotAreaBackgroundBrush(plotAreaBrush);
    m_chart->setPlotAreaBackgroundVisible(true);
}
