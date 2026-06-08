/**
 * @file FenwickTree7.h
 * @brief 树状数组(3D范围查询+Z序曲线索引空间体积查询) — Fenwick Tree with 3D Range Query Support and Z-Order Curve Indexing for Spatial Volumetric Queries
 *
 * 功能: 实现树状数组(Binary Indexed Tree)的3D扩展，支持空间体积范围查询，
 *       集成Z序曲线(Morton码)索引实现高效空间降维查询。
 *
 * 协作: SegmentTree6(线段树) / SplayTree9(伸展树) / KDTree7(KD树)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 树状数组(3D范围查询+Z序曲线索引)
 */
class FenwickTree7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int dimX = 0;
        int dimY = 0;
        int dimZ = 0;
        int numUpdates = 0;
        int numQueries = 0;
        int numZQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FenwickTree7(QObject *parent = nullptr);
    ~FenwickTree7() override;

    /** @brief Initialize 3D Fenwick tree with dimensions */
    bool init(int dimX, int dimY, int dimZ);

    /** @brief Point update: add value at (x, y, z) */
    void update(int x, int y, int z, double value);

    /** @brief 3D prefix sum: sum of all points in [1..x][1..y][1..z] */
    double prefixSum(int x, int y, int z) const;

    /** @brief 3D range sum query: sum in [x1..x2][y1..y2][z1..z2] */
    double rangeSum(int x1, int y1, int z1, int x2, int y2, int z2) const;

    /** @brief Z-order curve (Morton code) from 3D coordinates */
    quint64 mortonEncode(int x, int y, int z) const;

    /** @brief Decode Morton code back to 3D coordinates */
    void mortonDecode(quint64 code, int& x, int& y, int& z) const;

    /** @brief Z-order range query: sum within Morton curve range */
    double zOrderRangeSum(quint64 mortonStart, quint64 mortonEnd) const;

    /** @brief Bulk load from 3D data array */
    void bulkLoad(const QVector<QVector<QVector<double>>>& data);

    /** @brief Get dimensions */
    int sizeX() const { return m_dimX; }
    int sizeY() const { return m_dimY; }
    int sizeZ() const { return m_dimZ; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, double timeMs);

private:
    int m_dimX = 0;
    int m_dimY = 0;
    int m_dimZ = 0;

    // 3D BIT storage (1-indexed)
    QVector<QVector<QVector<double>>> m_tree;

    // Z-order sorted values for Morton queries
    QVector<QPair<quint64, double>> m_zOrderData;
    // Fenwick tree over Z-order sorted data
    QVector<double> m_zFenwick;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Least significant bit */
    int lsb(int idx) const { return idx & (-idx); }

    /** @brief Spread bits for Morton encoding */
    quint64 spreadBits(quint32 v) const;

    /** @brief Compact bits for Morton decoding */
    quint32 compactBits(quint64 v) const;

    /** @brief Build Z-order Fenwick from sorted data */
    void buildZFenwick();

    /** @brief Z-order prefix sum */
    double zPrefixSum(int idx) const;

    /** @brief Z-order update */
    void zUpdate(int idx, double delta);
};
