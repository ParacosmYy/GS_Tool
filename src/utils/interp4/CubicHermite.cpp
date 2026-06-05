/**
 * @file CubicHermite.cpp
 * @brief 三次Hermite插值实现
 */

#include "CubicHermite.h"
#include <QElapsedTimer>
#include <cmath>

CubicHermite::CubicHermite(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

void CubicHermite::setData(const QVector<double>& x, const QVector<double>& y,
                              QVector<double> slopes)
{
    m_x = x;
    m_y = y;

    if (slopes.isEmpty())
        m_slopes = monotoneSlopes(x, y);
    else
        m_slopes = slopes;
}

double CubicHermite::interpolate(double x) const
{
    if (m_x.isEmpty()) return 0.0;
    if (x <= m_x.first()) return m_y.first();
    if (x >= m_x.last()) return m_y.last();

    int i = findSegment(x);
    double h = m_x[i + 1] - m_x[i];
    if (h < 1e-15) return m_y[i];

    double t = (x - m_x[i]) / h;
    return hermite(t, m_y[i], m_y[i + 1], m_slopes[i] * h, m_slopes[i + 1] * h);
}

QVector<double> CubicHermite::interpolateBatch(const QVector<double>& xPoints) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    result.reserve(xPoints.size());
    for (double x : xPoints)
        result.append(interpolate(x));

    const_cast<CubicHermite*>(this)->m_stats.totalInterpolated++;
    const_cast<CubicHermite*>(this)->m_stats.totalPoints += xPoints.size();
    const_cast<CubicHermite*>(this)->m_timeSum += timer.elapsed();
    const_cast<CubicHermite*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalInterpolated;

    emit const_cast<CubicHermite*>(this)->interpolationCompleted(xPoints.size());
    return result;
}

QVector<double> CubicHermite::monotoneSlopes(const QVector<double>& x,
                                               const QVector<double>& y)
{
    int n = x.size();
    QVector<double> slopes(n, 0.0);
    if (n < 2) return slopes;

    /* 计算差分 */
    QVector<double> delta(n - 1);
    for (int i = 0; i < n - 1; ++i) {
        double h = x[i + 1] - x[i];
        delta[i] = (h > 1e-15) ? (y[i + 1] - y[i]) / h : 0.0;
    }

    /* 初始斜率 = 相邻差分调和平均 */
    slopes[0] = delta[0];
    for (int i = 1; i < n - 1; ++i) {
        if (delta[i - 1] * delta[i] <= 0)
            slopes[i] = 0.0;
        else
            slopes[i] = (delta[i - 1] + delta[i]) / 2.0;
    }
    slopes[n - 1] = delta[n - 2];

    /* Fritsch-Carlson单调性修正 */
    for (int i = 0; i < n - 1; ++i) {
        if (std::abs(delta[i]) < 1e-15) {
            slopes[i] = 0.0;
            slopes[i + 1] = 0.0;
        } else {
            double alpha = slopes[i] / delta[i];
            double beta = slopes[i + 1] / delta[i];
            double s = alpha * alpha + beta * beta;
            if (s > 9.0) {
                double t = 3.0 / std::sqrt(s);
                slopes[i] = t * alpha * delta[i];
                slopes[i + 1] = t * beta * delta[i];
            }
        }
    }

    return slopes;
}

double CubicHermite::hermite(double t, double p0, double p1,
                                double m0, double m1)
{
    double t2 = t * t, t3 = t2 * t;
    double h00 = 2 * t3 - 3 * t2 + 1;
    double h10 = t3 - 2 * t2 + t;
    double h01 = -2 * t3 + 3 * t2;
    double h11 = t3 - t2;
    return h00 * p0 + h10 * m0 + h01 * p1 + h11 * m1;
}

int CubicHermite::findSegment(double x) const
{
    int lo = 0, hi = m_x.size() - 2;
    while (lo < hi) {
        int mid = (lo + hi + 1) / 2;
        if (m_x[mid] <= x) lo = mid;
        else hi = mid - 1;
    }
    return lo;
}

CubicHermite::Stats CubicHermite::stats() const { return m_stats; }

void CubicHermite::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
