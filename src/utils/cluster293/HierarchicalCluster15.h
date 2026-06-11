/**
 * @file HierarchicalCluster15.h
 * @brief 层次聚类(质心链接与共表型相关系数验证树状图质量) — Hierarchical Clustering with Centroid Linkage and Cophenetic Correlation Coefficient for Dendrogram Quality Validation
 *
 * 功能: 实现层次聚类(hierarchical clustering)，采用质心链接(centroid linkage)
 *       与共表型相关系数(cophenetic correlation coefficient)验证树状图质量(dendrogram quality validation)。
 *
 * 协作: KMedoids23(K-中心点聚类) / GaussianMixture35(高斯混合模型) / BirchClustering15(BIRCH聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

class HierarchicalCluster15 : public QObject {
    Q_OBJECT

public:
    /** @brief Single merge step in the dendrogram */
    struct MergeStep {
        int clusterA = -1;
        int clusterB = -1;
        double distance = 0.0;
        int newSize = 0;
    };

    /** @brief Full clustering result */
    struct ClusterResult {
        QVector<MergeStep> merges;
        QVector<int> assignments;
        double copheneticCorrelation = 0.0;
        int numClusters = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalFits = 0;
        int numPoints = 0;
        double avgCophenetic = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HierarchicalCluster15(QObject *parent = nullptr);
    ~HierarchicalCluster15() override;

    void setNumClusters(int k);
    void setLinkageThreshold(double threshold);

    /** @brief Fit hierarchical clustering on data matrix */
    ClusterResult fit(const QVector<QVector<double>>& data);

    /** @brief Compute pairwise distance matrix (Euclidean) */
    QVector<QVector<double>> computeDistanceMatrix(
        const QVector<QVector<double>>& data) const;

    /** @brief Compute cophenetic correlation coefficient */
    double copheneticCorrelation(const QVector<QVector<double>>& distMatrix,
                                  const QVector<MergeStep>& merges) const;

    /** @brief Cut dendrogram at given number of clusters */
    QVector<int> cutDendrogram(const QVector<MergeStep>& merges,
                                int n, int k) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitDone(int n, int k, double cophenetic, double timeMs);

private:
    int m_k = 2;
    double m_threshold = 0.0;
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_cophSum = 0.0;

    /** @brief Centroid of a set of points */
    QVector<double> computeCentroid(const QVector<QVector<double>>& data,
                                     const QVector<int>& indices) const;

    /** @brief Euclidean distance between two vectors */
    double euclideanDistance(const QVector<double>& a,
                             const QVector<double>& b) const;
};
