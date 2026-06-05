/**
 * @file DataProfiler.cpp
 * @brief 数据画像引擎实现
 */

#include "utils/profiler/DataProfiler.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <set>

DataProfiler::DataProfiler(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

DataProfiler::Profile DataProfiler::profile(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    Profile p;
    p.count = data.size();
    if (data.isEmpty()) return p;

    double sum = 0.0;
    p.min = data[0]; p.max = data[0];
    std::set<double> unique;

    for (double v : data) {
        sum += v;
        if (v < p.min) p.min = v;
        if (v > p.max) p.max = v;
        if (v == 0.0) ++p.zeroCount;
        if (qIsNaN(v)) ++p.nanCount;
        unique.insert(v);
    }
    p.uniqueValues = static_cast<int>(unique.size());
    p.mean = sum / data.size();

    double sqSum = 0.0;
    for (double v : data) {
        double d = v - p.mean;
        sqSum += d * d;
    }
    p.stddev = qSqrt(sqSum / data.size());

    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());
    int n = sorted.size();
    p.median = (n % 2 == 0) ? (sorted[n/2-1] + sorted[n/2]) / 2.0 : sorted[n/2];
    p.q1 = sorted[n/4];
    p.q3 = sorted[3*n/4];

    /* Skewness and Kurtosis */
    if (p.stddev > 1e-10) {
        double m3 = 0.0, m4 = 0.0;
        for (double v : data) {
            double d = (v - p.mean) / p.stddev;
            m3 += d * d * d;
            m4 += d * d * d * d;
        }
        p.skewness = m3 / n;
        p.kurtosis = m4 / n - 3.0;
    }

    double elapsed = timer.elapsed();
    ++m_stats.totalProfiles;
    m_stats.totalPointsProfiled += n;
    m_timeSum += elapsed;
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalProfiles;

    emit profileComplete(p);
    return p;
}

void DataProfiler::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
