/**
 * @file FuzzyCMeans10.h
 * @brief 模糊C均值聚类(可能性隶属度+噪声簇自适应距离原型) — Fuzzy C-Means with Possibilistic Membership and Noise Cluster with Adaptive Distance Prototype
 *
 * 功能: 实现模糊C均值聚类算法，结合可能性隶属度消除噪声约束，
 *       噪声簇通过自适应距离原型处理离群点。
 *
 * 协作: BirchClustering10(BIRCH) / KMeans21(K均值) / DBSCAN12(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 模糊C均值聚类(可能性隶属度+噪声簇)
 */
class FuzzyCMeans10 : public QObject {
    Q_OBJECT

public:
    /** @brief Cluster result with membership grades */
    struct ClusterResult {
        QVector<QVector<double>> centroids;
        QVector<QVector<double>> membership;  // [n][c] membership matrix
        QVector<double> noiseWeight;           // noise cluster distances
        double objective = 0.0;
        int iterations = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numClusters = 0;
        int dim = 0;
        int totalIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FuzzyCMeans10(QObject *parent = nullptr);
    ~FuzzyCMeans10() override;

    /** @brief Set parameters: clusters, fuzziness m, noise delta, max iterations */
    void setParameters(int clusters = 3, double fuzziness = 2.0,
                       double noiseDelta = 1.0, int maxIter = 100);

    /** @brief Fit data and return cluster result */
    ClusterResult fit(const QVector<QVector<double>>& data);

    /** @brief Predict cluster memberships for new data */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    /** @brief Get current centroids */
    QVector<QVector<double>> centroids() const;

    /** @brief Compute possibilistic membership (relaxed constraint) */
    QVector<QVector<double>> computePossibilistic(
        const QVector<QVector<double>>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, int iterations, double timeMs);

private:
    int m_clusters = 3;
    double m_fuzziness = 2.0;
    double m_noiseDelta = 1.0;
    int m_maxIter = 100;
    int m_dim = 0;

    QVector<QVector<double>> m_centroids;
    QVector<QVector<double>> m_membership;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Euclidean distance between two vectors */
    double euclidean(const QVector<double>& a, const QVector<double>& b) const;

    /** @brief Initialize membership matrix randomly */
    void initMembership(int n);

    /** @brief Update centroids from membership matrix */
    void updateCentroids(const QVector<QVector<double>>& data);

    /** @brief Update membership with possibilistic and noise cluster */
    void updateMembership(const QVector<QVector<double>>& data);

    /** @brief Compute objective function value */
    double computeObjective(const QVector<QVector<double>>& data) const;

    /** @brief Adaptive noise distance based on cluster spread */
    double adaptiveNoiseDistance(const QVector<QVector<double>>& data) const;
};
