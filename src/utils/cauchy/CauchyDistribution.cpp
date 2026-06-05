/**
 * @file CauchyDistribution.cpp
 * @brief 柯西分布实现
 */

#include "CauchyDistribution.h"
#include <QElapsedTimer>
#include <cmath>
#include <cstdlib>
#include <algorithm>

CauchyDistribution::CauchyDistribution(double location, double scale,
                                           QObject* parent)
    : QObject(parent)
    , m_location(location)
    , m_scale(qMax(1e-15, scale))
    , m_timeSum(0.0)
{
}

double CauchyDistribution::pdf(double x) const
{
    m_stats.totalEvaluations++;
    double z = (x - m_location) / m_scale;
    return 1.0 / (M_PI * m_scale * (1.0 + z * z));
}

double CauchyDistribution::cdf(double x) const
{
    m_stats.totalEvaluations++;
    return 0.5 + std::atan((x - m_location) / m_scale) / M_PI;
}

double CauchyDistribution::quantile(double p) const
{
    m_stats.totalEvaluations++;
    p = qBound(1e-10, p, 1.0 - 1e-10);
    return m_location + m_scale * std::tan(M_PI * (p - 0.5));
}

QVector<double> CauchyDistribution::sample(int n) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> samples;
    samples.reserve(n);
    for (int i = 0; i < n; ++i) {
        double u = static_cast<double>(std::rand()) / RAND_MAX;
        u = qBound(1e-10, u, 1.0 - 1e-10);
        samples.append(m_location + m_scale * std::tan(M_PI * (u - 0.5)));
    }

    const_cast<CauchyDistribution*>(this)->m_stats.totalSamples += n;
    const_cast<CauchyDistribution*>(this)->m_timeSum += timer.elapsed();
    const_cast<CauchyDistribution*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / qMax(1, m_stats.totalSamples + m_stats.totalEvaluations);

    emit const_cast<CauchyDistribution*>(this)->sampled(n);
    return samples;
}

QPair<double, double> CauchyDistribution::estimate(const QVector<double>& samples)
{
    int n = samples.size();
    if (n < 2) return {0.0, 1.0};

    QVector<double> sorted = samples;
    std::sort(sorted.begin(), sorted.end());

    double loc = sorted[n / 2];
    double q25 = sorted[n / 4];
    double q75 = sorted[3 * n / 4];
    double scale = (q75 - q25) / 2.0;

    return {loc, qMax(1e-15, scale)};
}

void CauchyDistribution::setLocation(double loc) { m_location = loc; }
void CauchyDistribution::setScale(double s) { m_scale = qMax(1e-15, s); }
double CauchyDistribution::location() const { return m_location; }
double CauchyDistribution::scale() const { return m_scale; }

CauchyDistribution::Stats CauchyDistribution::stats() const { return m_stats; }

void CauchyDistribution::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
