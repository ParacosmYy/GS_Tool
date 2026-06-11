/**
 * @file SubspaceCluster13.h
 * @brief 子空间聚类(PROCLUS维度选择与K-中心点精炼实现高维投影聚类发现) — Subspace Clustering with PROCLUS Dimension Selection and K-medoid Refinement for High-dimensional Projected Cluster Discovery
 *
 * 功能: 实现子空间聚类(subspace clustering)，采用PROCLUS维度选择(PROCLUS dimension selection)
 *       与K-中心点精炼(k-medoid refinement)实现高维投影聚类发现(high-dimensional projected cluster discovery)。
 *
 * 协作: HierarchicalCluster15(层次聚类) / KMedoids23(K-中心点) / GaussianMixture35(高斯混合模型)
 */
#pragma once

#include <QObject>
#include <QVector>

class SubspaceCluster13 : public QObject {
    Q_OBJECT

public:
    /** @brief A single projected cluster result */
    struct ProjectedCluster {
        QVector<int> pointIndices;
        QVector<int> selectedDims;
        QVector<double> medoid;
        double avgDistance = 0.0;
    };

    /** @brief Full clustering output */
    struct ClusterResult {
        QVector<ProjectedCluster> clusters;
        QVector<int> assignments;
        double qualityScore = 0.0;
        int numClusters = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalFits = 0;
        int numPoints = 0;
        int numDims = 0;
        double avgQuality = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SubspaceCluster13(QObject *parent = nullptr);
    ~SubspaceCluster13() override;

    void setNumClusters(int k);
    void setAvgDimSelection(int l);

    /** @brief Run PROCLUS subspace clustering on data matrix */
    ClusterResult fit(const QVector<QVector<double>>& data);

    /** @brief Compute Manhattan segmental distance in selected dimensions */
    double manhattanSegmental(const QVector<double>& a,
                               const QVector<double>& b,
                               const QVector<int>& dims) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitDone(int n, int k, double quality, double timeMs);

private:
    int m_k = 3;
    int m_l = 5;
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_qualitySum = 0.0;

    /** @brief Find initial medoids via greedy farthest-first */
    QVector<int> findInitialMedoids(const QVector<QVector<double>>& data,
                                     int k) const;

    /** @brief Select dimensions for each medoid using PROCLUS locality */
    QVector<QVector<int>> selectDimensions(
        const QVector<QVector<double>>& data,
        const QVector<int>& medoids) const;

    /** @brief Assign points to nearest medoid in their projected subspace */
    QVector<int> assignPoints(const QVector<QVector<double>>& data,
                               const QVector<int>& medoids,
                               const QVector<QVector<int>>& dimSets) const;

    /** @brief Refine medoids using k-medoid swap strategy */
    QVector<int> refineMedoids(const QVector<QVector<double>>& data,
                                const QVector<int>& medoids,
                                const QVector<int>& assignments) const;

    /** @brief Compute cluster quality (silhouette-like) */
    double computeQuality(const QVector<QVector<double>>& data,
                           const QVector<int>& assignments,
                           const QVector<QVector<int>>& dimSets) const;
};
