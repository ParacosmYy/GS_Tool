/**
 * @file Introsort2.cpp
 * @brief Introsort自适应排序实现
 */

#include "Introsort2.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

Introsort2::Introsort2(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
    , m_comparisons(0)
{
}

QVector<double> Introsort2::sort(const QVector<double>& data)
{
    QVector<double> result = data;
    sortInPlace(result);
    return result;
}

void Introsort2::sortInPlace(QVector<double>& data)
{
    if (data.size() <= 1) return;

    QElapsedTimer timer;
    timer.start();
    m_comparisons = 0;

    int maxDepth = 2 * static_cast<int>(std::log2(data.size()));
    introsortLoop(data, 0, data.size() - 1, maxDepth);
    insertionSort(data, 0, data.size() - 1);

    m_stats.totalSorts++;
    m_stats.totalElementsSorted += data.size();
    m_stats.totalComparisons += m_comparisons;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorts;

    emit sorted(data.size(), m_stats.avgProcessingTimeMs);
}

void Introsort2::sortCustom(QVector<double>& data,
                              std::function<bool(double, double)> cmp)
{
    if (data.size() <= 1) return;
    QElapsedTimer timer;
    timer.start();
    std::stable_sort(data.begin(), data.end(), cmp);
    m_stats.totalSorts++;
    m_stats.totalElementsSorted += data.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorts;
}

void Introsort2::introsortLoop(QVector<double>& data, int lo, int hi,
                                 int depth)
{
    while (hi - lo > 16) {
        if (depth == 0) {
            heapSort(data, lo, hi);
            return;
        }
        depth--;

        int pivot = medianOf3(data, lo, lo + (hi - lo) / 2, hi);
        std::swap(data[pivot], data[hi]);

        double pval = data[hi];
        int i = lo - 1;
        for (int j = lo; j < hi; ++j) {
            m_comparisons++;
            if (data[j] <= pval) {
                ++i;
                std::swap(data[i], data[j]);
            }
        }
        std::swap(data[i + 1], data[hi]);
        int p = i + 1;

        /* 递归较小的一半,迭代较大的一半 */
        if (p - lo < hi - p) {
            introsortLoop(data, lo, p - 1, depth);
            lo = p + 1;
        } else {
            introsortLoop(data, p + 1, hi, depth);
            hi = p - 1;
        }
    }
}

void Introsort2::insertionSort(QVector<double>& data, int lo, int hi)
{
    for (int i = lo + 1; i <= hi; ++i) {
        double key = data[i];
        int j = i - 1;
        while (j >= lo && data[j] > key) {
            m_comparisons++;
            data[j + 1] = data[j];
            --j;
        }
        m_comparisons++;
        data[j + 1] = key;
    }
}

void Introsort2::heapSort(QVector<double>& data, int lo, int hi)
{
    int n = hi - lo + 1;
    for (int i = n / 2 - 1; i >= 0; --i)
        siftDown(data, lo, i, n);
    for (int i = n - 1; i > 0; --i) {
        std::swap(data[lo], data[lo + i]);
        siftDown(data, lo, 0, i);
    }
}

void Introsort2::siftDown(QVector<double>& data, int lo, int i, int n)
{
    double val = data[lo + i];
    while (true) {
        int child = 2 * i + 1;
        if (child >= n) break;
        if (child + 1 < n && data[lo + child] < data[lo + child + 1])
            child++;
        if (val >= data[lo + child]) break;
        data[lo + i] = data[lo + child];
        i = child;
    }
    data[lo + i] = val;
}

int Introsort2::medianOf3(QVector<double>& data, int a, int b, int c)
{
    m_comparisons += 3;
    if (data[a] > data[b]) std::swap(a, b);
    if (data[a] > data[c]) std::swap(a, c);
    if (data[b] > data[c]) std::swap(b, c);
    return b;
}

Introsort2::Stats Introsort2::stats() const { return m_stats; }

void Introsort2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
