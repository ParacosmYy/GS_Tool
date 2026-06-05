/**
 * @file IntroSort.h
 * @brief 内省排序 — 快排+堆排混合排序算法
 *
 * 功能: 结合快速排序、堆排序和插入排序的优点，
 *       保证最坏情况下O(NlogN)时间复杂度。
 *
 * 协作: MergeSorter(稳定排序) / RadixSorter(整数排序)
 */
#ifndef INTROSORT_H
#define INTROSORT_H

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @brief 内省排序器
 */
class IntroSort : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalSorts = 0;          ///< 累计排序次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief 比较器类型 */
    using Comparator = std::function<bool(double, double)>;

    explicit IntroSort(QObject* parent = nullptr);

    /** @brief 升序排序
     *  @param array 待排序数组(原地排序) */
    void sort(QVector<double>& array);

    /** @brief 自定义比较器排序
     *  @param array 待排序数组
     *  @param comp 比较函数(comp(a,b)=true 则a在前) */
    void sortWithComparator(QVector<double>& array, const Comparator& comp);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 排序完成 @param count 排序元素数量 */
    void sortCompleted(int count);

private:
    /** @brief 内省排序递归核心
     *  @param arr 数组 @param lo 下界 @param hi 上界 @param depthLimit 深度限制 @param comp 比较器 */
    void introSortImpl(QVector<double>& arr, int lo, int hi,
                       int depthLimit, const Comparator& comp);

    /** @brief 堆排序 @param arr 数组 @param lo 下界 @param hi 上界 @param comp 比较器 */
    void heapSort(QVector<double>& arr, int lo, int hi,
                  const Comparator& comp);

    /** @brief 堆化 @param arr 数组 @param start 起点 @param end 终点 @param root 根 @param comp 比较器 */
    void siftDown(QVector<double>& arr, int start, int end, int root,
                  const Comparator& comp);

    /** @brief 插入排序(小数组) @param arr 数组 @param lo 下界 @param hi 上界 @param comp 比较器 */
    void insertionSort(QVector<double>& arr, int lo, int hi,
                       const Comparator& comp);

    /** @brief 三数取中法划分 @param arr 数组 @param lo 下界 @param hi 上界 @param comp 比较器 @return 划分点 */
    int partition(QVector<double>& arr, int lo, int hi,
                  const Comparator& comp);

    double m_timeSum;   ///< 处理时间累加器
    Stats  m_stats;     ///< 统计信息
};

#endif // INTROSORT_H
