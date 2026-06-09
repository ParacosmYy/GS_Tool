/**
 * @file Agglomerative12.h
 * @brief 层次凝聚聚类(质心链接+树状图质量共表相关系数验证) — Agglomerative Clustering with Centroid Linkage and Cophenetic Correlation Coefficient for Dendrogram Quality Validation
 *
 * 功能: 实现层次凝聚聚类(Agglomerative Clustering)，使用质心链接(centroid
 *       linkage)策略递归合并最近簇对，通过共表相关系数(cophenetic correlation
 *       coefficient)评估树状图(dendrogram)对原始距离矩阵的保真度。
 *
 * 协作: KMeans25(K-means聚类) / DBSCAN14(密度聚类) / GaussianMixture25(高斯混合模型)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 层次凝聚聚类(质心链接+共表相关系数验证)
 */
class Agglomerative12 : public QObject {
    Q_OBJECT

public:
    /** @brief Merge event in the dendrogram */
    struct MergeStep {
        int clusterA = -1;
        int clusterB = -1;
        double mergeDistance = 0.0;
        int newSize = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        int numDimensions = 0;
        int numMerges = 0;
        double copheneticCorrelation = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Agglomerative12(QObject *parent = nullptr);
    ~Agglomerative12() override;

    /** @brief Set target number of clusters (cuts dendrogram) */
    void setNumClusters(int k);

    /** @brief Set linkage: only centroid supported */
    void setLinkageCentroid();

    /** @brief Fit and return cluster labels */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Get full dendrogram merge sequence */
    QVector<MergeStep> dendrogram() const;

    /** @brief Compute cophenetic correlation coefficient */
    double copheneticCorrelation() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numClusters, double cophenetic, double timeMs);

private:
    int m_targetK = 2;

    QVector<MergeStep> m_merges;
    QVector<QVector<double>> m_distanceMatrix;
    QVector<int> m_labels;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute pairwise Euclidean distance matrix */
    void buildDistanceMatrix(const QVector<QVector<double>>& data);

    /** @brief Compute centroid of a set of indices */
    QVector<double> computeCentroid(const QVector<QVector<double>>& data,
                                     const QVector<int>& indices) const;

    /** @brief Compute cophenetic distance matrix from merge history */
    QVector<QVector<double>> copheneticMatrix(int n) const;

    /** @brief Pearson correlation between condensed distance matrices */
    double pearsonCorrelation(const QVector<QVector<double>>& a,
                               const QVector<QVector<double>>& b) const;
};
