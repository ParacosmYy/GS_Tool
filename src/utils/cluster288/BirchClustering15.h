/**
 * @file BirchClustering15.h
 * @brief BIRCH聚类(动态阈值调整与子簇合并支持增量式内存感知流数据聚类) — BIRCH with Dynamic Threshold Adjustment and Subcluster Merging for Incremental Memory-aware Clustering of Streaming Data
 *
 * 功能: 实现BIRCH聚类(BIRCH clustering)，采用动态阈值调整(dynamic threshold adjustment)
 *       与子簇合并(subcluster merging)支持增量式内存感知流数据聚类(incremental memory-aware streaming data clustering)。
 *
 * 协作: Agglomerative15(层次聚类) / KMeans31(K-means) / DBSCAN17(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief BIRCH聚类(动态阈值调整与子簇合并支持增量式内存感知流数据聚类)
 */
class BirchClustering15 : public QObject {
    Q_OBJECT

public:
    /** @brief CF (Clustering Feature) subcluster node */
    struct CFNode {
        int n = 0;                          // Number of points
        QVector<double> linearSum;          // Linear sum of dimensions
        QVector<double> squareSum;          // Square sum of dimensions
        QVector<int> childIndices;          // Child CF indices (non-leaf)
        bool isLeaf = true;
        int parent = -1;
    };

    /** @brief Clustering result */
    struct ClusterResult {
        QVector<int> labels;               // Point-to-cluster assignment
        QVector<QVector<double>> centroids; // Final cluster centers
        int numClusters = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numSubclusters = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BirchClustering15(QObject *parent = nullptr);
    ~BirchClustering15() override;

    void setThreshold(double T);
    void setBranchingFactor(int B);
    void setMaxPoints(int maxPts);
    void setNumClusters(int k);

    /** @brief Insert a single data point into the CF tree */
    void insertPoint(const QVector<double>& point);

    /** @brief Batch insert and cluster */
    ClusterResult fit(const QVector<QVector<double>>& data);

    /** @brief Get current subcluster centroids */
    QVector<QVector<double>> getSubclusterCentroids() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitDone(int n, int k, int numSub, double timeMs);

private:
    double m_threshold = 0.5;
    int m_branching = 50;
    int m_maxPoints = 100000;
    int m_k = 8;
    int m_dims = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<CFNode> m_nodes;
    int m_root = -1;
    int m_leafHead = -1;

    /** @brief Compute radius of a CF node */
    double cfRadius(const CFNode& cf) const;

    /** @brief Compute distance between point and CF centroid */
    double distToCF(const QVector<double>& point, const CFNode& cf) const;

    /** @brief Merge point into existing CF */
    void mergeIntoCF(CFNode& cf, const QVector<double>& point);

    /** @brief Merge two CFs */
    void mergeCFs(CFNode& dst, const CFNode& src);

    /** @brief Find closest leaf subcluster */
    int findClosestLeaf(const QVector<double>& point) const;

    /** @brief Split an overfull node */
    int splitNode(int nodeIdx);

    /** @brief Adjust threshold dynamically based on tree size */
    void adjustThreshold();

    /** @brief Collect all leaf CFs */
    void collectLeaves(int idx, QVector<int>& leaves) const;

    /** @brief Apply global clustering on leaf subclusters via k-means */
    ClusterResult globalClustering(const QVector<QVector<double>>& data);
};
