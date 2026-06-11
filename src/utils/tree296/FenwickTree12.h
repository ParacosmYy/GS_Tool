/**
 * @file FenwickTree12.h
 * @brief 树状数组(二维范围查询与坐标压缩实现多维前缀和操作) — Fenwick Tree with 2D Range Query Support and Coordinate Compression for Multi-dimensional Prefix Sum Operations
 *
 * 功能: 实现树状数组(Fenwick tree)，采用二维范围查询(2D range query)
 *       与坐标压缩(coordinate compression)实现多维前缀和操作(multi-dimensional prefix sum operations)。
 *
 * 协作: SegmentTree13(线段树) / SplayTree14(伸展树) / PersistentSegmentTree11(可持久化线段树)
 */
#pragma once

#include <QObject>
#include <QVector>

class FenwickTree12 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int treeRows = 0;
        int treeCols = 0;
        int numPoints = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FenwickTree12(QObject *parent = nullptr);
    ~FenwickTree12() override;

    /** @brief Initialize 2D Fenwick tree with dimensions */
    void init(int rows, int cols);

    /** @brief Add value at compressed coordinate (r, c) */
    void add(int r, int c, double value);

    /** @brief Prefix sum from (1,1) to (r, c) */
    double prefixSum(int r, int c) const;

    /** @brief Range sum query from (r1,c1) to (r2,c2) */
    double rangeSum(int r1, int c1, int r2, int c2) const;

    /** @brief Compress coordinates for sparse data */
    void compressCoordinates(QVector<int>& xs, QVector<int>& ys);

    /** @brief Point query at (r, c) */
    double pointQuery(int r, int c) const;

    /** @brief Point update: set value at (r, c) */
    void pointUpdate(int r, int c, double value);

    int rows() const { return m_rows; }
    int cols() const { return m_cols; }
    int size() const { return m_numPoints; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationDone(const QString& op, int r, int c, double timeMs);

private:
    int m_rows = 0;
    int m_cols = 0;
    int m_numPoints = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief 2D Fenwick tree data */
    QVector<QVector<double>> m_tree;

    /** @brief Original values for point update */
    QVector<QVector<double>> m_original;

    /** @brief Compressed x-coordinates */
    QVector<int> m_compressedX;

    /** @brief Compressed y-coordinates */
    QVector<int> m_compressedY;

    /** @brief Lowest set bit */
    static int lsb(int i) { return i & (-i); }
};
