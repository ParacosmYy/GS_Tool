/**
 * @file DynamicHistogram.cpp
 * @brief 动态直方图构建器实现
 */

#include "utils/dynhistogram/DynamicHistogram.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

DynamicHistogram::DynamicHistogram(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

DynamicHistogram::Histogram DynamicHistogram::build(
    const QVector<double>& data, int nBins, BinStrategy strategy)
{
    QElapsedTimer timer;
    timer.start();

    Histogram result;
    result.totalPoints = data.size();
    result.nBins = nBins;

    if (data.isEmpty() || nBins < 1) {
        m_stats.totalBuilds++;
        return result;
    }

    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());

    double minVal = sorted.first();
    double maxVal = sorted.last();

    switch (strategy) {
    case BinStrategy::EqualWidth: {
        double range = maxVal - minVal;
        double binWidth = (range > 0) ? range / nBins : 1.0;

        result.binEdges.resize(nBins + 1);
        for (int i = 0; i <= nBins; ++i) {
            result.binEdges[i] = minVal + i * binWidth;
        }

        result.counts.resize(nBins, 0);
        for (double v : sorted) {
            int bin = static_cast<int>((v - minVal) / binWidth);
            if (bin >= nBins) bin = nBins - 1;
            result.counts[bin]++;
        }
        break;
    }
    case BinStrategy::EqualFrequency: {
        result.counts.resize(nBins, 0);
        result.binEdges.resize(nBins + 1);
        result.binEdges[0] = minVal;
        result.binEdges[nBins] = maxVal;

        int perBin = data.size() / nBins;
        int extra = data.size() % nBins;
        int idx = 0;
        for (int i = 0; i < nBins; ++i) {
            int count = perBin + (i < extra ? 1 : 0);
            result.counts[i] = count;
            idx += count;
            if (i < nBins - 1) {
                result.binEdges[i + 1] = (idx < sorted.size())
                    ? sorted[idx] : maxVal;
            }
        }
        break;
    }
    case BinStrategy::Custom:
        return build(data, nBins, BinStrategy::EqualWidth);
    }

    result.densities.resize(nBins);
    for (int i = 0; i < nBins; ++i) {
        double width = result.binEdges[i + 1] - result.binEdges[i];
        width = qMax(width, 1e-15);
        result.densities[i] = static_cast<double>(result.counts[i]) /
                              (result.totalPoints * width);
    }

    double sum = 0.0;
    for (double v : data) sum += v;
    result.mean = sum / data.size();

    double sum2 = 0.0, sum3 = 0.0, sum4 = 0.0;
    for (double v : data) {
        double d = v - result.mean;
        sum2 += d * d;
        sum3 += d * d * d;
        sum4 += d * d * d * d;
    }
    double n = data.size();
    result.variance = sum2 / n;
    double stdDev = qSqrt(result.variance);
    result.skewness = (stdDev > 1e-15)
        ? (sum3 / n) / (stdDev * stdDev * stdDev) : 0.0;
    result.kurtosis = (result.variance > 1e-30)
        ? (sum4 / n) / (result.variance * result.variance) - 3.0 : 0.0;

    m_stats.totalBuilds++;
    m_stats.totalDataPoints += data.size();
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalBuilds, 1ULL);

    emit buildCompleted(nBins, data.size());
    return result;
}

double DynamicHistogram::percentile(
    const QVector<double>& data, double p) const
{
    if (data.isEmpty()) return 0.0;
    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());

    double idx = (p / 100.0) * (sorted.size() - 1);
    int lo = qFloor(idx);
    int hi = qCeil(idx);
    if (lo == hi || hi >= sorted.size()) return sorted[lo];

    double frac = idx - lo;
    return sorted[lo] * (1.0 - frac) + sorted[hi] * frac;
}

int DynamicHistogram::sturgesBins(int dataSize) const
{
    return qMax(5, static_cast<int>(qCeil(qLn(dataSize) / qLn(2.0) + 1.0)));
}

int DynamicHistogram::freedmanDiaconisBins(
    const QVector<double>& data) const
{
    if (data.size() < 2) return 5;

    QVector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());

    int n = sorted.size();
    double q1Idx = 0.25 * (n - 1);
    double q3Idx = 0.75 * (n - 1);

    int q1Lo = qFloor(q1Idx);
    int q1Hi = qMin(qCeil(q1Idx), n - 1);
    double q1 = sorted[q1Lo] * (1.0 - (q1Idx - q1Lo)) +
                sorted[q1Hi] * (q1Idx - q1Lo);

    int q3Lo = qFloor(q3Idx);
    int q3Hi = qMin(qCeil(q3Idx), n - 1);
    double q3 = sorted[q3Lo] * (1.0 - (q3Idx - q3Lo)) +
                sorted[q3Hi] * (q3Idx - q3Lo);

    double iqr = q3 - q1;
    if (iqr < 1e-15) return sturgesBins(n);

    double binWidth = 2.0 * iqr / qPow(n, 1.0 / 3.0);
    double range = sorted.last() - sorted.first();
    if (binWidth < 1e-15) return sturgesBins(n);

    return qMax(5, static_cast<int>(qCeil(range / binWidth)));
}

void DynamicHistogram::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
