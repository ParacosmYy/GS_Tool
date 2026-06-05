/**
 * @file DividedDifference.cpp
 * @brief 牛顿差商插值实现
 */

#include "utils/divided_diff/DividedDifference.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
DividedDifference::DividedDifference(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 构建 Newton 差商表
 *
 * 递推公式: f[x_i,...,x_{i+j}] = (f[x_{i+1},...,x_{i+j}]
 *           - f[x_i,...,x_{i+j-1}]) / (x_{i+j} - x_i)
 * 结果为下三角矩阵，主对角线即 Newton 插值系数。
 */
QVector<QVector<double>> DividedDifference::buildTable(
    QVector<double> xData, QVector<double> yData)
{
    QElapsedTimer timer;
    timer.start();

    const int n = xData.size();
    QVector<QVector<double>> table(n);
    for (int i = 0; i < n; ++i) {
        table[i].resize(n);
        table[i][0] = yData[i];
    }

    for (int j = 1; j < n; ++j) {
        for (int i = 0; i < n - j; ++i) {
            double denom = xData[i + j] - xData[i];
            if (std::abs(denom) < 1e-300)
                denom = 1e-300;
            table[i][j] = (table[i + 1][j - 1] - table[i][j - 1]) / denom;
        }
    }

    m_stats.totalInterpolations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalInterpolations;
    emit interpolationCompleted(n);
    return table;
}

/**
 * @brief 利用差商表求值 Newton 插值多项式
 *
 * P(x) = f[x₀] + f[x₀,x₁](x-x₀) + f[x₀,x₁,x₂](x-x₀)(x-x₁) + ...
 */
double DividedDifference::interpolate(
    QVector<double> xData,
    QVector<QVector<double>> table,
    double xQuery)
{
    QElapsedTimer timer;
    timer.start();

    const int n = xData.size();
    double result = table[0][0];
    double product = 1.0;

    for (int j = 1; j < n; ++j) {
        product *= (xQuery - xData[j - 1]);
        result += table[0][j] * product;
    }

    m_stats.totalInterpolations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalInterpolations;
    emit interpolationCompleted(n);
    return result;
}

/** @brief 重置统计 */
void DividedDifference::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
