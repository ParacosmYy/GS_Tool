/**
 * @file HierarchicalCluster12.h
 * @brief 层次聚类(UPGMA非加权平均+共表距离矩阵构建有根树) — Hierarchical Clustering with UPGMA Unweighted Average and Cophenetic Distance Matrix for Rooted Tree Construction
 *
 * 功能: 实现层次聚类算法，使用UPGMA(Unweighted Pair Group Method with
 *       Arithmetic Mean)非加权配对平均法进行聚类合并，计算共表距离矩阵
 *       (cophenetic distance matrix)构建有根系统发育树(rooted tree)。
 *
 * 协作: KMedoids20(K中心点聚类) / KMeans25(K均值) / GaussianMixture26(高斯混合)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 层次聚类(UPGMA+共表距离矩阵)
 */
class HierarchicalCluster12 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        int numClusters = 0;
        int numMerges = 0;
        double copheneticCorrelation = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Merge event in dendrogram */
    struct MergeEvent {
        int clusterA = -1;
        int clusterB = -1;
        double distance = 0.0;
        int newSize = 0;
    };

    explicit HierarchicalCluster12(QObject *parent = nullptr);
    ~HierarchicalCluster12() override;

    /** @brief Set target number of clusters */
    void setTargetClusters(int k);

    /** @brief Perform hierarchical clustering, return cluster labels */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Get the dendrogram merge sequence */
    QVector<MergeEvent> dendrogram() const;

    /** @brief Compute cophenetic distance matrix from dendrogram */
    QVector<QVector<double>> copheneticMatrix() const;

    /** @brief Compute cophenetic correlation coefficient */
    double copheneticCorrelation(const QVector<QVector<double>>& originalDist) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int k, double correlation, double timeMs);

private:
    int m_targetK = 2;
    int m_n = 0;

    QVector<MergeEvent> m_merges;
    QVector<QVector<double>> m_distMatrix;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute Euclidean distance between two points */
    double euclidean(const QVector<QVector<double>>& data, int i, int j) const;

    /** @brief Initialize distance matrix from data */
    void initDistanceMatrix(const QVector<QVector<double>>& data);

    /** @brief UPGMA merge: update distances with unweighted average */
    void upgmaMerge(int a, int b, const QVector<int>& sizes,
                    QVector<int>& active);
};
