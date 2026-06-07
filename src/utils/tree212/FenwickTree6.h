/**
 * @file FenwickTree6.h
 * @brief 二维树状数组(范围求和+坐标压缩稀疏网格) — Fenwick Tree with 2D Range Sum Query and Coordinate Compression for Sparse Grid Support
 *
 * 功能: 实现二维树状数组，支持范围求和查询、
 *       单点更新和坐标压缩稀疏网格优化。
 *
 * 协作: SegmentTree9(线段树) / SparseTable5(稀疏表) / KDTree4(KD树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 二维树状数组(范围求和+坐标压缩稀疏网格)
 */
class FenwickTree6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int gridSize = 0;
        int gridSizeX = 0;
        int gridSizeY = 0;
        int numUpdates = 0;
        int numQueries = 0;
        int compressedX = 0;
        int compressedY = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FenwickTree6(QObject *parent = nullptr);
    ~FenwickTree6() override;

    /** @brief Initialize with fixed grid size */
    void init(int sizeX, int sizeY);

    /** @brief Initialize with coordinate compression */
    void initCompressed(const QVector<QPair<int, int>>& points);

    /** @brief Point update: add value at (x, y) */
    void update(int x, int y, double value);

    /** @brief 1D point update on compressed axis */
    void update1D(int idx, double value);

    /** @brief Prefix sum query: sum of [0..x] x [0..y] */
    double query(int x, int y) const;

    /** @brief Range sum query: sum of [x1..x2] x [y1..y2] */
    double rangeQuery(int x1, int y1, int x2, int y2) const;

    /** @brief 1D prefix sum query */
    double query1D(int idx) const;

    /** @brief 1D range sum query [l..r] */
    double rangeQuery1D(int l, int r) const;

    /** @brief Get value at (x, y) via difference */
    double pointQuery(int x, int y) const;

    /** @brief Clear all values */
    void clear();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, double timeMs);

private:
    int m_sizeX = 0;
    int m_sizeY = 0;

    // 2D BIT array (1-indexed internally)
    QVector<QVector<double>> m_tree;

    // 1D mode
    QVector<double> m_tree1D;
    int m_size1D = 0;

    // Coordinate compression maps
    QVector<int> m_compressX;  // Sorted unique X coordinates
    QVector<int> m_compressY;  // Sorted unique Y coordinates

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief LSB operation */
    static int lsb(int x) { return x & (-x); }

    /** @brief Compress coordinate to 1-based index */
    int compressX(int x) const;
    int compressY(int y) const;
};
