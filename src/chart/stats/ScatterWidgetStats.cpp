/**
 * @file ScatterWidgetStats.cpp
 * @brief ScatterWidget 数据计算、槽函数、主题样式和统计接口
 *
 * 从 ScatterWidget.cpp 拆分而来，包含散点绘制与Pearson相关系数计算、
 * 通道变更/数据更新等槽函数、主题颜色应用、以及散点图统计计数器接口。
 */

#include "chart/stats/ScatterWidget.h"
#include "chart/model/ChartModel.h"
#include "core/theme/ThemeManager.h"

#include <cmath>
#include <algorithm>

// ============================================================
// 散点绘制与相关系数计算
// ============================================================

/** @brief 刷新散点图显示，从ChartModel读取X/Y通道数据绘制散点并计算Pearson相关系数 */
void ScatterWidget::refreshPlot()
{
    if (!m_model || !m_series) {
        return;
    }

    // 获取当前选中的通道名
    QString xName = m_xChannelCombo->currentText();
    QString yName = m_yChannelCombo->currentText();
    if (xName.isEmpty() || yName.isEmpty()) {
        m_series->replace({});
        m_correlationLabel->setText(tr("数据不足"));
        ++m_totalClears;
        ++m_totalUpdates;
        ++m_totalPointsRemoved;
        m_pointDensityMax = 0;
        m_averageValue = 0.0;
        return;
    }

    // 读取通道数据
    QVector<QPointF> xData = m_model->channelData(xName);
    QVector<QPointF> yData = m_model->channelData(yName);

    int n = qMin(xData.size(), yData.size());
    if (n == 0) {
        m_series->replace({});
        m_correlationLabel->setText(tr("数据不足"));
        ++m_totalClears;
        ++m_totalUpdates;
        ++m_totalPointsRemoved;
        m_pointDensityMax = 0;
        m_averageValue = 0.0;
        return;
    }

    // 构造散点: x = X通道值的y分量, y = Y通道值的y分量
    QVector<QPointF> points;
    points.reserve(n);
    double xMin = xData[0].y(), xMax = xData[0].y();
    double yMin = yData[0].y(), yMax = yData[0].y();

    for (int i = 0; i < n; ++i) {
        double xv = xData[i].y();
        double yv = yData[i].y();
        points.append(QPointF(xv, yv));
        if (xv < xMin) xMin = xv;
        if (xv > xMax) xMax = xv;
        if (yv < yMin) yMin = yv;
        if (yv > yMax) yMax = yv;
    }

    m_series->replace(points);
    m_totalPointsPlotted += n;
    m_totalPointsAdded += n;
    ++m_totalUpdates;
    ++m_totalSelections;

    // 计算Y值平均
    double ySum = 0.0;
    for (int i = 0; i < n; ++i) {
        ySum += yData[i].y();
    }
    m_averageValue = (n > 0) ? ySum / n : 0.0;

    // 计算点密度网格(将数据空间划分为20x20网格，统计每格点数)
    constexpr int kGridSize = 20;
    int densityGrid[kGridSize][kGridSize] = {};
    double xRange = (xMax - xMin) > 1e-12 ? (xMax - xMin) : 1.0;
    double yRange = (yMax - yMin) > 1e-12 ? (yMax - yMin) : 1.0;
    int maxDensity = 0;
    for (int i = 0; i < n; ++i) {
        int gx = qBound(0, static_cast<int>((points[i].x() - xMin) / xRange * kGridSize), kGridSize - 1);
        int gy = qBound(0, static_cast<int>((points[i].y() - yMin) / yRange * kGridSize), kGridSize - 1);
        ++densityGrid[gx][gy];
        if (densityGrid[gx][gy] > maxDensity) {
            maxDensity = densityGrid[gx][gy];
        }
    }
    m_pointDensityMax = maxDensity;

    // 自动调整坐标轴范围（留5%余量）
    double xPad = qMax((xMax - xMin) * 0.05, 0.001);
    double yPad = qMax((yMax - yMin) * 0.05, 0.001);
    m_xAxis->setRange(xMin - xPad, xMax + xPad);
    m_yAxis->setRange(yMin - yPad, yMax + yPad);
    ++m_totalAutoFits;

    // 更新轴标题
    m_xAxis->setTitleText(tr("X: %1").arg(xName));
    m_yAxis->setTitleText(tr("Y: %1").arg(yName));

    // 计算Pearson相关系数
    double r = computePearsonCorrelation(xData, yData);
    if (std::isnan(r)) {
        m_correlationLabel->setText(tr("数据不足"));
    } else {
        m_correlationLabel->setText(
            tr("相关系数 r = %1  |  样本数: %2")
                .arg(r, 0, 'f', 4)
                .arg(n));
    }
}

