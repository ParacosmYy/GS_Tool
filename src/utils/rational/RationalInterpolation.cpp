/**
 * @file RationalInterpolation.cpp
 * @brief 重心有理插值实现
 */

#include "utils/rational/RationalInterpolation.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
RationalInterpolation::RationalInterpolation(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief Berrut 第一型重心有理插值
 *
 * 权重: w_i = (-1)^i
 * 公式: r(x) = Σ(w_i·y_i / (x - x_i)) / Σ(w_i / (x - x_i))
 *
 * 优点: 无极点、O(n) 求值、数值稳定、无需解线性方程组。
 */
QVector<double> RationalInterpolation::interpolate(
    QVector<double> xData, QVector<double> yData,
    QVector<double> xQuery)
{
    QElapsedTimer timer;
    timer.start();

    const int n = xData.size();
    const int m = xQuery.size();
    QVector<double> result(m, 0.0);

    /* 预计算 Berrut 第一型权重 w_i = (-1)^i */
    QVector<double> w(n);
    for (int i = 0; i < n; ++i)
        w[i] = (i % 2 == 0) ? 1.0 : -1.0;

    for (int q = 0; q < m; ++q) {
        double xq = xQuery[q];

        /* 检查是否恰好落在节点上 */
        bool exact = false;
        for (int i = 0; i < n; ++i) {
            if (std::abs(xq - xData[i]) < 1e-14) {
                result[q] = yData[i];
                exact = true;
                break;
            }
        }
        if (exact) continue;

        /* 重心公式 */
        double numer = 0.0;  ///< 分子 Σ w_i y_i / (x - x_i)
        double denom = 0.0;  ///< 分母 Σ w_i / (x - x_i)

        for (int i = 0; i < n; ++i) {
            double diff = xq - xData[i];
            double term = w[i] / diff;
            numer += term * yData[i];
            denom += term;
        }

        if (std::abs(denom) < 1e-300)
            result[q] = 0.0;  /* 回退 */
        else
            result[q] = numer / denom;
    }

    m_stats.totalInterpolations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalInterpolations;
    emit interpolationCompleted(m);
    return result;
}

/** @brief 重置统计 */
void RationalInterpolation::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
