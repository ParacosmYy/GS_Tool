/**
 * @file DBSCAN14.h
 * @brief DBSCAN密度聚类(k距离肘部估计+变epsilon密度自适应邻域) — DBSCAN Clustering with k-Distance Elbow Estimation and Variable Epsilon Per Cluster for Density-Adaptive Neighborhood
 *
 * 功能: 实现DBSCAN密度聚类算法，利用k距离肘部法(k-distance elbow)自动估计
 *       全局epsilon参数，并支持变epsilon(variable epsilon)机制使每个簇可以
 *       具有不同的邻域半径以适应密度变化(density-adaptive neighborhood)。
 *
 * 协作: KMeans24(K-means) / OPTICS10(排序聚类) / GaussianMixture24(高斯混合)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief DBSCAN密度聚类(k距离肘部估计+变epsilon密度自适应邻域)
 */
class DBSCAN14 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numClusters = 0;
        int numCorePoints = 0;
        int numBorderPoints = 0;
        int numNoisePoints = 0;
        int numSamples = 0;
        int numDimensions = 0;
        double estimatedEpsilon = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DBSCAN14(QObject *parent = nullptr);
    ~DBSCAN14() override;

    /** @brief Set minimum points to form a core point */
    void setMinPoints(int minPts);

    /** @brief Set base epsilon (neighborhood radius); 0 = auto-estimate */
    void setEpsilon(double eps);

    /** @brief Set k for k-distance elbow estimation (default = minPts) */
    void setElbowK(int k);

    /** @brief Enable variable epsilon per cluster for density adaptation */
    void setVariableEpsilon(bool enabled);

    /** @brief Run clustering, returns labels (-1 = noise) */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Get cluster labels */
    QVector<int> labels() const;

    /** @brief Get per-cluster epsilon values (empty if fixed epsilon) */
    QVector<double> clusterEpsilons() const;

    /** @brief Estimate epsilon via k-distance elbow method */
    double estimateEpsilon(const QVector<QVector<double>>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numClusters, int numNoise, double timeMs);

private:
    int m_minPts = 5;
    double m_eps = 0.0;
    int m_elbowK = 5;
    bool m_variableEps = false;

    QVector<QVector<double>> m_data;
    QVector<int> m_labels;
    QVector<double> m_clusterEps;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Squared Euclidean distance */
    double sqDist(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Find neighbors within radius eps for point idx */
    QVector<int> rangeQuery(int idx, double eps) const;

    /** @brief Compute k-distance for a single point */
    double kDistance(int idx, int k) const;

    /** @brief Compute local density-adaptive epsilon for a region */
    double localEpsilon(const QVector<int>& region) const;
};
