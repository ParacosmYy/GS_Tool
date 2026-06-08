/**
 * @file Agglomerative11.h
 * @brief 层次凝聚聚类(WPGMA加权配对平均+轮廓系数最优切割高度) — Agglomerative Clustering with WPGMA Weighted Pair-Group Average and Silhouette Index Validation for Optimal Cut Height
 *
 * 功能: 实现层次凝聚聚类(agglomerative clustering)，采用WPGMA加权配对平均
 *       (weighted pair-group method with arithmetic mean)合并策略，并利用轮廓系数
 *       (Silhouette index)评估不同切割高度以确定最优聚类数。
 *
 * 协作: KMeans23(K均值) / GaussianMixture22(高斯混合) / DBSCAN13(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 层次凝聚聚类(WPGMA加权配对平均+轮廓系数最优切割高度)
 */
class Agglomerative11 : public QObject {
    Q_OBJECT

public:
    /** @brief Cluster assignment for a data point */
    struct Assignment {
        int clusterId = -1;
        double silhouette = 0.0;
    };

    /** @brief Dendrogram merge step */
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
        int optimalClusters = 0;
        double bestSilhouette = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Agglomerative11(QObject *parent = nullptr);
    ~Agglomerative11() override;

    /** @brief Set linkage variant: 0=WPGMA (default) */
    void setLinkage(int mode);

    /** @brief Fit model on data matrix [n x d], auto-detect optimal cut */
    bool fit(const QVector<QVector<double>>& data, int maxClusters = 0);

    /** @brief Get cluster assignments at optimal cut height */
    QVector<Assignment> assignments() const;

    /** @brief Get full dendrogram merge history */
    QVector<MergeStep> dendrogram() const;

    /** @brief Get assignments at a specific number of clusters */
    QVector<int> cutAtK(int k) const;

    /** @brief Get silhouette scores for all points */
    QVector<double> silhouetteScores() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void mergeCompleted(int step, double distance);
    void fitCompleted(int clusters, double silhouette, double timeMs);

private:
    int m_linkage = 0;

    QVector<QVector<double>> m_data;
    QVector<Assignment> m_assignments;
    QVector<MergeStep> m_merges;
    QVector<int> m_clusterMap;    // point -> active cluster id

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute WPGMA distance between two clusters */
    double wpgmaDistance(int ci, int cj, const QVector<double>& distMatrix,
                         int n, const QVector<int>& sizes) const;

    /** @brief Compute pairwise distance matrix */
    QVector<double> computeDistanceMatrix(const QVector<QVector<double>>& data) const;

    /** @brief Compute Silhouette index for given partition */
    double computeSilhouette(const QVector<int>& labels) const;

    /** @brief Find optimal cut height via silhouette maximization */
    int findOptimalCut() const;

    /** @brief Squared Euclidean distance */
    double squaredDist(const QVector<double>& a, const QVector<double>& b) const;
};
