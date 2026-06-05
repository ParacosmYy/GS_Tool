/**
 * @file OnlineQuantile.cpp
 * @brief 在线分位数估计器实现
 */

#include "OnlineQuantile.h"
#include <QElapsedTimer>
#include <algorithm>

OnlineQuantile::OnlineQuantile(double epsilon, QObject* parent)
    : QObject(parent)
    , m_epsilon(epsilon)
    , m_n(0)
    , m_timeSum(0.0)
{
}

void OnlineQuantile::insert(double value)
{
    QElapsedTimer timer;
    timer.start();

    m_n++;

    /* 找到插入位置 */
    int pos = 0;
    while (pos < m_summary.size() && m_summary[pos].value <= value)
        pos++;

    /* 计算g和delta */
    int g = 1;
    int delta = static_cast<int>(2.0 * m_epsilon * m_n);

    /* 插入新元组 */
    Tuple t;
    t.value = value;
    t.g = g;
    t.delta = delta;
    m_summary.insert(pos, t);

    /* 定期压缩 */
    if (m_n % static_cast<int>(1.0 / (2.0 * m_epsilon)) == 0)
        compress();

    m_stats.totalInsertions++;
    m_stats.tupleCount = m_summary.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalInsertions + m_stats.totalQueries);

    emit valueInserted(value, m_n);
}

double OnlineQuantile::query(double q) const
{
    if (m_summary.isEmpty()) return 0.0;

    int targetRank = static_cast<int>(q * m_n);
    int rank = 0;

    for (int i = 0; i < m_summary.size(); ++i) {
        rank += m_summary[i].g;
        if (rank + m_summary[i].delta >= targetRank) {
            return m_summary[i].value;
        }
    }

    return m_summary.last().value;
}

QMap<double, double> OnlineQuantile::queryBatch(
    const QVector<double>& quantiles) const
{
    QMap<double, double> result;
    for (double q : quantiles)
        result[q] = query(q);
    return result;
}

double OnlineQuantile::median() const { return query(0.5); }

double OnlineQuantile::quartile(int q) const
{
    if (q == 1) return query(0.25);
    if (q == 2) return query(0.50);
    if (q == 3) return query(0.75);
    return 0.0;
}

void OnlineQuantile::clear()
{
    m_summary.clear();
    m_n = 0;
}

int OnlineQuantile::count() const { return m_n; }

void OnlineQuantile::compress()
{
    if (m_summary.size() < 3) return;

    int threshold = static_cast<int>(2.0 * m_epsilon * m_n);
    int i = m_summary.size() - 2;

    while (i > 0) {
        int totalG = m_summary[i].g;
        int j = i - 1;
        while (j >= 0) {
            totalG += m_summary[j].g;
            if (totalG + m_summary[i + 1].delta > threshold) break;

            /* 合并: 删除j,将g累加到i */
            m_summary[i].g += m_summary[j].g;
            m_summary.removeAt(j);
            i--;
            j--;
        }
        i--;
    }
}

OnlineQuantile::Stats OnlineQuantile::stats() const { return m_stats; }

void OnlineQuantile::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
