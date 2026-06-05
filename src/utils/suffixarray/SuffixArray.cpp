/**
 * @file SuffixArray.cpp
 * @brief 后缀数组实现
 */

#include "utils/suffixarray/SuffixArray.h"

#include <QElapsedTimer>
#include <algorithm>

SuffixArray::SuffixArray(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

QVector<int> SuffixArray::build(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    QVector<int> sa(n);
    for (int i = 0; i < n; ++i) sa[i] = i;

    /* 简单排序(对于大数据应用SA-IS) */
    std::sort(sa.begin(), sa.end(), [&data](int a, int b) {
        int n = data.size();
        while (a < n && b < n) {
            if (data[a] != data[b]) return static_cast<quint8>(data[a]) < static_cast<quint8>(data[b]);
            ++a; ++b;
        }
        return a >= n;
    });

    m_stats.totalBuilds++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalBuilds, 1ULL);

    emit buildCompleted(n);
    return sa;
}

QVector<int> SuffixArray::buildLcp(const QByteArray& data,
                                    const QVector<int>& sa)
{
    int n = sa.size();
    QVector<int> lcp(n, 0);
    QVector<int> rank(n);

    for (int i = 0; i < n; ++i) rank[sa[i]] = i;

    int h = 0;
    for (int i = 0; i < n; ++i) {
        if (rank[i] > 0) {
            int j = sa[rank[i] - 1];
            while (i + h < n && j + h < n &&
                   data[i + h] == data[j + h]) {
                ++h;
            }
            lcp[rank[i]] = h;
            if (h > 0) --h;
        }
    }

    return lcp;
}

QVector<int> SuffixArray::search(const QByteArray& data,
                                  const QVector<int>& sa,
                                  const QByteArray& pattern)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> results;
    int n = data.size();
    int m = pattern.size();
    if (m == 0 || n == 0) return results;

    /* 下界 */
    int lo = 0, hi = n;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        int cmp = 0;
        for (int i = 0; i < m && sa[mid] + i < n; ++i) {
            if (data[sa[mid] + i] != pattern[i]) {
                cmp = static_cast<quint8>(data[sa[mid] + i]) -
                      static_cast<quint8>(pattern[i]);
                break;
            }
        }
        if (cmp < 0) lo = mid + 1; else hi = mid;
    }
    int start = lo;

    /* 上界 */
    hi = n;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        int cmp = 0;
        for (int i = 0; i < m && sa[mid] + i < n; ++i) {
            if (data[sa[mid] + i] != pattern[i]) {
                cmp = static_cast<quint8>(data[sa[mid] + i]) -
                      static_cast<quint8>(pattern[i]);
                break;
            }
        }
        if (cmp <= 0) lo = mid + 1; else hi = mid;
    }

    for (int i = start; i < lo; ++i) {
        results.append(sa[i]);
    }

    m_stats.totalQueries++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalBuilds + m_stats.totalQueries, 1ULL);

    emit queryCompleted(results.size());
    return results;
}

QByteArray SuffixArray::longestRepeatedSubstring(
    const QByteArray& data, const QVector<int>& sa,
    const QVector<int>& lcp)
{
    int maxLen = 0, maxIdx = 0;
    for (int i = 1; i < lcp.size(); ++i) {
        if (lcp[i] > maxLen) {
            maxLen = lcp[i];
            maxIdx = sa[i];
        }
    }
    return data.mid(maxIdx, maxLen);
}

void SuffixArray::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
