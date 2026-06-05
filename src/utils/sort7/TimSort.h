/**
 * @file TimSort.h
 * @brief TimSort排序(Tim Sort)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @class TimSort
 * @brief TimSort — 自适应混合排序(归并+插入)
 *
 * 使用自然run检测、二分插入排序小段、归并排序大段。
 * 时间复杂度: 最优O(n), 平均O(nlogn), 最坏O(nlogn)。
 * 适用于部分有序数据的排序。
 */
class TimSort : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalSorted = 0;       /**< 总排序次数 */
        int totalElements = 0;     /**< 总元素数 */
        int totalRuns = 0;         /**< 总run数 */
        int totalMerges = 0;       /**< 总归并次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit TimSort(QObject* parent = nullptr);

    /**
     * @brief 升序排序
     * @param data 待排序数据(会被修改)
     */
    void sort(QVector<double>& data);

    /**
     * @brief 降序排序
     * @param data 待排序数据(会被修改)
     */
    void sortDescending(QVector<double>& data);

    /**
     * @brief 排序并返回副本
     * @param data 输入数据
     * @return 排序后的副本
     */
    QVector<double> sorted(const QVector<double>& data) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 排序完成信号 */
    void sorted(int count);

private:
    void binaryInsertionSort(QVector<double>& arr, int left, int right);
    void merge(QVector<double>& arr, int l, int m, int r,
               QVector<double>& temp);

    Stats m_stats;
    double m_timeSum;
};
