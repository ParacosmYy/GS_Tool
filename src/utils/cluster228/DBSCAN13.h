/**
 * @file DBSCAN13.h
 * @brief DBSCAN密度聚类(HNSW加速近邻搜索+局部可达密度自适应epsilon) — DBSCAN with HNSW-accelerated Neighbor Search and Variable-density Adaptive Epsilon via Local Reachability
 *
 * 功能: 实现DBSCAN聚类算法，使用HNSW(Hierarchical Navigable Small World)图加速近邻搜索，
 *       通过局部可达密度(local reachability density)自适应调整epsilon参数，处理变密度数据。
 *
 * 协作: KMeans22(K均值) / GaussianMixture21(高斯混合) / KMedoids18(K中心点)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief DBSCAN聚类(HNSW加速+局部可达密度自适应epsilon)
 */
class DBSCAN13 : public QObject {
    Q_OBJECT

public:
    /** @brief Cluster label: -1 = noise, 0+ = cluster id */
    struct PointLabel {
        int cluster = -1;
        bool visited = false;
        double localReachDensity = 0.0;
        double adaptiveEpsilon = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numClusters = 0;
        int numDimensions = 0;
        int numNoise = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DBSCAN13(QObject *parent = nullptr);
    ~DBSCAN13() override;

    /** @brief Set base epsilon, minPts, and HNSW layer parameter */
    void setParameters(double epsilon, int minPts, int hnswM = 16);

    /** @brief Fit DBSCAN to 2-D data */
    bool fit(const QVector<QVector<double>>& data);

    /** @brief Get cluster labels for all points */
    QVector<int> labels() const;

    /** @brief Get per-point adaptive epsilon values */
    QVector<double> adaptiveEpsilons() const;

    /** @brief Count of clusters found */
    int clusterCount() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitCompleted(int clusters, int noise, double timeMs);
    void clusterExpanded(int clusterId, int size);

private:
    double m_epsilon = 0.5;
    int m_minPts = 5;
    int m_hnswM = 16;

    QVector<QVector<double>> m_data;
    QVector<PointLabel> m_labels;
    int m_clusterCount = 0;

    // HNSW graph: adjacency list per node
    QVector<QVector<int>> m_hnswGraph;
    QVector<int> m_entryPoint;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Euclidean distance between two points */
    double distance(int i, int j) const;

    /** @brief Build HNSW index for fast neighbor lookup */
    void buildHNSW();

    /** @brief HNSW search: find approximate k-nearest neighbors */
    QVector<int> hnswSearch(const QVector<double>& query, int k) const;

    /** @brief Compute local reachability density for a point */
    double computeLRD(int idx) const;

    /** @brief Compute adaptive epsilon using local reachability */
    double computeAdaptiveEpsilon(int idx) const;

    /** @brief Region query using HNSW-accelerated search */
    QVector<int> regionQuery(int idx, double eps) const;

    /** @brief Expand cluster from seed point */
    void expandCluster(int idx, int clusterId, double eps);
};
