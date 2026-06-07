/**
 * @file HierarchicalCluster9.h
 * @brief 层次聚类(Lance-Williams柔性更新+共表相关验证) — Hierarchical Clustering with Lance-Williams Flexible Update and Cophenetic Correlation Validation
 *
 * 功能: 实现凝聚层次聚类，支持Lance-Williams柔性更新公式、
 *       共表距离相关系数验证和多种链接策略。
 *
 * 协作: KMedoids17(K中心点) / GaussianMixture17(高斯混合) / SpectralCluster10(谱聚类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 层次聚类(Lance-Williams柔性更新+共表相关验证)
 */
class HierarchicalCluster9 : public QObject {
    Q_OBJECT

public:
    /** @brief Linkage method enumeration */
    enum Linkage {
        SingleLink = 0,   // Single linkage (minimum)
        CompleteLink = 1,  // Complete linkage (maximum)
        AverageLink = 2,   // Group average (UPGMA)
        WardLink = 3       // Ward's minimum variance
    };

    /** @brief Merge event in the dendrogram */
    struct MergeEvent {
        int clusterA;      // First merged cluster index
        int clusterB;      // Second merged cluster index
        double distance;   // Distance at merge
        int newSize;       // Size of merged cluster
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numDimensions = 0;
        int numMerges = 0;
        double copheneticCorrelation = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HierarchicalCluster9(QObject *parent = nullptr);
    ~HierarchicalCluster9() override;

    void setLinkage(Linkage method);
    void setDistanceMetric(int metric);  // 0=euclidean, 1=manhattan

    /** @brief Fit model: compute full hierarchy */
    void fit(const QVector<QVector<double>>& data);

    /** @brief Cut dendrogram at k clusters */
    QVector<int> cutTree(int k) const;

    /** @brief Cut dendrogram at distance threshold */
    QVector<int> cutAtDistance(double threshold) const;

    /** @brief Get dendrogram merge sequence */
    QVector<MergeEvent> dendrogram() const;

    /** @brief Compute cophenetic correlation coefficient */
    double copheneticCorrelation(const QVector<QVector<double>>& data) const;

    /** @brief Get cluster labels */
    QVector<int> labels() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int merges, double cophCorr, double timeMs);

private:
    Linkage m_linkage = AverageLink;
    int m_metric = 0;

    QVector<MergeEvent> m_merges;
    QVector<int> m_labels;
    int m_n = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute pairwise distance */
    static double distance(const QVector<double>& a,
                           const QVector<double>& b, int metric);

    /** @brief Compute condensed distance matrix (upper triangle) */
    static QVector<double> condensedDistances(
        const QVector<QVector<double>>& data, int metric);

    /** @brief Lance-Williams update formula */
    double lanceWilliamsUpdate(double d_iq, double d_jq,
                               int si, int sj, int sq) const;

    /** @brief Compute cophenetic distance matrix from merges */
    QVector<double> copheneticDistances() const;
};
