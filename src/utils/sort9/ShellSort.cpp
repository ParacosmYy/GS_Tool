/**
 * @file ShellSort.cpp
 * @brief Shell排序实现 — Ciura/Tokuda间隙序列
 */

#include "ShellSort.h"

#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

/* ---------- 构造函数 ---------- */

ShellSort::ShellSort(QObject* parent)
    : QObject(parent)
{
}

/* ---------- QVector<double> 排序 ---------- */

QVector<double> ShellSort::sort(const QVector<double>& data,
                                  bool ascending,
                                  GapSequence seq) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> arr = data;
    int n = arr.size();

    if (n <= 1) {
        m_stats.totalSorts++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorts;
        return arr;
    }

    QVector<int> gaps = generateGapSequence(n, seq);
    int comparisons = 0;
    int swaps = 0;

    for (int gap : gaps) {
        for (int i = gap; i < n; ++i) {
            double temp = arr[i];
            int j = i;

            while (j >= gap) {
                comparisons++;
                bool shouldSwap = ascending
                    ? (arr[j - gap] > temp)
                    : (arr[j - gap] < temp);

                if (shouldSwap) {
                    arr[j] = arr[j - gap];
                    swaps++;
                    j -= gap;
                } else {
                    break;
                }
            }
            arr[j] = temp;
        }
    }

    m_stats.totalSorts++;
    m_stats.totalElements += n;
    m_stats.totalComparisons += comparisons;
    m_stats.totalSwaps += swaps;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorts;

    emit sortCompleted(n, comparisons, swaps);
    return arr;
}

/* ---------- QStringList 排序 ---------- */

QStringList ShellSort::sortStrings(const QStringList& data,
                                     bool caseSensitive,
                                     GapSequence seq) const
{
    QElapsedTimer timer;
    timer.start();

    QStringList result = data;
    int n = result.size();

    if (n <= 1) {
        m_stats.totalSorts++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorts;
        return result;
    }

    QVector<int> gaps = generateGapSequence(n, seq);
    int comparisons = 0;
    int swaps = 0;

    for (int gap : gaps) {
        for (int i = gap; i < n; ++i) {
            QString temp = result[i];
            int j = i;

            while (j >= gap) {
                comparisons++;
                int cmp = caseSensitive
                    ? result[j - gap].compare(temp)
                    : result[j - gap].toLower().compare(temp.toLower());

                if (cmp > 0) {
                    result[j] = result[j - gap];
                    swaps++;
                    j -= gap;
                } else {
                    break;
                }
            }
            result[j] = temp;
        }
    }

    m_stats.totalSorts++;
    m_stats.totalElements += n;
    m_stats.totalComparisons += comparisons;
    m_stats.totalSwaps += swaps;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorts;

    emit sortCompleted(n, comparisons, swaps);
    return result;
}

/* ---------- QVector<int> 排序 ---------- */

QVector<int> ShellSort::sortInt(const QVector<int>& data,
                                  bool ascending,
                                  GapSequence seq) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> arr = data;
    int n = arr.size();

    if (n <= 1) {
        m_stats.totalSorts++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorts;
        return arr;
    }

    QVector<int> gaps = generateGapSequence(n, seq);
    int comparisons = 0;
    int swaps = 0;

    for (int gap : gaps) {
        for (int i = gap; i < n; ++i) {
            int temp = arr[i];
            int j = i;

            while (j >= gap) {
                comparisons++;
                bool shouldSwap = ascending
                    ? (arr[j - gap] > temp)
                    : (arr[j - gap] < temp);

                if (shouldSwap) {
                    arr[j] = arr[j - gap];
                    swaps++;
                    j -= gap;
                } else {
                    break;
                }
            }
            arr[j] = temp;
        }
    }

    m_stats.totalSorts++;
    m_stats.totalElements += n;
    m_stats.totalComparisons += comparisons;
    m_stats.totalSwaps += swaps;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorts;

    emit sortCompleted(n, comparisons, swaps);
    return arr;
}

/* ---------- 带索引追踪的排序 ---------- */

