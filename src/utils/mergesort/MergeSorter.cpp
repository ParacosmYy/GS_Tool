/**
 * @file MergeSorter.cpp
 * @brief 归并排序引擎实现
 */

#include "MergeSorter.h"

#include <QElapsedTimer>
#include <algorithm>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

MergeSorter::MergeSorter(QObject* parent) : QObject(parent) {}
MergeSorter::~MergeSorter() = default;

// ═══════════════════════════════════════════════════════════
// 排序操作
// ═══════════════════════════════════════════════════════════

QVector<double> MergeSorter::sort(const QVector<double>& data)
{
    if (data.size() <= 1) return data;

    QElapsedTimer timer;
    timer.start();

    QVector<double> result = data;
    QVector<double> tmp(result.size());
    mergeSort(result, tmp, 0, result.size() - 1);

    const double elapsed = timer.elapsed();
    m_stats.totalSorts++;
    m_stats.avgTimeMs = (m_stats.avgTimeMs * (m_stats.totalSorts - 1) + elapsed) /
                        static_cast<double>(m_stats.totalSorts);

    emit sorted(result.size(), elapsed);
    return result;
}

QVector<qint64> MergeSorter::sortInt64(const QVector<qint64>& data)
{
    if (data.size() <= 1) return data;

    QElapsedTimer timer;
    timer.start();

    QVector<qint64> result = data;
    QVector<qint64> tmp(result.size());
    mergeSortInt64(result, tmp, 0, result.size() - 1);

    const double elapsed = timer.elapsed();
    m_stats.totalSorts++;
    m_stats.avgTimeMs = (m_stats.avgTimeMs * (m_stats.totalSorts - 1) + elapsed) /
                        static_cast<double>(m_stats.totalSorts);

    emit sorted(result.size(), elapsed);
    return result;
}

void MergeSorter::sortInPlace(QVector<double>& data)
{
    if (data.size() <= 1) return;

    QElapsedTimer timer;
    timer.start();

    QVector<double> tmp(data.size());
    mergeSort(data, tmp, 0, data.size() - 1);

    const double elapsed = timer.elapsed();
    m_stats.totalSorts++;
    m_stats.avgTimeMs = (m_stats.avgTimeMs * (m_stats.totalSorts - 1) + elapsed) /
                        static_cast<double>(m_stats.totalSorts);

    emit sorted(data.size(), elapsed);
}

// ═══════════════════════════════════════════════════════════
// 逆序对计数
// ═══════════════════════════════════════════════════════════

quint64 MergeSorter::countInversions(const QVector<double>& data)
{
    if (data.size() <= 1) return 0;

    QVector<double> arr = data;
    QVector<double> tmp(arr.size());
    quint64 inv = countInversionsRec(arr, tmp, 0, arr.size() - 1);

    m_stats.totalInversions = inv;
    m_stats.totalSorts++;
    return inv;
}

// ═══════════════════════════════════════════════════════════
// 多路归并
// ═══════════════════════════════════════════════════════════

QVector<double> MergeSorter::mergeKSorted(const QVector<QVector<double>>& sortedArrays)
{
    if (sortedArrays.isEmpty()) return {};

    // 计算总大小
    qsizetype totalSize = 0;
    for (const auto& arr : sortedArrays) totalSize += arr.size();

    QVector<double> result;
    result.reserve(static_cast<int>(totalSize));

    // 使用索引数组进行K路归并
    QVector<int> indices(sortedArrays.size(), 0);

    while (static_cast<qsizetype>(result.size()) < totalSize) {
        double minVal = 0;
        int minArr = -1;
        bool first = true;

        for (int k = 0; k < sortedArrays.size(); ++k) {
            if (indices[k] < sortedArrays[k].size()) {
                double val = sortedArrays[k][indices[k]];
                if (first || val < minVal) {
                    minVal = val;
                    minArr = k;
                    first = false;
                }
                m_stats.totalComparisons++;
            }
        }

        if (minArr < 0) break;
        result.append(minVal);
        indices[minArr]++;
        m_stats.totalCopies++;
    }

    m_stats.totalSorts++;
    return result;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

MergeSorter::Stats MergeSorter::stats() const { return m_stats; }

void MergeSorter::resetStatistics() { m_stats = Stats{}; }

// ═══════════════════════════════════════════════════════════
// 内部辅助: double归并排序
// ═══════════════════════════════════════════════════════════

void MergeSorter::mergeSort(QVector<double>& arr, QVector<double>& tmp,
                            int left, int right)
{
    if (left >= right) return;
    const int mid = left + (right - left) / 2;
    mergeSort(arr, tmp, left, mid);
    mergeSort(arr, tmp, mid + 1, right);
    merge(arr, tmp, left, mid, right);
}

void MergeSorter::merge(QVector<double>& arr, QVector<double>& tmp,
                        int left, int mid, int right)
{
    // 拷贝到临时数组
    for (int i = left; i <= right; ++i) {
        tmp[i] = arr[i];
        m_stats.totalCopies++;
    }

    int i = left, j = mid + 1, k = left;
    while (i <= mid && j <= right) {
        m_stats.totalComparisons++;
        if (tmp[i] <= tmp[j]) {
            arr[k++] = tmp[i++];
        } else {
            arr[k++] = tmp[j++];
        }
        m_stats.totalCopies++;
    }
    while (i <= mid) { arr[k++] = tmp[i++]; m_stats.totalCopies++; }
    while (j <= right) { arr[k++] = tmp[j++]; m_stats.totalCopies++; }
}

// ═══════════════════════════════════════════════════════════
// 内部辅助: qint64归并排序
// ═══════════════════════════════════════════════════════════

void MergeSorter::mergeSortInt64(QVector<qint64>& arr, QVector<qint64>& tmp,
                                 int left, int right)
{
    if (left >= right) return;
    const int mid = left + (right - left) / 2;
    mergeSortInt64(arr, tmp, left, mid);
    mergeSortInt64(arr, tmp, mid + 1, right);
    mergeInt64(arr, tmp, left, mid, right);
}

void MergeSorter::mergeInt64(QVector<qint64>& arr, QVector<qint64>& tmp,
                             int left, int mid, int right)
{
    for (int i = left; i <= right; ++i) tmp[i] = arr[i];

    int i = left, j = mid + 1, k = left;
    while (i <= mid && j <= right) {
        m_stats.totalComparisons++;
        if (tmp[i] <= tmp[j]) arr[k++] = tmp[i++];
        else arr[k++] = tmp[j++];
    }
    while (i <= mid) arr[k++] = tmp[i++];
    while (j <= right) arr[k++] = tmp[j++];
}

// ═══════════════════════════════════════════════════════════
// 内部辅助: 逆序对计数
// ═══════════════════════════════════════════════════════════

quint64 MergeSorter::countInversionsRec(QVector<double>& arr, QVector<double>& tmp,
                                        int left, int right)
{
    if (left >= right) return 0;
    const int mid = left + (right - left) / 2;
    quint64 inv = countInversionsRec(arr, tmp, left, mid);
    inv += countInversionsRec(arr, tmp, mid + 1, right);

    // 归并并计数跨段逆序对
    for (int i = left; i <= right; ++i) tmp[i] = arr[i];

    int i = left, j = mid + 1, k = left;
    while (i <= mid && j <= right) {
        if (tmp[i] <= tmp[j]) {
            arr[k++] = tmp[i++];
        } else {
            inv += static_cast<quint64>(mid - i + 1);
            arr[k++] = tmp[j++];
        }
    }
    while (i <= mid) arr[k++] = tmp[i++];
    while (j <= right) arr[k++] = tmp[j++];

    return inv;
}
