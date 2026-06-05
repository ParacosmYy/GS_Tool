/**
 * @file IntervalHeap.h
 * @brief 区间堆(双端优先队列) — Interval Heap (Double-Ended Priority Queue)
 *
 * 功能: 支持O(log n)的min/max同时操作：insert/deleteMin/deleteMax/
 *       getMin/getMax。基于区间堆节点结构，每个节点存储一对(min,max)。
 *
 * 协作: BinaryHeap(单端优先队列) / MinMaxHeap(另一种双端实现)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 区间堆，同时支持高效的最小值和最大值操作
 */
class IntervalHeap : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalInserts = 0;           ///< 累计插入次数
        quint64 totalDeletes = 0;           ///< 累计删除次数
        quint64 totalMinQueries = 0;        ///< 累计最小值查询次数
        quint64 totalMaxQueries = 0;        ///< 累计最大值查询次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit IntervalHeap(QObject* parent = nullptr);

    /**
     * @brief 插入元素
     * @param value 待插入的值
     */
    void insert(double value);

    /**
     * @brief 删除并返回最小值
     * @return 最小值
     */
    double deleteMin();

    /**
     * @brief 删除并返回最大值
     * @return 最大值
     */
    double deleteMax();

    /**
     * @brief 获取最小值(不删除)
     * @return 最小值
     */
    double getMin() const;

    /**
     * @brief 获取最大值(不删除)
     * @return 最大值
     */
    double getMax() const;

    /** @brief 堆是否为空 */
    bool isEmpty() const { return m_size == 0; }

    /** @brief 堆中元素数量 */
    int size() const { return m_size; }

    /** @brief 清空堆 */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 插入完成 @param value 插入值 @param size 当前堆大小 */
    void elementInserted(double value, int size);
    /** @brief 删除完成 @param value 删除值 @param isMin 是否最小值 */
    void elementDeleted(double value, bool isMin);

private:
    /** @brief 区间堆节点，存储一对值 */
    struct IntervalNode {
        double left = 0.0;   ///< 较小值
        double right = 0.0;  ///< 较大值
        bool full = false;   ///< 是否已存两个值
    };

    /** @brief 向上修正堆性质 */
    void bubbleUp(int index);
    /** @brief 向下修正堆性质 */
    void trickleDown(int index);

    /** @brief 父节点索引 */
    static int parent(int i) { return (i - 1) / 2; }
    /** @brief 左子节点索引 */
    static int leftChild(int i) { return 2 * i + 1; }

    QVector<IntervalNode> m_heap;  ///< 堆存储
    int m_size = 0;                ///< 总元素数

    Stats m_stats;
    double m_timeSum = 0.0;
};
