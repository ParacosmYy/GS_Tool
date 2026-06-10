/**
 * @file FenwickTree10.h
 * @brief 二维树状数组(嵌套BIT点更新与二维区间和查询矩阵前缀和) — Fenwick Tree with 2D Range Sum Queries and Point Updates via Nested BIT for Matrix Prefix Sum Operations
 *
 * 功能: 实现二维树状数组(Fenwick tree / BIT)，采用嵌套BIT(nested BIT)实现
 *       点更新(point update)与二维区间和查询(2D range sum query)的矩阵前缀和操作。
 *
 * 协作: SegmentTree13(线段树) / SplayTree12(伸展树) / RedBlackTree15(红黑树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 二维树状数组(嵌套BIT点更新与二维区间和查询矩阵前缀和)
 */
class FenwickTree10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numRows = 0;
        int numCols = 0;
        int numUpdates = 0;
        int numQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FenwickTree10(QObject *parent = nullptr);
    ~FenwickTree10() override;

    /** @brief Initialize 2D Fenwick tree with rows x cols */
    void init(int rows, int cols);

    /** @brief Build from existing matrix */
    void build(const QVector<QVector<double>>& matrix);

    /** @brief Point update: add delta at (row, col) */
    void update(int row, int col, double delta);

    /** @brief Prefix sum query: sum from (1,1) to (row, col) */
    double prefixSum(int row, int col) const;

    /** @brief Range sum query: sum within (r1,c1) to (r2,c2) inclusive */
    double rangeSum(int r1, int c1, int r2, int c2) const;

    /** @brief Get value at (row, col) via query */
    double get(int row, int col) const;

    /** @brief Get tree dimensions */
    int rows() const;
    int cols() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void treeUpdated(int numUpdates, int numQueries, double timeMs);

private:
    int m_rows = 0;
    int m_cols = 0;

    // 2D BIT: m_tree[i][j] stores partial sums
    QVector<QVector<double>> m_tree;
    // Original values for delta computation
    QVector<QVector<double>> m_values;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Lowest set bit (LSB) index */
    static int lsb(int x) { return x & (-x); }
};
