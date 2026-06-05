/**
 * @file LagrangeInterpolation.cpp
 * @brief Lagrange多项式插值实现
 */

#include "utils/lagrange/LagrangeInterpolation.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
LagrangeInterpolation::LagrangeInterpolation(QObject* parent)
    : QObject(parent)
{
}

/** @brief 执行Lagrange插值 */
QVector<double> LagrangeInterpolation::interpolate(
    const QVector<double>& x,
    const QVector<double>& y,
    const QVector<double>& xQuery)
{
    QElapsedTimer timer;
    timer.start();

    int n = x.size();
    if (n < 2 || y.size() != n || xQuery.isEmpty())
        return QVector<double>(xQuery.size(), 0.0);

    QVector<double> results;
    results.reserve(xQuery.size());

    for (double xq : xQuery) {
        double sum = 0.0;
        for (int i = 0; i < n; ++i) {
            double basis = y[i];
            for (int j = 0; j < n; ++j) {
                if (j != i) {
                    double denom = x[i] - x[j];
                    if (std::abs(denom) < 1e-15) {
                        basis = 0.0;
                        break;
                    }
                    basis *= (xq - x[j]) / denom;
                }
            }
            sum += basis;
        }
        results.append(sum);
    }

    m_stats.totalInterpolations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalInterpolations;

    emit interpolationCompleted(xQuery.size());
    return results;
}

/** @brief 重置统计 */
void LagrangeInterpolation::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
