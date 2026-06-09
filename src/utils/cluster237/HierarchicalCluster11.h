/**
 * @file HierarchicalCluster11.h
 * @brief 层次聚类(Ward链接+Lance-Williams相异性更新+最小方差合并) — Hierarchical Clustering with Ward Linkage and Lance-Williams Dissimilarity Update for Minimum Variance Merging
 *
 * 功能: 实现层次聚类(Hierarchical clustering)，采用Ward链接(Ward linkage)在每步合并中选择
 *       使簇内方差增量最小的簇对，利用Lance-Williams相异性更新公式(Lance-Williams dissimilarity
 *       update)高效维护簇间距离，实现最小方差合并(minimum variance merging)。
 *
 * 协作: KMedoids19(K-中心点) / GaussianMixture23(高斯混合) / DBSCAN12(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 层次聚类(Ward链接+Lance-Williams相异性更新+最小方差合并)
 */
class HierarchicalCluster11 : public QObject {
    Q_OBJECT

public:
    /** @brief Merge step record */
    struct MergeStep {
        int clusterA = -1;
        int clusterB = -1;
        double distance = 0.0;
        int newSize = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numDimensions = 0;
        int numClusters = 0;
        int numMerges = 0;
        double finalDistance = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HierarchicalCluster11(QObject *parent = nullptr);
    ~HierarchicalCluster11() override;

    /** @brief Set desired number of clusters */
    void setNumClusters(int k);

    /** @brief Run agglomerative clustering on [n x d] data */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Cut dendrogram at given distance threshold */
    QVector<int> cutAtDistance(double threshold) const;

    /** @brief Get full merge history (dendrogram) */
    QVector<MergeStep> mergeHistory() const;

    /** @brief Get cluster assignments */
    QVector<int> labels() const;

    /** @brief Compute cophenetic correlation coefficient */
    double copheneticCorrelation(const QVector<QVector<double>>& origDist) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void mergeCompleted(int step, int clusterA, int clusterB, double dist);
    void fitCompleted(int k, int merges, double totalTimeMs);

private:
    int m_k = 2;

    QVector<QVector<double>> m_data;
    QVector<int> m_labels;
    QVector<MergeStep> m_history;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Squared Euclidean distance */
    double sqDist(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Lance-Williams update for Ward linkage */
    double lanceWilliamsUpdate(double dAi, double dBi, double dAB,
                               int sizeA, int sizeB, int sizeI) const;

    /** @brief Union-Find: find root with path compression */
    int ufFind(QVector<int>& parent, int i) const;

    /** @brief Union-Find: union by size */
    void ufUnion(QVector<int>& parent, QVector<int>& size, int a, int b);
};
