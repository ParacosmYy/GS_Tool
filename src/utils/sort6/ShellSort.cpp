/**
 * @file ShellSort.cpp
 * @brief Shell排序实现
 */

#include "ShellSort.h"
#include <QElapsedTimer>
#include <algorithm>

ShellSort::ShellSort(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<int> ShellSort::generateGapSequence(int n, GapSequence seq) const
{
    QVector<int> gaps;

    switch (seq) {
    case Ciura: {
        /* Ciura 2001最优序列 */
        static const int ciura[] = {1, 4, 10, 23, 57, 132, 301, 701, 1750};
        for (int g : ciura) {
            if (g > n) break;
            gaps.prepend(g);
        }
        break;
    }
    case Sedgewick: {
        int k = 0;
        while (true) {
            int g = (k % 2 == 0)
                ? 9 * (1 << k) - 9 * (1 << (k / 2)) + 1
                : 8 * (1 << k) - 6 * (1 << ((k + 1) / 2)) + 1;
            if (g > n) break;
            gaps.prepend(g);
            k++;
        }
        break;
    }
    case Knuth: {
        int g = 1;
        while (g < n / 3) g = 3 * g + 1;
        while (g >= 1) {
            gaps.append(g);
            g /= 3;
        }
        break;
    }
    case Hibbard: {
        int k = 1;
        while (true) {
            int g = (1 << k) - 1;
            if (g > n) break;
            gaps.prepend(g);
            k++;
        }
        break;
    }
    }

    if (gaps.isEmpty()) gaps.append(1);
    return gaps;
}

void ShellSort::sort(QVector<double>& data, GapSequence seq)
{
    sortWithComparator(data, [](double a, double b) { return a < b; }, seq);
}

void ShellSort::sortWithComparator(QVector<double>& data,
                                    std::function<bool(double, double)> cmp,
                                    GapSequence seq)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    QVector<int> gaps = generateGapSequence(n, seq);

    for (int gap : gaps) {
        for (int i = gap; i < n; ++i) {
            double temp = data[i];
            int j = i;

            while (j >= gap) {
                m_stats.totalComparisons++;
                if (!cmp(temp, data[j - gap])) break;
                data[j] = data[j - gap];
                m_stats.totalSwaps++;
                j -= gap;
            }
            data[j] = temp;
        }
    }

    double elapsed = timer.elapsed();

    m_stats.totalSorted++;
    m_stats.totalElements += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorted;

    emit sortCompleted(n, elapsed);
}

QVector<int> ShellSort::sortIndices(const QVector<double>& data, GapSequence seq) const
{
    int n = data.size();
    QVector<int> indices(n);
    for (int i = 0; i < n; ++i) indices[i] = i;

    QVector<int> gaps = generateGapSequence(n, seq);

    for (int gap : gaps) {
        for (int i = gap; i < n; ++i) {
            int tempIdx = indices[i];
            double tempVal = data[tempIdx];
            int j = i;

            while (j >= gap && tempVal < data[indices[j - gap]]) {
                indices[j] = indices[j - gap];
                j -= gap;
            }
            indices[j] = tempIdx;
        }
    }

    return indices;
}

bool ShellSort::isSorted(const QVector<double>& data) const
{
    for (int i = 1; i < data.size(); ++i) {
        if (data[i] < data[i - 1]) return false;
    }
    return true;
}

ShellSort::Stats ShellSort::stats() const { return m_stats; }

void ShellSort::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
