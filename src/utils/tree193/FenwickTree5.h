/**
 * @file FenwickTree5.h
 * @brief 树状数组3D扩展(批量点更新+矩形范围查询) — Fenwick Tree with 3D Extension, Batch Point Updates and Rectangular Range Query
 *
 * 功能: 实现三维树状数组(BIT)，支持批量点更新、
 *       矩形/长方体范围查询和O(log^3 N)操作。
 *
 * 协作: SegmentTree6(线段树) / VanEmdeBoas4(vEB树) / SplayTree7(伸展树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 三维树状数组(批量更新+矩形范围查询)
 */
class FenwickTree5 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int dimX = 0;
        int dimY = 0;
        int dimZ = 0;
        int pointUpdates = 0;
        int rangeQueries = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FenwickTree5(int nx = 0, int ny = 0, int nz = 0,
                           QObject *parent = nullptr);
    ~FenwickTree5() override;

    /** @brief Initialize with dimensions */
    void resize(int nx, int ny, int nz);

    /** @brief Single point update: add delta at (x,y,z) */
    void pointUpdate(int x, int y, int z, double delta);

    /** @brief Batch point updates from list */
    void batchUpdate(const QVector<QPair<QVector<int>, double>>& updates);

    /** @brief Prefix sum from (1,1,1) to (x,y,z) inclusive */
    double prefixSum(int x, int y, int z) const;

    /** @brief Range query sum over rectangular box [x1..x2, y1..y2, z1..z2] */
    double rangeQuery(int x1, int y1, int z1,
                      int x2, int y2, int z2) const;

    /** @brief Point query: value at (x,y,z) */
    double pointQuery(int x, int y, int z) const;

    /** @brief Reset all values to zero */
    void clear();

    int sizeX() const { return m_nx; }
    int sizeY() const { return m_ny; }
    int sizeZ() const { return m_nz; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int count, double timeMs);

private:
    int m_nx = 0;
    int m_ny = 0;
    int m_nz = 0;

    // 3D BIT stored as flat array: tree[x][y][z]
    QVector<QVector<QVector<double>>> m_tree;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Internal 3D update at BIT indices */
    void updateInternal(int x, int y, int z, double delta);

    /** @brief Internal 3D prefix sum at BIT indices */
    double queryInternal(int x, int y, int z) const;

    /** @brief Validate coordinate bounds */
    bool validCoord(int x, int y, int z) const;
};