/** @brief 计算Pearson相关系数，标准公式r=Σ((x_i-x̄)(y_i-ȳ))/sqrt(Σ(x_i-x̄)²×Σ(y_i-ȳ)²) @param xData X轴数据（取QPointF的y分量作为值） @param yData Y轴数据（取QPointF的y分量作为值） @return Pearson r，数据不足或零方差时返回NaN */
double ScatterWidget::computePearsonCorrelation(
    const QVector<QPointF>& xData, const QVector<QPointF>& yData)
{
    int n = qMin(xData.size(), yData.size());
    if (n < 2) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    // 计算均值
    double xSum = 0.0, ySum = 0.0;
    for (int i = 0; i < n; ++i) {
        xSum += xData[i].y();
        ySum += yData[i].y();
    }
    double xMean = xSum / n;
    double yMean = ySum / n;

    // 计算协方差和方差
    double covXY = 0.0, varX = 0.0, varY = 0.0;
    for (int i = 0; i < n; ++i) {
        double dx = xData[i].y() - xMean;
        double dy = yData[i].y() - yMean;
        covXY += dx * dy;
        varX  += dx * dx;
        varY  += dy * dy;
    }

    // 零方差检查
    if (varX == 0.0 || varY == 0.0) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    return covXY / std::sqrt(varX * varY);
}

// ============================================================
// 槽函数
// ============================================================

/** @brief X轴通道变更时自动刷新 @param index 下拉框索引(未使用) */
void ScatterWidget::onXChannelChanged(int /*index*/)
{
    ++m_totalAxisChanges;
    if (m_autoRefresh) {
        refreshPlot();
    }
}

/** @brief Y轴通道变更时自动刷新 @param index 下拉框索引(未使用) */
void ScatterWidget::onYChannelChanged(int /*index*/)
{
    ++m_totalAxisChanges;
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

/** @brief 获取累计添加点数(每次刷新时新增的散点数) @return 添加点计数 */
quint64 ScatterWidget::totalPointsAdded() const { return m_totalPointsAdded; }

/** @brief 获取累计移除点数(数据清空时清除的散点数) @return 移除点计数 */
quint64 ScatterWidget::totalPointsRemoved() const { return m_totalPointsRemoved; }

/** @brief 获取累计自动拟合次数(坐标轴范围自适应) @return 自动拟合计数 */
quint64 ScatterWidget::totalAutoFits() const { return m_totalAutoFits; }

/** @brief 获取累计轴变更次数(X/Y通道切换触发) @return 轴变更计数 */
quint64 ScatterWidget::totalAxisChanges() const { return m_totalAxisChanges; }

/** @brief 获取累计选择次数(数据刷新时的散点集替换) @return 选择计数 */
quint64 ScatterWidget::totalSelections() const { return m_totalSelections; }

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
    m_totalPointsAdded = 0;
    m_totalPointsRemoved = 0;
    m_totalAutoFits = 0;
    m_totalAxisChanges = 0;
    m_totalSelections = 0;
    m_totalUpdates = 0;
    m_totalClears = 0;
    m_pointDensityMax = 0;
    m_averageValue = 0.0;
}
