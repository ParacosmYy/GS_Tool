/**
 * @file SubspaceCluster12.h
 * @brief 子空间聚类(PROCLUS中心点投影与k-medoid精炼的轴平行子空间发现) — Subspace Clustering with PROCLUS Medoid-based Projection and k-medoid Refinement for Axis-parallel Subspace Discovery
 *
 * 功能: 实现子空间聚类(subspace clustering)，采用PROCLUS中心点投影(PROCLUS medoid-based projection)
 *       与k-medoid精炼(k-medoid refinement)实现轴平行子空间发现(axis-parallel subspace discovery)。
 *
 * 协作: HierarchicalCluster14(层次聚类) / KMedoids22(K中心点) / DBSCAN16(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 子空间聚类(PROCLUS中心点投影与k-medoid精炼)
 */
class SubspaceCluster12 : public QObject {
    Q_OBJECT

public:
    /** @brief A single subspace cluster result */
    struct SubspaceCluster {
        QVector<int> pointIndices;
        QVector<int> relevantDimensions;
        double quality = 0.0;
    };

    /** @brief Full clustering result */
    struct ClusterResult {
        QVector<SubspaceCluster> clusters;
        double totalQuality = 0.0;
        int numIterations = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numDimensions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SubspaceCluster12(QObject *parent = nullptr);
    ~SubspaceCluster12() override;

    /** @brief Set target number of clusters k */
    void setNumClusters(int k);

    /** @brief Set average subspace dimensionality l */
    void setAvgSubspaceDim(int l);

    /** @brief Run PROCLUS subspace clustering */
    ClusterResult fit(const QVector<QVector<double>>& data);

    /** @brief Compute distance in a specific subspace */
    double subspaceDistance(const QVector<double>& a, const QVector<double>& b,
                            const QVector<int>& dims) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationDone(int iter, int k, double quality, double timeMs);
    void fittingDone(int n, int k, double quality, double timeMs);

private:
    int m_k = 3;
    int m_l = 2;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Select initial medoids via greedy farthest-first */
    QVector<int> selectMedoids(const QVector<QVector<double>>& data, int k) const;

    /** @brief Find relevant dimensions for each medoid */
    QVector<QVector<int>> findRelevantDimensions(
        const QVector<QVector<double>>& data,
        const QVector<int>& medoids) const;

    /** @brief Assign points to nearest medoid in their subspace */
    QVector<int> assignPoints(const QVector<QVector<double>>& data,
                               const QVector<int>& medoids,
                               const QVector<QVector<int>>& subspaces) const;

    /** @brief Refine medoids using k-medoid swaps */
    QVector<int> refineMedoids(const QVector<QVector<double>>& data,
                                const QVector<int>& medoids,
                                const QVector<int>& labels,
                                const QVector<QVector<int>>& subspaces) const;

    /** @brief Compute cluster quality (isolation metric) */
    double computeQuality(const QVector<QVector<double>>& data,
                           const QVector<int>& medoids,
                           const QVector<QVector<int>>& subspaces) const;
};