QVector<QPair<double, int>> ShellSort::sortWithIndex(
    const QVector<double>& data,
    bool ascending,
    GapSequence seq) const
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    QVector<QPair<double, int>> arr;
    arr.reserve(n);
    for (int i = 0; i < n; ++i) {
        arr.append({data[i], i});
    }

    if (n <= 1) {
        m_stats.totalSorts++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorts;
        return arr;
    }

    QVector<int> gaps = generateGapSequence(n, seq);
    int comparisons = 0;
    int swaps = 0;

    for (int gap : gaps) {
        for (int i = gap; i < n; ++i) {
            auto temp = arr[i];
            int j = i;

            while (j >= gap) {
                comparisons++;
                bool shouldSwap = ascending
                    ? (arr[j - gap].first > temp.first)
                    : (arr[j - gap].first < temp.first);

                if (shouldSwap) {
                    arr[j] = arr[j - gap];
                    swaps++;
                    j -= gap;
                } else {
                    break;
                }
            }
            arr[j] = temp;
        }
    }

    m_stats.totalSorts++;
    m_stats.totalElements += n;
    m_stats.totalComparisons += comparisons;
    m_stats.totalSwaps += swaps;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorts;

    emit sortCompleted(n, comparisons, swaps);
    return arr;
}

/* ---------- 生成间隙序列 ---------- */

QVector<int> ShellSort::generateGapSequence(int n, GapSequence seq) const
{
    switch (seq) {
    case GapSequence::Ciura:     return ciuraSequence(n);
    case GapSequence::Tokuda:    return tokudaSequence(n);
    case GapSequence::Hibbard:   return hibbardSequence(n);
    case GapSequence::Sedgewick: return sedgewickSequence(n);
    case GapSequence::Shell:     return shellSequence(n);
    }
    return ciuraSequence(n);
}

/* ---------- Ciura序列 ---------- */

QVector<int> ShellSort::ciuraSequence(int n) const
{
    /* Ciura经验最优序列, 扩展到大于701 */
    QVector<int> base = {1, 4, 10, 23, 57, 132, 301, 701};

    /* 扩展: 701 * 2.25 向上取整 */
    while (base.last() * 2.25 < n) {
        base.append(static_cast<int>(base.last() * 2.25));
    }

    /* 移除>=n的间隙, 降序排列 */
    QVector<int> gaps;
    for (int i = base.size() - 1; i >= 0; --i) {
        if (base[i] < n) gaps.append(base[i]);
    }
    return gaps;
}

/* ---------- Tokuda序列 ---------- */

QVector<int> ShellSort::tokudaSequence(int n) const
{
    QVector<int> gaps;
    int k = 1;
    while (true) {
        int gap = static_cast<int>(std::ceil(9.0 * std::pow(2.25, k - 1) - 4.0));
        if (gap >= n) break;
        gaps.prepend(gap);
        k++;
    }
    /* 确保1在末尾 */
    if (gaps.isEmpty() || gaps.last() != 1) {
        gaps.append(1);
    }
    /* 降序排列 */
    std::sort(gaps.begin(), gaps.end(), std::greater<int>());
    return gaps;
}

/* ---------- Hibbard序列 ---------- */

QVector<int> ShellSort::hibbardSequence(int n) const
{
    QVector<int> gaps;
    int k = 1;
    while (true) {
        int gap = (1 << k) - 1; /* 2^k - 1 */
        if (gap >= n) break;
        gaps.prepend(gap);
        k++;
    }
    if (gaps.isEmpty() || gaps.last() != 1) {
        gaps.append(1);
    }
    return gaps;
}

/* ---------- Sedgewick序列 ---------- */

QVector<int> ShellSort::sedgewickSequence(int n) const
{
    QVector<int> gaps;
    int k = 0;
    while (true) {
        int gap;
        if (k % 2 == 0) {
            int j = k / 2;
            gap = static_cast<int>(9 * std::pow(4, j) - 9 * std::pow(2, j) + 1);
        } else {
            int j = (k + 1) / 2;
            gap = static_cast<int>(std::pow(4, j) - 3 * std::pow(2, j) + 1);
        }
        if (gap >= n) break;
        gaps.prepend(gap);
        k++;
    }
    if (gaps.isEmpty() || gaps.last() != 1) {
        gaps.append(1);
    }
    return gaps;
}

/* ---------- Shell原始序列 ---------- */

QVector<int> ShellSort::shellSequence(int n) const
{
    QVector<int> gaps;
    int gap = n / 2;
    while (gap > 0) {
        gaps.append(gap);
        gap /= 2;
    }
    return gaps;
}

/* ---------- 统计 ---------- */

ShellSort::Stats ShellSort::stats() const { return m_stats; }

void ShellSort::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
