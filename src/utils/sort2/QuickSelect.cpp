/**
 * @file QuickSelect.cpp
 * @brief 快速选择算法实现
 */

#include "utils/sort2/QuickSelect.h"

#include <QElapsedTimer>
#include <algorithm>
#include <random>

QuickSelect::QuickSelect(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

double QuickSelect::select(QVector<double>& data, int k)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) return 0.0;
    k = qBound(0, k, data.size() - 1);
    double result = selectImpl(data, 0, data.size() - 1, k);

    m_stats.totalSelects++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalSelects, 1ULL);

    emit elementSelected(k, result);
    return result;
}

double QuickSelect::selectKthLargest(QVector<double>& data, int k)
{
    if (data.isEmpty()) return 0.0;
    return select(data, data.size() - 1 - k);
}

double QuickSelect::median(QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    double result = 0.0;
    if (data.isEmpty()) return result;

    int n = data.size();
    if (n % 2 == 1) {
        result = selectImpl(data, 0, n - 1, n / 2);
    } else {
        double a = selectImpl(data, 0, n - 1, n / 2 - 1);
        double b = selectImpl(data, 0, n - 1, n / 2);
        result = (a + b) / 2.0;
    }

    m_stats.totalSelects++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalSelects, 1ULL);

    emit medianComputed(result);
    return result;
}

double QuickSelect::percentile(QVector<double>& data, double pct)
{
    if (data.isEmpty()) return 0.0;
    pct = qBound(0.0, pct, 100.0);
    int k = static_cast<int>(pct / 100.0 * (data.size() - 1));
    return select(data, k);
}

QVector<double> QuickSelect::multiSelect(QVector<double>& data,
                                          const QVector<int>& ks)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> results;
    results.reserve(ks.size());
    for (int k : ks) {
        if (k >= 0 && k < data.size()) {
            results.append(selectImpl(data, 0, data.size() - 1, k));
        }
    }

    m_stats.totalSelects++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalSelects, 1ULL);

    return results;
}

double QuickSelect::selectImpl(QVector<double>& data, int left, int right, int k)
{
    while (left < right) {
        /* 如果范围较大，使用中位数的中位数避免最坏情况 */
        int pivotIdx;
        if (right - left > 10) {
            double pivot = medianOfMedians(data, left, right);
            pivotIdx = partition(data, left, right, pivot);
            m_stats.medianOfMediansUsed++;
        } else {
            /* 小范围随机选择 */
            int mid = left + (right - left) / 2;
            std::swap(data[mid], data[right]);
            pivotIdx = partition(data, left, right, data[right]);
        }

        if (k == pivotIdx) {
            return data[k];
        } else if (k < pivotIdx) {
            right = pivotIdx - 1;
        } else {
            left = pivotIdx + 1;
        }
    }
    return data[left];
}

double QuickSelect::medianOfMedians(QVector<double>& data, int left, int right)
{
    int n = right - left + 1;
    if (n <= 5) {
        /* 5个以下直接排序取中位数 */
        QVector<double> sub(data.begin() + left, data.begin() + right + 1);
        std::sort(sub.begin(), sub.end());
        return sub[sub.size() / 2];
    }

    /* 每5个取中位数 */
    QVector<double> medians;
    for (int i = left; i <= right; i += 5) {
        int end = qMin(i + 4, right);
        QVector<double> sub(data.begin() + i, data.begin() + end + 1);
        std::sort(sub.begin(), sub.end());
        medians.append(sub[sub.size() / 2]);
        m_stats.totalComparisons += end - i;
    }

    /* 递归求中位数的中位数 */
    return selectImpl(medians, 0, medians.size() - 1, medians.size() / 2);
}

int QuickSelect::partition(QVector<double>& data, int left, int right,
                            double pivot)
{
    /* 找到pivot位置并放到最右 */
    int pivotIdx = left;
    for (int i = left; i < right; ++i) {
        m_stats.totalComparisons++;
        if (data[i] == pivot) {
            pivotIdx = i;
            break;
        }
    }
    std::swap(data[pivotIdx], data[right]);

    /* 标准partition */
    int store = left;
    for (int i = left; i < right; ++i) {
        m_stats.totalComparisons++;
        if (data[i] < pivot) {
            std::swap(data[i], data[store]);
            ++store;
        }
    }
    std::swap(data[store], data[right]);
    return store;
}

void QuickSelect::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
