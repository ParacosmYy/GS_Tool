/**
 * @file ShellSort.h
 * @brief Shell排序算法实现
 */

#pragma once

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @class ShellSort
 * @brief Shell排序 — 基于插入排序的改进排序算法
 *
 * 使用递减增量序列对子序列进行插入排序。
 * 支持自定义比较函数和多种增量序列(Ciura/Sedgewick/Knuth)。
 */
class ShellSort : public QObject
{
    Q_OBJECT

public:
    /** @brief 增量序列类型 */
    enum GapSequence {
        Ciura,      /**< Ciura序列(实际最优) */
        Sedgewick,  /**< Sedgewick序列 */
        Knuth,      /**< Knuth序列(3x+1) */
        Hibbard     /**< Hibbard序列(2^k-1) */
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalSorted = 0;        /**< 总排序次数 */
        int totalElements = 0;      /**< 总排序元素数 */
        int totalComparisons = 0;   /**< 总比较次数 */
        int totalSwaps = 0;         /**< 总交换次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit ShellSort(QObject* parent = nullptr);

    /**
     * @brief 排序
     * @param data 待排序数据
     * @param seq 增量序列
     */
    void sort(QVector<double>& data, GapSequence seq = Ciura);

    /**
     * @brief 排序(自定义比较)
     * @param data 待排序数据
     * @param cmp 比较函数
     * @param seq 增量序列
     */
    void sortWithComparator(QVector<double>& data,
                             std::function<bool(double, double)> cmp,
                             GapSequence seq = Ciura);

    /**
     * @brief 排序并返回索引(不修改原数组)
     * @param data 数据
     * @param seq 增量序列
     * @return 排序后的索引
     */
    QVector<int> sortIndices(const QVector<double>& data,
                              GapSequence seq = Ciura) const;

    /** @brief 检查数组是否已排序 */
    bool isSorted(const QVector<double>& data) const;

    /** @brief 生成增量序列 */
    QVector<int> generateGapSequence(int n, GapSequence seq) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 排序完成信号 */
    void sortCompleted(int count, double timeMs);

private:
    Stats m_stats;
    double m_timeSum;
};
