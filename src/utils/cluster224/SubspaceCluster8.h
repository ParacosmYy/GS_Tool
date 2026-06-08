/**
 * @file SubspaceCluster8.h
 * @brief 子空间聚类(INSCY密度子空间聚类+冗余感知子空间选择) — Subspace Clustering with INSCY Density-Based Subspace Clustering and Redundancy-Aware Subspace Selection
 *
 * 功能: 实现INSCY密度子空间聚类算法，在多个子空间维度上发现密集区域，
 *       并通过冗余感知选择策略筛选高质量、非冗余的子空间聚类结果。
 *
 * 协作: HierarchicalCluster10(层次聚类) / DBSCAN6(密度聚类) / KMedoids18(K中心点)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 子空间聚类(INSCY密度+冗余感知选择)
 */
class SubspaceCluster8 : public QObject {
    Q_OBJECT

public:
    /** @brief Subspace clustering result for one subspace */
    struct SubspaceCluster {
        QVector<int> dimensions;       // subspace dimension indices
        QVector<QVector<int>> clusters; // point-index groups per cluster
        double quality = 0.0;          // cluster quality score
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int fullDim = 0;
        int maxSubspaceDim = 0;
        int numSubspacesExplored = 0;
        int numClustersFound = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SubspaceCluster8(QObject *parent = nullptr);
    ~SubspaceCluster8() override;

    /** @brief Set clustering parameters */
    void setParameters(double minDensity, int minPoints,
                       double redundancyThreshold = 0.7,
                       int maxSubspaceDim = 0);

    /** @brief Run INSCY subspace clustering on high-dimensional data */
    QVector<SubspaceCluster> fit(const QVector<QVector<double>>& data);

    /** @brief Compute redundancy between two subspace clusters (Jaccard) */
    double redundancy(const SubspaceCluster& a,
                      const SubspaceCluster& b) const;

    /** @brief Select non-redundant clusters via greedy quality maximization */
    QVector<SubspaceCluster> selectNonRedundant(
        const QVector<SubspaceCluster>& candidates) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int subspaces, int clusters, double timeMs);

private:
    double m_minDensity = 0.3;
    int m_minPoints = 5;
    double m_redundancyThreshold = 0.7;
    int m_maxSubspaceDim = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Project data onto given dimensions */
    QVector<QVector<double>> project(
        const QVector<QVector<double>>& data,
        const QVector<int>& dims) const;

    /** @brief INSCY density-based clustering in a subspace */
    QVector<QVector<int>> inscyCluster(
        const QVector<QVector<double>>& projected,
        double epsilon) const;

    /** @brief Generate all subspace dimension combinations */
    QVector<QVector<int>> generateSubspaces(
        int fullDim, int maxDim) const;

    /** @brief Compute cluster quality (silhouette-like) */
    double clusterQuality(const QVector<QVector<double>>& projected,
                          const QVector<QVector<int>>& clusters) const;

    /** @brief Euclidean distance in projected space */
    double euclidean(const QVector<double>& a,
                     const QVector<double>& b) const;
};
