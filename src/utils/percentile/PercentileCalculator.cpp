/**
 * @file PercentileCalculator.cpp
 * @brief 百分位数/分位数计算器实现
 */

#include "utils/percentile/PercentileCalculator.h"

#include <QElapsedTimer>
#include <algorithm>

PercentileCalculator::PercentileCalculator(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

double PercentileCalculator::compute(const QVector<double>& data, double p,
                                      Interpolation method) const
{
    if (data.isEmpty() || p < 0.0 || p > 100.0) return 0.0;

    QElapsedTimer timer;
    timer.start();

    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());

    int n = sorted.size();
    double idx = (p / 100.0) * (n - 1);
    int lo = qFloor(idx);
    int hi = qCeil(idx);
    double frac = idx - lo;

    double result = 0.0;
    lo = qBound(0, lo, n - 1);
    hi = qBound(0, hi, n - 1);

    switch (method) {
    case Interpolation::Linear:
        result = sorted[lo] + frac * (sorted[hi] - sorted[lo]);
        break;
    case Interpolation::Lower:
        result = sorted[lo];
        break;
    case Interpolation::Higher:
        result = sorted[hi];
        break;
    case Interpolation::Midpoint:
        result = (sorted[lo] + sorted[hi]) / 2.0;
        break;
    case Interpolation::Nearest:
        result = (frac < 0.5) ? sorted[lo] : sorted[hi];
        break;
    }

    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationCompleted(p, result);
    return result;
}

QVector<double> PercentileCalculator::computeMany(
    const QVector<double>& data, const QVector<double>& percentiles) const
{
    if (data.isEmpty()) return {};

    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());

    int n = sorted.size();
    QVector<double> results;
    results.reserve(percentiles.size());

    for (double p : percentiles) {
        double idx = (qBound(0.0, p, 100.0) / 100.0) * (n - 1);
        int lo = qBound(0, qFloor(idx), n - 1);
        int hi = qBound(0, qCeil(idx), n - 1);
        double frac = idx - qFloor(idx);
        results.append(sorted[lo] + frac * (sorted[hi] - sorted[lo]));
    }

    m_stats.totalComputations += percentiles.size();
    return results;
}

PercentileCalculator::BoxPlotStats PercentileCalculator::boxPlotStats(
    const QVector<double>& data, double kIqr) const
{
    BoxPlotStats stats;
    if (data.isEmpty()) return stats;

    QElapsedTimer timer;
    timer.start();

    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());

    int n = sorted.size();
    auto pctVal = [&](double p) -> double {
        double idx = (p / 100.0) * (n - 1);
        int lo = qBound(0, qFloor(idx), n - 1);
        int hi = qBound(0, qCeil(idx), n - 1);
        return sorted[lo] + (idx - qFloor(idx)) * (sorted[hi] - sorted[lo]);
    };

    stats.minimum = sorted.first();
    stats.maximum = sorted.last();
    stats.q1 = pctVal(25.0);
    stats.median = pctVal(50.0);
    stats.q3 = pctVal(75.0);
    stats.iqr = stats.q3 - stats.q1;
    stats.lowerFence = stats.q1 - kIqr * stats.iqr;
    stats.upperFence = stats.q3 + kIqr * stats.iqr;

    for (double v : sorted) {
        if (v < stats.lowerFence || v > stats.upperFence) {
            stats.outliers.append(v);
        }
    }

    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    return stats;
}

void PercentileCalculator::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
