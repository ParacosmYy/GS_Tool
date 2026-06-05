/**
 * @file Introsort2.h
 * @brief Introsort自适应排序
 */

#pragma once

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @brief Introsort自适应排序
 *
 * 结合快速排序、堆排序和插入排序的混合算法,
 * 保证最坏情况O(n log n)。
 */
class Introsort2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalSorts = 0;             ///< 总排序次数
        int totalElementsSorted = 0;    ///< 总排序元素数
        int totalComparisons = 0;       ///< 总比较次数
        double avgProcessingTimeMs = 0.0;
    };

    explicit Introsort2(QObject* parent = nullptr);

    /**
     * @brief 排序(升序)
     * @param data 输入数据
     * @return 排序后数据
     */
    QVector<double> sort(const QVector<double>& data);

    /**
     * @brief 原地排序
     */
    void sortInPlace(QVector<double>& data);

    /**
     * @brief 自定义比较器排序
     */
    void sortCustom(QVector<double>& data,
                    std::function<bool(double, double)> cmp);

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 排序完成信号 */
    void sorted(int count, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_comparisons;

    void introsortLoop(QVector<double>& data, int lo, int hi, int depth);
    void insertionSort(QVector<double>& data, int lo, int hi);
    void heapSort(QVector<double>& data, int lo, int hi);
    void siftDown(QVector<double>& data, int lo, int i, int hi);
    int medianOf3(QVector<double>& data, int a, int b, int c);
};
