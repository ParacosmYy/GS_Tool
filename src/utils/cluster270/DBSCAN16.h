/**
 * @file DBSCAN16.h
 * @brief DBSCAN密度聚类(空间索引网格加速与核心/边界点分类) — DBSCAN with Spatial Index Grid Acceleration and Border-Core Point Classification for Density-Based Clustering
 *
 * 功能: 实现DBSCAN密度聚类(DBSCAN density-based clustering)，采用空间索引网格加速(spatial index grid acceleration)
 *       与核心/边界点分类(border-core point classification)实现高效密度聚类(efficient density clustering)。
 *
 * 协作: KMeans28(K均值) / OPTICS12(排序聚类) / GaussianMixture30(高斯混合)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief DBSCAN密度聚类(空间索引网格加速与核心/边界点分类)
 */
class DBSCAN16 : public QObject {
    Q_OBJECT

public:
    /** @brief Point classification */
    enum PointType { Core = 0, Border = 1, Noise = 2 };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numClusters = 0;
        int numCore = 0;
        int numBorder = 0;
        int numNoise = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DBSCAN16(QObject *parent = nullptr);
    ~DBSCAN16() override;

    /** @brief Set neighborhood radius epsilon */
    void setEpsilon(double eps);

    /** @brief Set minimum points to form a core point */
    void setMinPoints(int minPts);

    /** @brief Fit model to 2D data, returns cluster labels (-1 = noise) */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Get point type classification */
    QVector<int> pointTypes() const;

    /** @brief Get cluster count */
    int numClusters() const;

    /** @brief Get cluster labels from last fit */
    QVector<int> labels() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringDone(int numClusters, int numCore, int numBorder, int numNoise, double timeMs);

private:
    double m_epsilon = 1.0;
    int m_minPts = 5;

    QVector<int> m_labels;
    QVector<int> m_pointTypes;
    int m_numClusters = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Spatial grid cell for O(1) neighbor lookup */
    struct GridCell {
        QVector<int> pointIndices;
    };

    /** @brief Compute grid cell key for a 2D point */
    qint64 gridKey(double x, double y, double cellSize) const;

    /** @brief Build spatial index grid from data */
    void buildGrid(const QVector<QVector<double>>& data, double cellSize);

    /** @brief Find neighbors within epsilon using grid index */
    QVector<int> regionQuery(const QVector<QVector<double>>& data, int idx, double cellSize) const;

    /** @brief Expand cluster from a core point via BFS */
    void expandCluster(const QVector<QVector<double>>& data, int idx,
                       int clusterId, double cellSize,
                       const QVector<QVector<int>>& neighborCache);

    /** @brief Hash map for grid cells (open addressing) */
    QVector<QPair<qint64, int>> m_gridMap;
    QVector<GridCell> m_gridCells;
    int m_gridCapacity = 0;

    int gridFind(qint64 key) const;
    int gridInsert(qint64 key);
};
