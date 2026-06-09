/**
 * @file SegmentTree9.h
 * @brief 线段树(Beats操作+懒惰传播区间赋值最大最小和查询) — Segment Tree with Beats Operation and Lazy Propagation for Range Max-min Sum Queries with Interval Assignment
 *
 * 功能: 实现线段树(segment tree)的Beats操作，采用懒惰传播(lazy
 *       propagation)支持区间赋值(interval assignment)，以及区间
 *       最大值、最小值、求和(range max-min sum)查询。
 *
 * 协作: FenwickTree7(树状数组) / BTree8(B树) / AVLTree8(AVL树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 线段树(Beats操作+懒惰传播区间赋值最大最小和查询)
 */
class SegmentTree9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int treeSize = 0;
        int numUpdates = 0;
        int numQueries = 0;
        int numBeatsPushes = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Query result with max, min, and sum */
    struct RangeResult {
        double maximum = 0.0;
        double minimum = 0.0;
        double sum = 0.0;
    };

    explicit SegmentTree9(QObject *parent = nullptr);
    ~SegmentTree9() override;

    /** @brief Build tree from initial values */
    void build(const QVector<double>& values);

    /** @brief Range assignment: set [lo, hi] to value */
    void rangeAssign(int lo, int hi, double value);

    /** @brief Beats operation: chmin for [lo, hi] — a[i] = min(a[i], value) */
    void rangeChmin(int lo, int hi, double value);

    /** @brief Beats operation: chmax for [lo, hi] — a[i] = max(a[i], value) */
    void rangeChmax(int lo, int hi, double value);

    /** @brief Range query: get max, min, sum for [lo, hi] */
    RangeResult rangeQuery(int lo, int hi) const;

    /** @brief Point query: get value at index */
    double pointQuery(int index) const;

    /** @brief Get array size */
    int size() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void updateCompleted(int lo, int hi, double timeMs);
    void queryCompleted(int lo, int hi, double maxVal, double minVal, double sum, double timeMs);

private:
    /** @brief Segment tree node for Beats */
    struct Node {
        double maxValue = 0.0;
        double secondMax = 0.0;
        double minValue = 0.0;
        double secondMin = 0.0;
        double sum = 0.0;
        int maxCount = 0;
        int minCount = 0;
        double lazyAssign = qQNaN();   // NaN means no pending assign
        bool hasAssign = false;
    };

    int m_n = 0;
    QVector<Node> m_tree;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build tree recursively */
    void buildHelper(int idx, int lo, int hi, const QVector<double>& values);

    /** @brief Push lazy assignment to children */
    void pushDown(int idx, int lo, int hi);

    /** @brief Apply beats chmin to node */
    void applyChmin(int idx, double value);

    /** @brief Apply beats chmax to node */
    void applyChmax(int idx, double value);

    /** @brief Range assign helper */
    void assignHelper(int idx, int lo, int hi, int qLo, int qHi, double value);

    /** @brief Range chmin helper */
    void chminHelper(int idx, int lo, int hi, int qLo, int qHi, double value);

    /** @brief Range chmax helper */
    void chmaxHelper(int idx, int lo, int hi, int qLo, int qHi, double value);

    /** @brief Range query helper */
    RangeResult queryHelper(int idx, int lo, int hi, int qLo, int qHi) const;

    /** @brief Pull up from children */
    void pullUp(int idx);
};
