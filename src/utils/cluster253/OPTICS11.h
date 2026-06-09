/**
 * @file OPTICS11.h
 * @brief OPTICS聚类(Xi簇提取+层次可达距离边界检测) — OPTICS with Xi Cluster Extraction Method and Hierarchical Reachability-Based Cluster Boundary Detection
 *
 * 功能: 实现OPTICS排序算法(Ordering Points To Identify the Clustering
 *       Structure)，通过Xi簇提取方法(Xi cluster extraction)从可达距离
 *       图中提取层次聚类结构，支持基于可达距离的簇边界检测(cluster
 *       boundary detection)。
 *
 * 协作: DBSCAN19(密度聚类) / SubspaceCluster10(子空间聚类) / KMedoids20(K中心点)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief OPTICS聚类(Xi簇提取+层次可达距离边界)
 */
class OPTICS11 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numClusters = 0;
        int numCorePoints = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief A cluster extracted from reachability plot */
    struct Cluster {
        QVector<int> pointIndices;
        double startXi = 0.0;
        double endXi = 0.0;
    };

    explicit OPTICS11(QObject *parent = nullptr);
    ~OPTICS11() override;

    /** @brief Set epsilon neighborhood radius */
    void setEpsilon(double eps);

    /** @brief Set minimum points for core point criterion */
    void setMinPoints(int minPts);

    /** @brief Set Xi parameter for cluster extraction */
    void setXi(double xi);

    /** @brief Run OPTICS ordering, return reachability distances */
    QVector<double> fit(const QVector<QVector<double>>& data);

    /** @brief Extract clusters from reachability plot using Xi method */
    QVector<Cluster> extractClusters() const;

    /** @brief Get ordered point indices */
    QVector<int> ordering() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numClusters, int corePoints, double timeMs);

private:
    double m_epsilon = 1.0;
    int m_minPts = 5;
    double m_xi = 0.05;
    int m_n = 0;
    int m_dims = 0;

    QVector<QVector<double>> m_data;
    QVector<double> m_reachDist;    // Reachability distance per point
    QVector<double> m_coreDist;     // Core distance per point
    QVector<int> m_order;           // OPTICS ordering
    QVector<bool> m_processed;
    QVector<Cluster> m_clusters;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Euclidean distance between two points */
    double distance(int i, int j) const;

    /** @brief Compute core distance for a point */
    double computeCoreDist(int idx, const QVector<int>& neighbors) const;

    /** @brief Find epsilon-neighborhood of a point */
    QVector<int> findNeighbors(int idx) const;

    /** @brief Update reachability distances for neighbors */
    void updateSeeds(int idx, QVector<int>& seeds, QVector<double>& seedsRD);

    /** @brief Xi cluster extraction from reachability plot */
    void extractXiClusters();
};
