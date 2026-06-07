/**
 * @file HierarchicalCluster8.h
 * @brief 层次聚类(WPGMA/WPGMO加权+不一致链切割) — Hierarchical Clustering with WPGMA/WPGMO Weighting and Inconsistent-Link Cluster Cutting
 *
 * 功能: 实现层次聚类算法，支持WPGMA/WPGMO加权策略、
 *       不一致系数链切割和树状图构建。
 *
 * 协作: KMedoids16(K中心点) / DBSCAN11(DBSCAN) / GaussianMixture16(高斯混合)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 层次聚类(WPGMA/WPGMO加权+不一致链切割)
 */
class HierarchicalCluster8 : public QObject {
    Q_OBJECT

public:
    /** @brief Linkage method */
    enum class Linkage { Single, Complete, WPGMA, WPGMO };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalFits = 0;
        int numSamples = 0;
        int numClusters = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HierarchicalCluster8(QObject *parent = nullptr);
    ~HierarchicalCluster8() override;

    void setNumClusters(int k);
    void setLinkage(Linkage method);
    void setInconsistentThreshold(double t);
    void setMaxInconsistentDepth(int depth);

    /** @brief Build dendrogram from distance matrix */
    QVector<QVector<double>> buildDendrogram(const QVector<QVector<double>>& distMatrix);

    /** @brief Cut dendrogram into k clusters */
    QVector<int> cutTree(const QVector<QVector<double>>& dendrogram, int k) const;

    /** @brief Cut using inconsistent-link criterion */
    QVector<int> cutInconsistent(const QVector<QVector<double>>& dendrogram) const;

    /** @brief Compute inconsistency coefficient for merge step */
    QVector<double> computeInconsistency(const QVector<QVector<double>>& dendrogram) const;

    /** @brief Full clustering pipeline */
    QVector<int> fit(const QVector<QVector<double>>& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, int samples, double timeMs);

private:
    int m_k = 2;
    Linkage m_linkage = Linkage::WPGMA;
    double m_inconsistentThreshold = 1.15;
    int m_maxDepth = 2;

    QVector<QVector<double>> m_distMatrix;
    QVector<QVector<double>> m_dendrogram;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Find minimum distance pair in condensed matrix */
    QPair<int, int> findMinPair(const QVector<double>& condensed, int n) const;

    /** @brief Compute WPGMA/WPGMO merged distance */
    double mergedDistance(double dij, double dik, double djk,
                          int si, int sj, int sk) const;

    /** @brief Map cluster index to condensed position */
    int condensedIndex(int i, int j, int n) const;

    /** @brief Union-Find: find root */
    QVector<int> findComponents(int n, const QVector<QVector<double>>& dendro,
                                 int numMerges) const;
};
