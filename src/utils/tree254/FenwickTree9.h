/**
 * @file FenwickTree9.h
 * @brief 树状数组(批量更新+range-max查询滑动窗口最大值变体) — Fenwick Tree with Batch Update Support and Range-Max Query Variant for Sliding Window Maximum Computation
 *
 * 功能: 实现树状数组(Fenwick Tree / Binary Indexed Tree)，支持批量更新
 *       (batch update)一次性提交多个修改，range-max查询变体(range-max
 *       query variant)用于滑动窗口最大值(sliding window maximum)高效计算。
 *
 * 协作: SegmentTree8(线段树) / SplayTree11(伸展树) / RedBlackTree14(红黑树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 树状数组(批量更新+range-max滑动窗口)
 */
class FenwickTree9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int treeSize = 0;
        int numUpdates = 0;
        int numBatchUpdates = 0;
        int numQueries = 0;
        int numRangeMaxQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FenwickTree9(QObject *parent = nullptr);
    ~FenwickTree9() override;

    /** @brief Initialize tree with size n, all values set to 0 */
    void init(int n);

    /** @brief Initialize from existing array */
    void initFromArray(const QVector<double>& values);

    /** @brief Point update: add delta at index i (0-based) */
    void update(int i, double delta);

    /** @brief Batch update: apply multiple deltas at once */
    void batchUpdate(const QVector<QPair<int, double>>& updates);

    /** @brief Point query: prefix sum [0..i] */
    double query(int i) const;

    /** @brief Range sum query [l..r] */
    double rangeQuery(int l, int r) const;

    /** @brief Point set: set index i to value (for max variant) */
    void set(int i, double value);

    /** @brief Batch set: set multiple values at once */
    void batchSet(const QVector<QPair<int, double>>& sets);

    /** @brief Range max query [l..r] using max-Fenwick variant */
    double rangeMax(int l, int r) const;

    /** @brief Sliding window maximum over the array */
    QVector<double> slidingWindowMax(int windowSize) const;

    /** @brief Get value at index i */
    double get(int i) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void updateCompleted(int numUpdates, double timeMs);

private:
    int m_n = 0;
    QVector<double> m_tree;     // Sum BIT
    QVector<double> m_maxTree;  // Max BIT variant
    QVector<double> m_values;   // Original values for max rebuild

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief LSB (least significant bit) */
    static int lsb(int x) { return x & (-x); }

    /** @brief Rebuild max tree from scratch (after batch set) */
    void rebuildMaxTree();
};
