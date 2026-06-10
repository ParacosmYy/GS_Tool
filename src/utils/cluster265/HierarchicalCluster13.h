/**
 * @file HierarchicalCluster13.h
 * @brief 层次聚类(Ward最小方差法与Lance-Williams递推凝聚合并聚合聚类) — Hierarchical Clustering with Ward's Minimum Variance Method and Lance-Williams Recurrence for Agglomerative Merging
 *
 * 功能: 实现层次聚类(Hierarchical clustering)，采用Ward最小方差法(Ward's minimum variance)
 *       和Lance-Williams递推公式(Lance-Williams recurrence)实现凝聚合并(agglomerative merging)。
 *
 * 协作: KMedoids21(K-中心点) / GaussianMixture29(高斯混合) / SpectralCluster14(谱聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 层次聚类(Ward最小方差法与Lance-Williams递推凝聚合并聚合聚类)
 */
class HierarchicalCluster13 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int dimension = 0;
        int numMerges = 0;
        double finalDistance = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Single merge step in the dendrogram */
    struct MergeStep {
        int clusterA = -1;
        int clusterB = -1;
        double distance = 0.0;
        int newSize = 0;
    };

    explicit HierarchicalCluster13(QObject *parent = nullptr);
    ~HierarchicalCluster13() override;

    /** @brief Set linkage method: 0=Ward, 1=Single, 2=Complete, 3=Average */
    void setLinkage(int method);

    /** @brief Run agglomerative clustering, returns merge history */
    QVector<MergeStep> fit(const QVector<QVector<double>>& data);

    /** @brief Cut dendrogram at given distance threshold */
    QVector<int> cutDendrogram(double threshold) const;

    /** @brief Cut dendrogram to obtain exactly k clusters */
    QVector<int> cutToKClusters(int k) const;

    /** @brief Get full merge history */
    QVector<MergeStep> mergeHistory() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringUpdated(int numClusters, double distance, double timeMs);

private:
    int m_linkage = 0;  // 0=Ward
    int m_n = 0;
    int m_dim = 0;

    QVector<MergeStep> m_merges;
    QVector<int> m_labels;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Squared Euclidean distance between two points */
    double sqEuclidean(const QVector<double>& a,
                       const QVector<double>& b) const;

    /** @brief Lance-Williams update for distance between merged(ij) and k */
    double lanceWilliams(double dij, double dik, double djk,
                         int si, int sj, int sk) const;

    /** @brief Find index of minimum value in condensed distance matrix */
    int findMinIndex(const QVector<double>& dist, int activeCount,
                     const QVector<int>& activeMap) const;
};
