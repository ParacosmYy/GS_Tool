/**
 * @file DBSCAN11.h
 * @brief DBSCAN聚类(混合网格/HNSW索引+并行簇扩展) — DBSCAN with Hybrid Grid/HNSW Index for O(n log n) Neighbor Search and Parallel Cluster Expansion
 *
 * 功能: 实现DBSCAN密度聚类算法，支持混合网格/HNSW近邻索引、
 *       O(n log n)邻域查询和并行簇扩展。
 *
 * 协作: BirchClustering8(BIRCH聚类) / KMeans18(K均值) / GaussianMixture15(高斯混合)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief DBSCAN聚类(混合网格/HNSW索引+并行扩展)
 */
class DBSCAN11 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numPoints = 0;
        int numClusters = 0;
        int numNoise = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Grid cell for spatial indexing */
    struct GridCell {
        QVector<int> pointIndices;
    };

    /** @brief HNSW layer node */
    struct HNSWNode {
        int id = -1;
        QVector<int> neighbors;  // connections per layer
    };

    explicit DBSCAN11(QObject *parent = nullptr);
    ~DBSCAN11() override;

    void setEpsilon(double eps);
    void setMinPoints(int minPts);
    void setGridResolution(int res);
    void setHNSWMaxConnections(int m);
    void setParallelThreshold(int threshold);

    /** @brief Run DBSCAN clustering on data points */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Query epsilon-neighborhood for a point using hybrid index */
    QVector<int> rangeQuery(int pointIdx) const;

    /** @brief Get cluster centers (mean of each cluster) */
    QVector<QVector<double>> clusterCenters() const;

    /** @brief Get noise point indices */
    QVector<int> noiseIndices() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, int noise, double timeMs);

private:
    double m_epsilon = 0.5;
    int m_minPoints = 5;
    int m_gridRes = 32;
    int m_hnswM = 16;
    int m_parallelThreshold = 1000;

    QVector<QVector<double>> m_data;
    QVector<int> m_labels;      // -1=unvisited, -2=noise, >=0=cluster
    QVector<QVector<double>> m_centers;
    int m_dim = 0;

    // Grid index
    QVector<GridCell> m_grid;
    double m_gridScale = 1.0;

    // HNSW index
    QVector<HNSWNode> m_hnswNodes;
    int m_hnswMaxLayer = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build grid spatial index */
    void buildGridIndex();

    /** @brief Build HNSW graph index */
    void buildHNSWIndex();

    /** @brief Grid cell key for a point */
    int gridKey(const QVector<double>& pt) const;

    /** @brief Euclidean distance between two points */
    double distance(int i, int j) const;

    /** @brief Expand cluster from seed point via BFS */
    int expandCluster(int seedIdx, int clusterId);

    /** @brief HNSW search for nearest neighbors within epsilon */
    QVector<int> hnswRangeSearch(int queryIdx) const;

    /** @brief Grid-based range query */
    QVector<int> gridRangeQuery(int queryIdx) const;
};
