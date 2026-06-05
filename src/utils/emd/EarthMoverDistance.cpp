/**
 * @file EarthMoverDistance.cpp
 * @brief EMD实现 — CDF差分法(1D)
 */

#include "utils/emd/EarthMoverDistance.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

EarthMoverDistance::EarthMoverDistance(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

double EarthMoverDistance::compute(const QVector<double>& dist1,
                                   const QVector<double>& dist2)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(dist1.size(), dist2.size());
    if (n == 0) return 0.0;

    double emd = 0.0;
    double cdf1 = 0.0;
    double cdf2 = 0.0;

    for (int i = 0; i < n; ++i) {
        cdf1 += dist1[i];
        cdf2 += dist2[i];
        emd += qAbs(cdf1 - cdf2);
    }

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalComputations;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    emit computationCompleted(emd);
    return emd;
}

double EarthMoverDistance::computeWeighted(
    const QVector<double>& values1,
    const QVector<double>& weights1,
    const QVector<double>& values2,
    const QVector<double>& weights2)
{
    QElapsedTimer timer;
    timer.start();

    /* 合并排序所有值 */
    QVector<QPair<double, double>> events;

    for (int i = 0; i < values1.size(); ++i)
        events.append({values1[i], weights1[i]});
    for (int i = 0; i < values2.size(); ++i)
        events.append({values2[i], -weights2[i]});

    std::sort(events.begin(), events.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    double emd = 0.0;
    double cdf = 0.0;
    double prevX = events.isEmpty() ? 0.0 : events[0].first;

    for (const auto& e : events) {
        emd += qAbs(cdf) * (e.first - prevX);
        cdf += e.second;
        prevX = e.first;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalComputations;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalComputations);

    emit computationCompleted(emd);
    return emd;
}

void EarthMoverDistance::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
