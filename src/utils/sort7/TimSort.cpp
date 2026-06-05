/**
 * @file TimSort.cpp
 * @brief TimSort排序实现
 */

#include "TimSort.h"
#include <QElapsedTimer>
#include <algorithm>

TimSort::TimSort(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

void TimSort::sort(QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n < 2) return;

    const int MIN_RUN = 32;

    /* 对小段使用二分插入排序 */
    for (int i = 0; i < n; i += MIN_RUN) {
        int end = qMin(i + MIN_RUN, n);
        binaryInsertionSort(data, i, end);
    }

    /* 逐级归并 */
    QVector<double> temp(n);
    for (int size = MIN_RUN; size < n; size *= 2) {
        for (int left = 0; left < n; left += 2 * size) {
            int mid = qMin(left + size, n);
            int right = qMin(left + 2 * size, n);
            if (mid < right) {
                merge(data, left, mid, right, temp);
                m_stats.totalMerges++;
            }
        }
    }

    m_stats.totalSorted++;
    m_stats.totalElements += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSorted;

    emit sorted(n);
}

void TimSort::sortDescending(QVector<double>& data)
{
    sort(data);
    std::reverse(data.begin(), data.end());
}

QVector<double> TimSort::sorted(const QVector<double>& data) const
{
    QVector<double> copy = data;
    const_cast<TimSort*>(this)->sort(copy);
    return copy;
}

void TimSort::binaryInsertionSort(QVector<double>& arr, int left, int right)
{
    for (int i = left + 1; i < right; ++i) {
        double key = arr[i];
        int lo = left, hi = i;

        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (key < arr[mid]) hi = mid;
            else lo = mid + 1;
        }

        for (int j = i; j > lo; --j)
            arr[j] = arr[j - 1];
        arr[lo] = key;
    }
}

void TimSort::merge(QVector<double>& arr, int l, int m, int r,
                       QVector<double>& temp)
{
    int len1 = m - l, len2 = r - m;
    for (int i = 0; i < len1; ++i) temp[i] = arr[l + i];
    for (int i = 0; i < len2; ++i) temp[len1 + i] = arr[m + i];

    int i = 0, j = 0, k = l;
    while (i < len1 && j < len2) {
        if (temp[i] <= temp[len1 + j])
            arr[k++] = temp[i++];
        else
            arr[k++] = temp[len1 + j++];
    }
    while (i < len1) arr[k++] = temp[i++];
    while (j < len2) arr[k++] = temp[len1 + j++];
}

TimSort::Stats TimSort::stats() const { return m_stats; }

void TimSort::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
