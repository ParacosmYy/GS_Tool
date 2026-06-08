/**
 * @file HierarchicalCluster10.h
 * @brief 层次聚类(UPGMA超度量树+共表距离矩阵树状图验证) — Hierarchical Clustering with UPGMA Ultrametric Tree and Cophenetic Distance Matrix for Dendrogram Validation
 *
 * 功能: 实现UPGMA层次聚类算法，构建超度量树，
 *       计算共表距离矩阵(cophenetic matrix)验证树状图质量。
 *
 * 协作: KMedoids18(K中心点) / GaussianMixture20(高斯混合) / DBSCAN6(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 层次聚类(UPGMA+共表距离验证)
 */
class HierarchicalCluster10 : public QObject {
    Q_OBJECT

public:
    /** @brief Cluster merge record */
    struct MergeRecord {
        int clusterA = 0;
        int clusterB = 0;
        double distance = 0.0;
        int newSize = 0;
    };

    /** @brief Clustering result */
    struct ClusterResult {
        QVector<MergeRecord> merges;
        QVector<QVector<int>> clusters;
        double copheneticCorrelation = 0.0;
        int numClusters = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int dim = 0;
        int targetClusters = 0;
        double copheneticCorrelation = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HierarchicalCluster10(QObject *parent = nullptr);
    ~HierarchicalCluster10() override;

    /** @brief Set target number of clusters */
    void setParameters(int targetClusters = 2);

    /** @brief Perform hierarchical clustering */
    ClusterResult fit(const QVector<QVector<double>>& data);

    /** @brief Compute cophenetic distance matrix from merge history */
    QVector<QVector<double>> copheneticMatrix(const QVector<MergeRecord>& merges,
                                               int numPoints) const;

    /** @brief Compute cophenetic correlation coefficient */
    double copheneticCorrelation(const QVector<QVector<double>>& originalDist,
                                  const QVector<QVector<double>>& cophDist) const;

    /** @brief Get cluster assignments at given cut distance */
    QVector<int> cutAtDistance(const QVector<MergeRecord>& merges,
                                int numPoints, double threshold) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, double cophenetic, double timeMs);

private:
    int m_targetClusters = 2;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute pairwise distance matrix */
    QVector<QVector<double>> computeDistanceMatrix(
        const QVector<QVector<double>>& data) const;

    /** @brief UPGMA distance update between merged cluster and others */
    double upgmaDistance(int sizeA, int sizeB, int sizeC,
                          double dAC, double dBC) const;

    /** @brief Euclidean distance between two points */
    double euclidean(const QVector<double>& a, const QVector<double>& b) const;
};
