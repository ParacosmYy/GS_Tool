/**
 * @file FenwickTree8.h
 * @brief 二维树状数组(2D范围求和+点更新+二维累积频率计算) — Fenwick Tree with 2D Range Sum Query and Point Update for Bi-Dimensional Cumulative Frequency Computation
 *
 * 功能: 实现二维树状数组(2D Fenwick tree / Binary Indexed Tree)，支持2D范围求和查询
 *       (2D range sum query)在O(log^2 n)时间内计算矩形区域元素和，利用点更新(point update)
 *       在O(log^2 n)时间修改单个元素，用于二维累积频率计算(bi-dimensional cumulative
 *       frequency computation)。
 *
 * 协作: SplayTree10(伸展树) / RedBlackTree13(红黑树) / SegmentTree7(线段树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 二维树状数组(2D范围求和+点更新+二维累积频率计算)
 */
class FenwickTree8 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int rows = 0;
        int cols = 0;
        int numUpdates = 0;
        int numQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FenwickTree8(QObject *parent = nullptr);
    ~FenwickTree8() override;

    /** @brief Initialize tree with dimensions (rows x cols) */
    void resize(int rows, int cols);

    /** @brief Build tree from 2D data array */
    void build(const QVector<QVector<double>>& data);

    /** @brief Point update: add delta at (row, col) */
    void update(int row, int col, double delta);

    /** @brief Set value at (row, col) */
    void set(int row, int col, double value);

    /** @brief Prefix sum: sum of [0..row][0..col] */
    double prefixSum(int row, int col) const;

    /** @brief Range sum: sum of rectangle (r1,c1) to (r2,c2) inclusive */
    double rangeSum(int r1, int c1, int r2, int c2) const;

    /** @brief Get value at (row, col) */
    double get(int row, int col) const;

    /** @brief Get number of rows */
    int rows() const;

    /** @brief Get number of columns */
    int cols() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void updated(int row, int col, double delta);
    void queryCompleted(double result, double timeMs);

private:
    int m_rows = 0;
    int m_cols = 0;

    // BIT array (1-indexed internally)
    QVector<QVector<double>> m_tree;

    // Original values for set() operations
    QVector<QVector<double>> m_values;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Internal prefix sum on single dimension */
    double prefixSum1D(const QVector<double>& arr, int idx) const;

    /** @brief Internal point update on single dimension */
    void update1D(QVector<double>& arr, int idx, double delta);

    /** @brief Lowest set bit position (LSB) */
    static int lsb(int i) { return i & (-i); }
};
