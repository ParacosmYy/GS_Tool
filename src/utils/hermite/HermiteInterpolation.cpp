/**
 * @file HermiteInterpolation.cpp
 * @brief Hermite插值实现 — 匹配值和导数
 */

#include "utils/hermite/HermiteInterpolation.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cmath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
HermiteInterpolation::HermiteInterpolation(QObject* parent)
    : QObject(parent)
{
}

/** @brief 多点Hermite插值(分段三次) */
QVector<double> HermiteInterpolation::interpolate(
    const QVector<double>& x,
    const QVector<double>& y,
    const QVector<double>& dy,
    const QVector<double>& xQuery)
{
    QElapsedTimer timer;
    timer.start();

    int n = x.size();
    if (n < 2 || y.size() != n || dy.size() != n || xQuery.isEmpty()) {
        return QVector<double>(xQuery.size(), 0.0);
    }

    QVector<double> results;
    results.reserve(xQuery.size());

    for (double xq : xQuery) {
        /* 找到xq所在的区间 */
        int idx = 0;
        for (int i = 0; i < n - 1; ++i) {
            if (xq >= x[i] && xq <= x[i + 1]) { idx = i; break; }
            if (xq > x[n - 1]) { idx = n - 2; break; }
        }

        /* 归一化参数 t ∈ [0,1] */
        double interval = x[idx + 1] - x[idx];
        double t = (std::abs(interval) > 1e-15)
            ? (xq - x[idx]) / interval : 0.0;
        t = qBound(0.0, t, 1.0);

        results.append(cubicHermite(x[idx], y[idx], dy[idx],
                                     x[idx + 1], y[idx + 1], dy[idx + 1], t));
    }

    m_stats.totalInterpolations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalInterpolations;

    emit interpolationCompleted(xQuery.size());
    return results;
}

/** @brief 单区间三次Hermite插值 */
double HermiteInterpolation::cubicHermite(double x0, double y0, double d0,
                                            double x1, double y1, double d1,
                                            double t) const
{
    double t2 = t * t;
    double t3 = t2 * t;

    /* Hermite基函数 */
    double h00 = 2.0 * t3 - 3.0 * t2 + 1.0;
    double h10 = t3 - 2.0 * t2 + t;
    double h01 = -2.0 * t3 + 3.0 * t2;
    double h11 = t3 - t2;

    double dx = x1 - x0;
    return h00 * y0 + h10 * dx * d0 + h01 * y1 + h11 * dx * d1;
}

/** @brief 重置统计 */
void HermiteInterpolation::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
