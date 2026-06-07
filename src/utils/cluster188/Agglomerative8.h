/**
 * @file Agglomerative8.h
 * @brief 层次凝聚聚类(全连接+Genolini共识合并准则) — Agglomerative Clustering with Complete-Linkage and Genolini-Consensus Cluster Merging Criterion
 *
 * 功能: 实现层次凝聚聚类算法，支持全连接(complete-linkage)距离度量、
 *       Genolini共识合并准则自适应阈值、树状图(dendrogram)输出和多维度数据。
 *
 * 协作: KMeans17(K均值) / DBSCAN10(密度聚类) / MeanShift9(均值漂移)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 层次凝聚聚类器(全连接+Genolini共识合并)
 */
class Agglomerative8 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numPoints = 0;
        int numClusters = 0;
        int mergeSteps = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Dendrogram merge record */
    struct MergeRecord {
        int clusterA = -1;
        int clusterB = -1;
        double distance = 0.0;
        int newSize = 0;
    };

    explicit Agglomerative8(QObject *parent = nullptr);
    ~Agglomerative8() override;

    void setTargetClusters(int k);
    void setDistanceThreshold(double t);
    void setConsensusAlpha(double alpha);
    void setMaxIterations(int iter);

    /** @brief Execute agglomerative clustering, return per-point labels */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Get dendrogram merge history */
    QVector<MergeRecord> dendrogram() const { return m_merges; }

    /** @brief Get cluster centers (mean of each cluster) */
    QVector<QVector<double>> centers() const { return m_centers; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, int merges, double timeMs);

private:
    int m_targetClusters = 2;
    double m_distThreshold = 0.0;
    double m_consensusAlpha = 0.5;
    int m_maxIterations = 1000;

    QVector<MergeRecord> m_merges;
    QVector<QVector<double>> m_centers;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Complete-linkage distance between two clusters */
    double completeLinkage(const QVector<int>& c1, const QVector<int>& c2,
                           const QVector<QVector<double>>& dist) const;

    /** @brief Genolini consensus criterion for merge decision */
    bool consensusCriterion(double linkDist, double avgIntraDist1,
                            double avgIntraDist2) const;

    /** @brief Compute pairwise distance matrix */
    QVector<QVector<double>> computeDistMatrix(
        const QVector<QVector<double>>& data) const;

    /** @brief Compute mean intra-cluster distance */
    double avgIntraDistance(const QVector<int>& cluster,
                           const QVector<QVector<double>>& dist) const;
};
