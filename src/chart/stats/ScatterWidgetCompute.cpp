/**
 * @file ScatterWidgetCompute.cpp
 * @brief ScatterWidget 散点绘制、密度网格计算与Pearson相关系数计算
 *
 * 从 ScatterWidgetStats.cpp 拆分而来，集中承载计算密集型方法:
 *   - refreshPlot(): 读取通道数据、构造散点、密度网格、坐标轴自适应、相关系数更新
 *   - computePearsonCorrelation(): 标准Pearson r公式实现
 *
 * 设计目标:
 *   1. 将纯计算逻辑与槽函数/主题/统计接口分离，降低单文件复杂度
 *   2. 便于后续对计算核心进行单元测试和性能优化
 */

#include "chart/stats/ScatterWidget.h"
#include "chart/model/ChartModel.h"

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
    ++m_totalPointUpdates;

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
    ++m_totalAxisAutoScales;

    // 更新轴标题
    m_xAxis->setTitleText(tr("X: %1").arg(xName));
    m_yAxis->setTitleText(tr("Y: %1").arg(yName));

    // 累计渲染计数
    ++m_totalRenders;

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

/** @brief 计算Pearson相关系数，标准公式r=Σ((x_i-x̄)(y_i-ȳ))/sqrt(Σ(x_i-x̄)²×Σ(y_i-ȳ)²)
 *  @param xData X轴数据（取QPointF的y分量作为值）
 *  @param yData Y轴数据（取QPointF的y分量作为值）
 *  @return Pearson r，数据不足或零方差时返回NaN */
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
