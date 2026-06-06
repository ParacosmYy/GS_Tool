/**
 * @file HierarchicalCluster7.h
 * @brief 层次聚类(Lance-Williams递推+共表距离矩阵) — Hierarchical Agglomerative Clustering with Lance-Williams Recurrence and Cophenetic Distance Matrix
 *
 * 功能: 实现层次凝聚聚类，支持Lance-Williams递推公式、多种链接策略、
 *       共表距离矩阵计算和树状图生成。
 *
 * 协作: KMedoids15(K-Medoids) / DBSCAN10(DBSCAN) / GaussianMixture13(高斯混合)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 层次聚类器(Lance-Williams+共表距离)
 */
class HierarchicalCluster7 : public QObject {
    Q_OBJECT

public:
    /** @brief Linkage strategy */
    enum class Linkage { Single, Complete, Average, Ward };

    /** @brief Merge event in dendrogram */
    struct MergeStep {
        int clusterA = 0;
        int clusterB = 0;
        double distance = 0.0;
        int newSize = 0;
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numPoints = 0;
        int numMerges = 0;
        double copheneticCorrelation = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit HierarchicalCluster7(QObject *parent = nullptr);
    ~HierarchicalCluster7() override;

    void setLinkage(Linkage linkage);
    void setNumClusters(int k);

    /** @brief 执行层次聚类，返回每个样本的簇标签 */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief 获取合并步骤序列(树状图) */
    QVector<MergeStep> dendrogram() const;

    /** @brief 计算共表距离矩阵 */
    QVector<QVector<double>> copheneticMatrix() const;

    /** @brief 计算共表相关系数 */
    double copheneticCorrelation(
        const QVector<QVector<double>>& origDist) const;

    /** @brief 距离矩阵计算 */
    QVector<QVector<double>> distanceMatrix(
        const QVector<QVector<double>>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numClusters, double copheneticCorr);

private:
    Linkage m_linkage = Linkage::Average;
    int m_numClusters = 2;

    QVector<MergeStep> m_merges;
    QVector<int> m_labels;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Lance-Williams coefficient computation */
    double lanceWilliams(double dij, double dik, double djk,
                         int si, int sj, int sk) const;

    /** @brief Euclidean distance between two vectors */
    double euclidean(const QVector<double>& a,
                     const QVector<double>& b) const;
};
