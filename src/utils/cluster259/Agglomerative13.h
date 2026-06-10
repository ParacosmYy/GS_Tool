/**
 * @file Agglomerative13.h
 * @brief 层次聚合聚类(WPGMC加权质心中位链接距离单调树状图构建) — Agglomerative Clustering with WPGMC Weighted Centroid and Median Linkage for Distance-Monotone Dendrogram Construction
 *
 * 功能: 实现层次聚合聚类(agglomerative clustering)，采用WPGMC加权质心
 *       (WPGMC weighted centroid)和中位链接(median linkage)进行距离单调
 *       (distance-monotone)树状图构建(dendrogram construction)。
 *
 * 协作: KMeans27(K均值聚类) / DBSCAN15(密度聚类) / GaussianMixture28(高斯混合)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 层次聚合聚类(WPGMC加权质心中位链接距离单调树状图构建)
 */
class Agglomerative13 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        int numDimensions = 0;
        int numMerges = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Single merge step in the dendrogram */
    struct MergeStep {
        int clusterA = -1;
        int clusterB = -1;
        double distance = 0.0;
        int newSize = 0;
    };

    /** @brief A cluster node with weighted centroid */
    struct ClusterNode {
        QVector<double> centroid;
        QVector<int> memberIndices;
        double weight = 1.0;
        bool active = true;
    };

    explicit Agglomerative13(QObject *parent = nullptr);
    ~Agglomerative13() override;

    /** @brief Set target number of clusters (0 = full dendrogram) */
    void setNumClusters(int k);

    /** @brief Run agglomerative clustering */
    bool fit(const QVector<QVector<double>>& data);

    /** @brief Get merge history (dendrogram) */
    QVector<MergeStep> dendrogram() const;

    /** @brief Get current cluster assignments */
    QVector<int> labels() const;

    /** @brief Get all cluster nodes */
    QVector<ClusterNode> clusters() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numClusters, int numMerges, double timeMs);
    void mergePerformed(int idA, int idB, double distance);

private:
    int m_targetK = 0;
    int m_dims = 0;
    int m_n = 0;

    QVector<ClusterNode> m_nodes;
    QVector<MergeStep> m_merges;
    QVector<int> m_labels;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief WPGMC distance between two cluster nodes */
    double wpgmcDistance(const ClusterNode& a, const ClusterNode& b) const;

    /** @brief Median linkage distance between two clusters */
    double medianLinkage(const ClusterNode& a, const ClusterNode& b) const;

    /** @brief Compute WPGMC weighted centroid of merged cluster */
    ClusterNode mergeClusters(const ClusterNode& a, const ClusterNode& b, int newId);

    /** @brief Find the pair of active clusters with minimum distance */
    bool findMinPair(int& outA, int& outB, double& outDist) const;

    /** @brief Euclidean distance between two points */
    double euclidean(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Assign flat labels from dendrogram */
    void assignLabels();
};
