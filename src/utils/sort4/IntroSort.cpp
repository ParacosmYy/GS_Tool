/**
 * @file IntroSort.cpp
 * @brief 内省排序实现 — 快排+堆排混合
 */

#include "utils/sort4/IntroSort.h"

#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
IntroSort::IntroSort(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 升序排序 @param array 待排序数组 */
void IntroSort::sort(QVector<double>& array)
{
    sortWithComparator(array, [](double a, double b) { return a < b; });
}

/** @brief 自定义比较器排序 @param array 待排序数组 @param comp 比较函数 */
void IntroSort::sortWithComparator(QVector<double>& array, const Comparator& comp)
{
    QElapsedTimer timer;
    timer.start();

    int n = array.size();
    if (n < 2) {
        double elapsed = static_cast<double>(timer.elapsed());
        ++m_stats.totalSorts;
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / static_cast<double>(m_stats.totalSorts);
        emit sortCompleted(n);
        return;
    }

    /* 深度限制 = 2*log2(n) */
    int depthLimit = 2 * static_cast<int>(std::log2(static_cast<double>(n))) + 1;
    introSortImpl(array, 0, n - 1, depthLimit, comp);

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalSorts;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / static_cast<double>(m_stats.totalSorts);

    emit sortCompleted(n);
}

/** @brief 重置统计 */
void IntroSort::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 内省排序递归核心 */
void IntroSort::introSortImpl(QVector<double>& arr, int lo, int hi,
                               int depthLimit, const Comparator& comp)
{
    while (lo < hi) {
        /* 小数组使用插入排序 */
        if (hi - lo < 16) {
            insertionSort(arr, lo, hi, comp);
            return;
        }

        /* 深度限制耗尽，切换到堆排序 */
        if (depthLimit <= 0) {
            heapSort(arr, lo, hi, comp);
            return;
        }

        --depthLimit;

        /* 快排划分 */
        int p = partition(arr, lo, hi, comp);

        /* 递归较小的半边，循环处理较大半边(尾递归优化) */
        if (p - lo < hi - p) {
            introSortImpl(arr, lo, p - 1, depthLimit, comp);
            lo = p + 1;
        } else {
            introSortImpl(arr, p + 1, hi, depthLimit, comp);
            hi = p - 1;
        }
    }
}

/** @brief 堆排序 */
void IntroSort::heapSort(QVector<double>& arr, int lo, int hi, const Comparator& comp)
{
    int n = hi - lo + 1;

    /* 建堆 */
    for (int i = n / 2 - 1; i >= 0; --i) {
        siftDown(arr, lo, hi, lo + i, comp);
    }

    /* 逐个提取最大元素 */
    for (int i = hi; i > lo; --i) {
        std::swap(arr[lo], arr[i]);
        siftDown(arr, lo, i - 1, lo, comp);
    }
}

/** @brief 堆化 */
void IntroSort::siftDown(QVector<double>& arr, int start, int end, int root,
                          const Comparator& comp)
{
    while (true) {
        int left = 2 * (root - start) + 1 + start;
        int right = left + 1;
        int largest = root;

        if (left <= end && comp(arr[largest], arr[left])) {
            largest = left;
        }
        if (right <= end && comp(arr[largest], arr[right])) {
            largest = right;
        }
        if (largest == root) break;

        std::swap(arr[root], arr[largest]);
        root = largest;
    }
}

/** @brief 插入排序 */
void IntroSort::insertionSort(QVector<double>& arr, int lo, int hi,
                               const Comparator& comp)
{
    for (int i = lo + 1; i <= hi; ++i) {
        double key = arr[i];
        int j = i - 1;
        while (j >= lo && comp(key, arr[j])) {
            arr[j + 1] = arr[j];
            --j;
        }
        arr[j + 1] = key;
    }
}

/** @brief 三数取中法划分 */
int IntroSort::partition(QVector<double>& arr, int lo, int hi, const Comparator& comp)
{
    /* 三数取中作为pivot */
    int mid = lo + (hi - lo) / 2;
    if (comp(arr[mid], arr[lo])) std::swap(arr[lo], arr[mid]);
    if (comp(arr[hi], arr[lo])) std::swap(arr[lo], arr[hi]);
    if (comp(arr[hi], arr[mid])) std::swap(arr[mid], arr[hi]);

    double pivot = arr[mid];
    std::swap(arr[mid], arr[hi - 1]);

    int i = lo;
    int j = hi - 1;

    while (true) {
        while (comp(arr[++i], pivot)) {}
        while (comp(pivot, arr[--j])) {}
        if (i >= j) break;
        std::swap(arr[i], arr[j]);
    }

    std::swap(arr[i], arr[hi - 1]);
    return i;
}
