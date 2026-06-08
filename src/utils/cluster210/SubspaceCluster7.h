/**
 * @file SubspaceCluster7.h
 * @brief 子空间聚类(PROCLUS投影聚类+熵质量评估) — Subspace Clustering with PROCLUS Projected Clustering and Entropy-Based Subspace Quality Measure
 *
 * 功能: 实现PROCLUS投影聚类算法，支持子空间发现、
 *       熵质量评估和迭代medoid优化。
 *
 * 协作: HierarchicalCluster9(层次聚类) / KMedoids17(K中心点) / SpectralCluster10(谱聚类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 子空间聚类(PROCLUS投影聚类+熵质量评估)
 */
class SubspaceCluster7 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numDimensions = 0;
        int numClusters = 0;
        double avgSubspaceQuality = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief A subspace cluster result */
    struct SubspaceCluster {
        QVector<int> pointIndices;       // Member point indices
        QVector<int> selectedDimensions; // Relevant dimensions
        double quality = 0.0;            // Entropy-based quality
        double avgDistance = 0.0;         // Average intra-cluster distance
    };

    explicit SubspaceCluster7(QObject *parent = nullptr);
    ~SubspaceCluster7() override;

    /** @brief Set number of clusters k and subspace dimensionality l */
    void setParameters(int k, int subspaceDim);

    /** @brief Fit model: run PROCLUS clustering */
    void fit(const QVector<QVector<double>>& data);

    /** @brief Get discovered subspace clusters */
    QVector<SubspaceCluster> clusters() const;

    /** @brief Compute entropy-based quality of a subspace */
    double subspaceQuality(const QVector<int>& dimensions,
                           const QVector<int>& pointIndices,
                           const QVector<QVector<double>>& data) const;

    /** @brief Get cluster labels for each point */
    QVector<int> labels() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, double quality, double timeMs);

private:
    int m_k = 3;
    int m_l = 2;
    int m_n = 0;
    int m_dim = 0;

    QVector<SubspaceCluster> m_clusters;
    QVector<int> m_labels;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute Manhattan distance in a subspace */
    static double manhattanSubspace(const QVector<double>& a,
                                    const QVector<double>& b,
                                    const QVector<int>& dims);

    /** @brief Select initial medoids using greedy strategy */
    QVector<int> selectMedoids(const QVector<QVector<double>>& data, int k) const;

    /** @brief Find locality for each medoid */
    QVector<QVector<int>> computeLocalities(const QVector<int>& medoids,
                                             const QVector<QVector<double>>& data) const;

    /** @brief Select best dimensions for each locality */
    QVector<QVector<int>> selectDimensions(
        const QVector<QVector<int>>& localities,
        const QVector<QVector<double>>& data, int l) const;

    /** @brief Assign points to nearest medoid in its subspace */
    QVector<int> assignPoints(const QVector<int>& medoids,
                               const QVector<QVector<int>>& dimensions,
                               const QVector<QVector<double>>& data) const;

    /** @brief Compute entropy of value distribution in one dimension */
    double dimensionEntropy(const QVector<double>& values) const;
};
