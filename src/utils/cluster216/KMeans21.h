/**
 * @file KMeans21.h
 * @brief 平衡K均值(最小费用流分配+Lloyd迭代重平衡) — Balanced K-Means with Min-Cost Flow Assignment and Lloyd Iteration with Rebalancing
 *
 * 功能: 实现带平衡约束的K均值聚类，采用最小费用流进行簇分配，
 *       Lloyd迭代过程中动态重平衡，确保各簇大小均匀。
 *
 * 协作: GaussianMixture19(高斯混合) / DBSCAN12(密度聚类) / KMeans20(核K均值)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 平衡K均值(最小费用流+Lloyd重平衡)
 */
class KMeans21 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        int numClusters = 0;
        int dimensions = 0;
        double inertia = 0.0;
        int iterations = 0;
        double balanceRatio = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit KMeans21(QObject *parent = nullptr);
    ~KMeans21() override;

    /** @brief Set parameters: cluster count, max iterations, balance tolerance */
    void setParameters(int clusters, int maxIter = 100,
                       double balanceTolerance = 0.1);

    /** @brief Fit balanced k-means to data */
    void fit(const QVector<QVector<double>>& data);

    /** @brief Predict cluster assignments for data */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    /** @brief Get cluster centroids */
    QVector<QVector<double>> centroids() const;

    /** @brief Compute total inertia (sum of squared distances) */
    double inertia(const QVector<QVector<double>>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fittingCompleted(int clusters, double inertia, double timeMs);

private:
    int m_clusters = 3;
    int m_maxIter = 100;
    double m_balanceTol = 0.1;
    int m_dim = 0;

    QVector<QVector<double>> m_centroids;
    QVector<int> m_labels;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize centroids via k-means++ */
    void initializeCentroids(const QVector<QVector<double>>& data);

    /** @brief Balanced assignment via min-cost flow */
    QVector<int> balancedAssign(const QVector<QVector<double>>& data) const;

    /** @brief Compute squared Euclidean distance */
    double squaredDist(const QVector<double>& a,
                       const QVector<double>& b) const;

    /** @brief Update centroids from assignments */
    void updateCentroids(const QVector<QVector<double>>& data,
                         const QVector<int>& labels);
};
