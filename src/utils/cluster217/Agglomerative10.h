/**
 * @file Agglomerative10.h
 * @brief 层次凝聚聚类(Ward最小方差+动态树切割) — Agglomerative Clustering with Ward's Minimum Variance and Dynamic Tree Cutting via Dendrogram Height Threshold
 *
 * 功能: 实现基于Ward最小方差准则的凝聚层次聚类，构建树状图，
 *       通过高度阈值动态切割获取聚类结果。
 *
 * 协作: KMeans21(平衡K均值) / GaussianMixture19(高斯混合) / DBSCAN12(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 层次凝聚聚类(Ward最小方差+动态树切割)
 */
class Agglomerative10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        int numClusters = 0;
        int dimensions = 0;
        int mergeSteps = 0;
        double cutHeight = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Dendrogram merge record */
    struct MergeRecord {
        int clusterA = 0;
        int clusterB = 0;
        double distance = 0.0;
        int newSize = 0;
    };

    explicit Agglomerative10(QObject *parent = nullptr);
    ~Agglomerative10() override;

    /** @brief Set parameters: target clusters (0 = auto via height), cut height threshold */
    void setParameters(int targetClusters = 0, double heightThreshold = 0.0);

    /** @brief Fit agglomerative clustering to data */
    void fit(const QVector<QVector<double>>& data);

    /** @brief Get cluster labels */
    QVector<int> labels() const;

    /** @brief Get dendrogram merge history */
    QVector<MergeRecord> dendrogram() const;

    /** @brief Compute Ward distance between two point sets */
    double wardDistance(const QVector<QVector<double>>& a,
                       const QVector<QVector<double>>& b) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, double cutHeight, double timeMs);

private:
    int m_targetClusters = 0;
    double m_heightThreshold = 0.0;
    int m_dim = 0;

    QVector<int> m_labels;
    QVector<MergeRecord> m_merges;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Squared Euclidean distance */
    double squaredDist(const QVector<double>& a,
                       const QVector<double>& b) const;

    /** @brief Compute centroid of point set */
    QVector<double> centroid(const QVector<QVector<double>>& pts) const;

    /** @brief Compute Ward linkage distance */
    double wardLinkage(int nA, const QVector<double>& cA,
                       int nB, const QVector<double>& cB) const;
};
