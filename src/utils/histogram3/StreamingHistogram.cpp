/**
 * @file StreamingHistogram.cpp
 * @brief 流式直方图 — 固定分箱的增量更新
 */

#include "StreamingHistogram.h"
#include <QElapsedTimer>
#include <cmath>

StreamingHistogram::StreamingHistogram(int nBins, double minVal, double maxVal,
                                         QObject* parent)
    : QObject(parent)
    , m_nBins(nBins > 0 ? nBins : 50)
    , m_minVal(minVal)
    , m_maxVal(maxVal)
    , m_binWidth((maxVal - minVal) / nBins)
    , m_counts(nBins > 0 ? nBins : 50, 0)
    , m_totalPoints(0)
    , m_timeSum(0.0)
{
}

void StreamingHistogram::update(double value)
{
    QElapsedTimer timer;
    timer.start();

    int bin = static_cast<int>((value - m_minVal) / m_binWidth);
    if (bin < 0) bin = 0;
    if (bin >= m_nBins) bin = m_nBins - 1;

    m_counts[bin]++;
    m_totalPoints++;

    m_stats.totalUpdates++;
    m_stats.totalDataPoints++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalUpdates;

    emit binUpdated(bin, m_counts[bin]);
}

void StreamingHistogram::updateBatch(const QVector<double>& values)
{
    for (double v : values) {
        update(v);
    }
}

void StreamingHistogram::merge(const StreamingHistogram& other)
{
    if (other.m_nBins != m_nBins) return;

    for (int i = 0; i < m_nBins; ++i) {
        m_counts[i] += other.m_counts[i];
    }
    m_totalPoints += other.m_totalPoints;
}

QVector<int> StreamingHistogram::binCounts() const
{
    return m_counts;
}

QVector<double> StreamingHistogram::binEdges() const
{
    QVector<double> edges;
    edges.reserve(m_nBins + 1);
    for (int i = 0; i <= m_nBins; ++i) {
        edges.append(m_minVal + i * m_binWidth);
    }
    return edges;
}

double StreamingHistogram::estimatePercentile(double p) const
{
    if (m_totalPoints == 0) return 0.0;

    double target = (p / 100.0) * m_totalPoints;
    double cumulative = 0.0;

    for (int i = 0; i < m_nBins; ++i) {
        cumulative += m_counts[i];
        if (cumulative >= target) {
            double frac = (target - (cumulative - m_counts[i])) / m_counts[i];
            return m_minVal + (i + frac) * m_binWidth;
        }
    }

    return m_maxVal;
}

void StreamingHistogram::reset()
{
    m_counts.fill(0);
    m_totalPoints = 0;
}

void StreamingHistogram::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
