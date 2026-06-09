/**
 * @file SubspaceCluster9.h
 * @brief 子空间聚类(PROCLUS轴平行投影+质量引导中心点精炼) — Subspace Clustering with PROCLUS Axis-Parallel Projection and Quality-Guided Medoid Refinement
 *
 * 功能: 实现子空间聚类(Subspace clustering)，采用PROCLUS算法(PROCLUS algorithm)通过
 *       轴平行投影(axis-parallel projection)发现高维子空间中的簇，利用质量引导中心点精炼
 *       (quality-guided medoid refinement)迭代优化中心点与子空间选择，实现高效高维聚类。
 *
 * 协作: HierarchicalCluster11(层次聚类) / KMedoids19(K-中心点) / GaussianMixture23(高斯混合)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 子空间聚类(PROCLUS轴平行投影+质量引导中心点精炼)
 */
class SubspaceCluster9 : public QObject {
    Q_OBJECT

public:
    /** @brief Cluster result with associated subspace dimensions */
    struct ClusterResult {
        QVector<int> pointIndices;
        QVector<int> subspaceDims;
        double quality = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numDimensions = 0;
        int numClusters = 0;
        int numIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SubspaceCluster9(QObject *parent = nullptr);
    ~SubspaceCluster9() override;

    /** @brief Set number of clusters k */
    void setNumClusters(int k);

    /** @brief Set average subspace dimensionality l */
    void setSubspaceSize(int l);

    /** @brief Run PROCLUS clustering on [n x d] data */
    QVector<ClusterResult> fit(const QVector<QVector<double>>& data);

    /** @brief Get cluster label for each point */
    QVector<int> labels() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationCompleted(int iter, double quality);
    void fitCompleted(int k, int iterations, double totalTimeMs);

private:
    int m_k = 3;
    int m_l = 2;
    int m_maxIter = 20;

    QVector<QVector<double>> m_data;
    QVector<int> m_labels;
    QVector<ClusterResult> m_clusters;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Manhattan segmental distance in subspace */
    double manhattanDist(const QVector<double>& a, const QVector<double>& b,
                         const QVector<int>& dims) const;

    /** @brief Compute quality of a set of medoids with subspaces */
    double computeQuality(const QVector<int>& medoids,
                          const QVector<QVector<int>>& subspaces) const;

    /** @brief Select initial medoids via greedy farthest-first */
    QVector<int> selectInitialMedoids(int n) const;

    /** @brief Determine subspace dimensions for a medoid */
    QVector<int> findSubspace(int medoid, const QVector<int>& neighbors) const;
};
