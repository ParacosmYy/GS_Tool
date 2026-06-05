/**
 * @file CatmullRomSpline.cpp
 * @brief Catmull-Rom样条实现
 */

#include "CatmullRomSpline.h"
#include <QElapsedTimer>
#include <cmath>

CatmullRomSpline::CatmullRomSpline(QObject* parent)
    : QObject(parent)
    , m_closed(false)
    , m_param(Centripetal)
    , m_timeSum(0.0)
{
}

void CatmullRomSpline::setPoints(const QVector<double>& x,
                                     const QVector<double>& y,
                                     bool closed, Parameterization param)
{
    m_x = x;
    m_y = y;
    m_closed = closed;
    m_param = param;

    /* 计算节点参数 */
    int n = x.size();
    m_knots.resize(n);
    m_knots[0] = 0.0;

    for (int i = 1; i < n; ++i) {
        double dx = x[i] - x[i - 1];
        double dy = y[i] - y[i - 1];
        double dist = std::sqrt(dx * dx + dy * dy);

        if (m_param == Centripetal)
            dist = std::sqrt(dist);

        m_knots[i] = m_knots[i - 1] + qMax(1e-10, dist);
    }
}

QPair<double, double> CatmullRomSpline::evaluate(double t) const
{
    if (m_x.size() < 2) return {0.0, 0.0};

    int n = m_x.size();
    t = qBound(0.0, t, static_cast<double>(n - 1));

    int i0 = qMax(0, qMin(n - 1, static_cast<int>(std::floor(t))));

    int i_minus1 = (i0 > 0) ? i0 - 1 : (m_closed ? n - 1 : 0);
    int i1 = qMin(i0 + 1, n - 1);
    int i2 = (i1 < n - 1) ? i1 + 1 : (m_closed ? 0 : n - 1);

    double frac = t - i0;

    /* Catmull-Rom矩阵 */
    double t2 = frac * frac, t3 = t2 * frac;

    double rx = 0.5 * ((2.0 * m_x[i0]) +
                        (-m_x[i_minus1] + m_x[i1]) * frac +
                        (2.0 * m_x[i_minus1] - 5.0 * m_x[i0] + 4.0 * m_x[i1] - m_x[i2]) * t2 +
                        (-m_x[i_minus1] + 3.0 * m_x[i0] - 3.0 * m_x[i1] + m_x[i2]) * t3);

    double ry = 0.5 * ((2.0 * m_y[i0]) +
                        (-m_y[i_minus1] + m_y[i1]) * frac +
                        (2.0 * m_y[i_minus1] - 5.0 * m_y[i0] + 4.0 * m_y[i1] - m_y[i2]) * t2 +
                        (-m_y[i_minus1] + 3.0 * m_y[i0] - 3.0 * m_y[i1] + m_y[i2]) * t3);

    return {rx, ry};
}

QVector<QPair<double, double>> CatmullRomSpline::sample(int nSamples) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, double>> result;
    int n = segments();

    for (int seg = 0; seg < n; ++seg) {
        for (int j = 0; j < nSamples; ++j) {
            double t = seg + static_cast<double>(j) / nSamples;
            result.append(evaluate(t));
        }
    }
    result.append(evaluate(n));

    const_cast<CatmullRomSpline*>(this)->m_stats.totalEvaluated++;
    const_cast<CatmullRomSpline*>(this)->m_stats.totalPoints += m_x.size();
    const_cast<CatmullRomSpline*>(this)->m_timeSum += timer.elapsed();
    const_cast<CatmullRomSpline*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalEvaluated;

    emit const_cast<CatmullRomSpline*>(this)->evaluated(m_x.size());
    return result;
}

int CatmullRomSpline::segments() const
{
    return qMax(0, m_x.size() - 1);
}

CatmullRomSpline::Stats CatmullRomSpline::stats() const { return m_stats; }

void CatmullRomSpline::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
