/**
 * @file StreamingQuantile2.cpp
 * @brief 流式分位数估计器实现
 */

#include "StreamingQuantile2.h"
#include <QElapsedTimer>
#include <algorithm>

StreamingQuantile2::StreamingQuantile2(double epsilon, QObject* parent)
    : QObject(parent)
    , m_epsilon(qBound(0.001, epsilon, 0.1))
    , m_n(0)
    , m_timeSum(0.0)
{
}

void StreamingQuantile2::insert(double value)
{
    QElapsedTimer timer;
    timer.start();

    if (m_n == 0) {
        m_summary.append({value, 1, 0});
        m_n = 1;

        m_stats.totalInserted++;
        m_stats.currentTuples = m_summary.size();
        m_timeSum += timer.elapsed();
        int total = m_stats.totalInserted + m_stats.totalQueries;
        if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

        emit valueInserted(value, m_n);
        return;
    }

    /* 找插入位置 */
    int idx = 0;
    while (idx < m_summary.size() && m_summary[idx].value < value)
        idx++;

    int delta = 0;
    if (idx > 0 && idx < m_summary.size()) {
        delta = static_cast<int>(2 * m_epsilon * m_n);
    }

    Tuple t{value, 1, delta};

    if (idx < m_summary.size())
        m_summary.insert(idx, t);
    else
        m_summary.append(t);

    m_n++;

    /* 周期性压缩 */
    int compressInterval = static_cast<int>(1.0 / (2.0 * m_epsilon));
    if (m_n % qMax(1, compressInterval) == 0)
        compress();

    m_stats.totalInserted++;
    m_stats.currentTuples = m_summary.size();
    m_timeSum += timer.elapsed();
    int total = m_stats.totalInserted + m_stats.totalQueries;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit valueInserted(value, m_n);
}

void StreamingQuantile2::compress()
{
    if (m_summary.size() < 3) return;

    int threshold = static_cast<int>(2 * m_epsilon * m_n);
    int i = m_summary.size() - 2;

    while (i > 0) {
        int sumGap = 0;
        int j = i;
        while (j < m_summary.size() - 1) {
            sumGap += m_summary[j].gap;
            if (sumGap + m_summary[j + 1].gap + m_summary[j + 1].delta > threshold) {
                break;
            }
            j++;
        }

        if (sumGap + m_summary[i].delta <= threshold && j > i) {
            /* 合并i到j-1 */
            int totalGap = 0;
            for (int k = i; k < j; ++k)
                totalGap += m_summary[k].gap;

            m_summary[i].gap += totalGap;
            m_summary.remove(i + 1, j - i);
        }
        i--;
    }
}

double StreamingQuantile2::query(double phi) const
{
    QElapsedTimer timer;
    timer.start();

    if (m_summary.isEmpty()) return 0.0;

    phi = qBound(0.0, phi, 1.0);
    int targetRank = static_cast<int>(phi * m_n);
    int maxError = static_cast<int>(m_epsilon * m_n);

    int cumulativeGap = 0;
    double bestValue = m_summary.first().value;

    for (int i = 0; i < m_summary.size(); ++i) {
        cumulativeGap += m_summary[i].gap;
        if (cumulativeGap + m_summary[i].delta > targetRank + maxError) break;
        bestValue = m_summary[i].value;
    }

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalInserted + m_stats.totalQueries;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return bestValue;
}

void StreamingQuantile2::insertBatch(const QVector<double>& values)
{
    for (double v : values)
        insert(v);
}

double StreamingQuantile2::median() const { return query(0.5); }
double StreamingQuantile2::q1() const { return query(0.25); }
double StreamingQuantile2::q3() const { return query(0.75); }
int StreamingQuantile2::tupleCount() const { return m_summary.size(); }

void StreamingQuantile2::reset()
{
    m_summary.clear();
    m_n = 0;
}

StreamingQuantile2::Stats StreamingQuantile2::stats() const { return m_stats; }

void StreamingQuantile2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
