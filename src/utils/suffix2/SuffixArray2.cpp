/**
 * @file SuffixArray2.cpp
 * @brief 后缀数组(简化SA-IS)实现
 */

#include "utils/suffix2/SuffixArray2.h"

#include <QElapsedTimer>
#include <algorithm>

SuffixArray2::SuffixArray2(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<int> SuffixArray2::build(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    int n = text.length();
    if (n == 0) {
        m_lastLcp.clear();
        emit buildCompleted(0);
        return {};
    }

    /* 将QString转为整数数组 */
    QVector<int> input(n + 1);
    int maxChar = 0;
    for (int i = 0; i < n; ++i) {
        input[i] = text[i].unicode();
        maxChar = qMax(maxChar, input[i]);
    }
    input[n] = 0; /* 哨兵 */
    int alphabetSize = maxChar + 2;

    QVector<int> sa = saisBuild(input, alphabetSize);

    /* 构建LCP */
    m_lastLcp = buildLcpImpl(sa, text);

    /* 移除哨兵位置 */
    if (!sa.isEmpty() && sa.last() == n) {
        sa.removeLast();
    }
    if (!m_lastLcp.isEmpty()) {
        m_lastLcp.removeLast();
    }

    m_stats.totalBuilt++;
    m_timeSum += timer.elapsed();
    double total = m_stats.totalBuilt + m_stats.totalSearches;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit buildCompleted(n);
    return sa;
}

QVector<int> SuffixArray2::saisBuild(const QVector<int>& input,
                                       int alphabetSize) const
{
    int n = input.size();
    QVector<int> sa(n, -1);

    /* 简化实现: 使用前缀倍增法(O(n log^2 n)) */
    QVector<int> rank(n), tmp(n);
    for (int i = 0; i < n; ++i) {
        rank[i] = input[i];
        sa[i] = i;
    }

    for (int gap = 1; gap < n; gap *= 2) {
        auto cmp = [&](int a, int b) -> bool {
            if (rank[a] != rank[b]) return rank[a] < rank[b];
            int ra = (a + gap < n) ? rank[a + gap] : -1;
            int rb = (b + gap < n) ? rank[b + gap] : -1;
            return ra < rb;
        };
        std::sort(sa.begin(), sa.end(), cmp);

        tmp[sa[0]] = 0;
        for (int i = 1; i < n; ++i) {
            tmp[sa[i]] = tmp[sa[i - 1]];
            if (cmp(sa[i - 1], sa[i])) tmp[sa[i]]++;
        }
        rank = tmp;
        if (rank[sa[n - 1]] == n - 1) break;
    }

    return sa;
}

QVector<int> SuffixArray2::buildLcpImpl(const QVector<int>& sa,
                                          const QString& text) const
{
    int n = sa.size();
    if (n == 0) return {};

    /* Kasai算法 */
    QVector<int> rank(n);
    for (int i = 0; i < n; ++i) {
        rank[sa[i]] = i;
    }

    QVector<int> lcp(n, 0);
    int h = 0;
    for (int i = 0; i < n; ++i) {
        if (rank[i] > 0) {
            int j = sa[rank[i] - 1];
            while (i + h < n && j + h < n &&
                   text[i + h] == text[j + h]) {
                ++h;
            }
            lcp[rank[i]] = h;
            if (h > 0) --h;
        } else {
            h = 0;
        }
    }
    return lcp;
}

QVector<int> SuffixArray2::lcpArray() const
{
    return m_lastLcp;
}

QVector<int> SuffixArray2::search(const QString& pattern,
                                    const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    int n = text.length();
    int m = pattern.length();
    if (m == 0 || n == 0 || m > n) {
        m_stats.totalSearches++;
        m_timeSum += timer.elapsed();
        double total = m_stats.totalBuilt + m_stats.totalSearches;
        m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;
        return result;
    }

    /* 构建后缀数组 */
    QVector<int> sa = build(text);

    /* 二分搜索找下界 */
    int lo = 0, hi = n - m;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        QString suffix = text.mid(sa[mid], qMin(m, n - sa[mid]));
        if (suffix < pattern) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }

    /* 从下界开始收集所有匹配 */
    for (int i = lo; i < n; ++i) {
        if (sa[i] + m > n) break;
        bool match = true;
        for (int j = 0; j < m; ++j) {
            if (text[sa[i] + j] != pattern[j]) {
                match = false;
                break;
            }
        }
        if (!match) break;
        result.append(sa[i]);
    }

    m_stats.totalSearches++;
    m_timeSum += timer.elapsed();
    double total = m_stats.totalBuilt + m_stats.totalSearches;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    return result;
}

void SuffixArray2::radixSort(QVector<int>& data,
                               const QVector<int>& keys,
                               int alphabetSize) const
{
    Q_UNUSED(data);
    Q_UNUSED(keys);
    Q_UNUSED(alphabetSize);
    /* 前缀倍增法中不需要单独的基数排序 */
}

void SuffixArray2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_lastLcp.clear();
}
