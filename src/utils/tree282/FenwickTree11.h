/**
 * @file FenwickTree11.h
 * @brief 树状数组(差分BIT的范围更新与点查询批量高效区间修改) — Fenwick Tree with Range Update and Point Query Support via Difference Array BIT for Efficient Batch Range Modifications
 *
 * 功能: 实现树状数组(Fenwick tree)，采用差分BIT(difference array BIT)
 *       与范围更新与点查询(range update and point query)实现批量高效区间修改(efficient batch range modifications)。
 *
 * 协作: SegmentTree10(线段树) / SplayTree13(伸展树) / SparseTable9(稀疏表)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 树状数组(差分BIT的范围更新与点查询)
 */
class FenwickTree11 : public QObject {
    Q_OBJECT

public:
    /** @brief Batch operation result */
    struct BatchResult {
        int numOperations = 0;
        double processingTimeMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int treeSize = 0;
        int numRangeUpdates = 0;
        int numPointQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FenwickTree11(QObject *parent = nullptr);
    ~FenwickTree11() override;

    /** @brief Initialize tree with size n (1-indexed internally) */
    void init(int n);

    /** @brief Initialize from existing values (point query mode) */
    void initFromValues(const QVector<double>& values);

    /** @brief Range update: add delta to [l, r] (0-indexed, inclusive) */
    void rangeUpdate(int l, int r, double delta);

    /** @brief Point query: get value at index i (0-indexed) */
    double pointQuery(int i) const;

    /** @brief Range query: sum of [l, r] via two BITs (0-indexed, inclusive) */
    double rangeQuery(int l, int r) const;

    /** @brief Point update: add delta at index i (0-indexed) */
    void pointUpdate(int i, double delta);

    /** @brief Prefix query: sum of [0, i] */
    double prefixQuery(int i) const;

    /** @brief Batch range updates from (l, r, delta) triples */
    BatchResult batchRangeUpdate(const QVector<QVector<double>>& operations);

    /** @brief Get all point values */
    QVector<double> getAllValues() const;

    /** @brief Get tree size */
    int size() const { return m_n; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void rangeUpdateDone(int l, int r, double delta, double timeMs);
    void batchDone(int ops, double timeMs);

private:
    int m_n = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief BIT1 and BIT2 for range update / range query (difference array) */
    QVector<double> m_bit1;
    QVector<double> m_bit2;

    /** @brief Internal BIT update at index i with value delta */
    void internalUpdate(QVector<double>& bit, int i, double delta);

    /** @brief Internal BIT prefix sum up to index i */
    double internalQuery(const QVector<double>& bit, int i) const;

    /** @brief Combined prefix sum using two BITs */
    double combinedPrefixQuery(int i) const;
};
