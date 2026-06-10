/**
 * @file HierarchicalCluster14.h
 * @brief 层次聚类(非加权配对组法与共表距离矩阵的树状图验证) — Hierarchical Clustering with Unweighted Pair Group Method and Cophenetic Distance Matrix for Dendrogram Validation
 *
 * 功能: 实现层次聚类(hierarchical clustering)，采用非加权配对组法(UPGMA)
 *       与共表距离矩阵(cophenetic distance matrix)实现树状图验证(dendrogram validation)。
 *
 * 协作: KMedoids22(K中心点) / GaussianMixture32(高斯混合) / DBSCAN16(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 层次聚类(非加权配对组法与共表距离矩阵)
 */
class HierarchicalCluster14 : public QObject {
    Q_OBJECT

public:
    /** @brief Merge event in dendrogram */
    struct MergeStep {
        int clusterA = -1;
        int clusterB = -1;
        double distance = 0.0;
        int newSize = 0;
    };

    /** @brief Full clustering result */
    struct ClusterResult {
        QVector<MergeStep> merges;
        QVector<QVector<int>> clusters;
        double copheneticCorrelation = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numClusters = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HierarchicalCluster14(QObject *parent = nullptr);
    ~HierarchicalCluster14() override;

    /** @brief Set target number of clusters */
    void setNumClusters(int k);

    /** @brief Perform agglomerative clustering with UPGMA linkage */
    ClusterResult fit(const QVector<QVector<double>>& data);

    /** @brief Compute cophenetic distance matrix from dendrogram */
    QVector<QVector<double>> copheneticMatrix(const QVector<MergeStep>& merges, int n) const;

    /** @brief Compute cophenetic correlation coefficient */
    double copheneticCorrelation(const QVector<QVector<double>>& originalDist,
                                  const QVector<QVector<double>>& cophDist) const;

    /** @brief Cut dendrogram at distance threshold */
    QVector<int> cutAtDistance(const QVector<MergeStep>& merges, int n, double threshold) const;

    /** @brief Get cluster labels for k clusters */
    QVector<int> getLabels(const QVector<MergeStep>& merges, int n, int k) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void mergeDone(int step, int clusterA, int clusterB, double dist);
    void fittingDone(int n, int k, double cophCorr, double timeMs);

private:
    int m_k = 2;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build pairwise distance matrix */
    QVector<QVector<double>> buildDistMatrix(const QVector<QVector<double>>& data) const;

    /** @brief Euclidean distance between two points */
    double euclidean(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief UPGMA linkage distance between two clusters */
    double upgmaDistance(const QVector<QVector<double>>& dist,
                         const QVector<int>& ci, const QVector<int>& cj) const;
};
