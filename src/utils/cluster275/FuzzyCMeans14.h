/**
 * @file FuzzyCMeans14.h
 * @brief 模糊C均值聚类(熵正则化与可能性隶属度的噪声鲁棒模糊划分) — Fuzzy C-means with Entropy Regularization and Possibilistic Membership for Noise-robust Fuzzy Partitioning
 *
 * 功能: 实现模糊C均值聚类(Fuzzy C-means)，采用熵正则化(entropy regularization)
 *       与可能性隶属度(possibilistic membership)实现噪声鲁棒模糊划分(noise-robust fuzzy partitioning)。
 *
 * 协作: BirchClustering14(BIRCH聚类) / KMeans29(K均值) / DBSCAN16(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 模糊C均值聚类(熵正则化与可能性隶属度)
 */
class FuzzyCMeans14 : public QObject {
    Q_OBJECT

public:
    /** @brief Clustering result */
    struct ClusterResult {
        QVector<QVector<double>> centroids;   // k centroids
        QVector<QVector<double>> membership;   // n x k fuzzy membership matrix
        QVector<int> labels;                    // Hard labels (max membership)
        double objectiveValue = 0.0;
        int iterations = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numClusters = 0;
        int numIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FuzzyCMeans14(QObject *parent = nullptr);
    ~FuzzyCMeans14() override;

    /** @brief Set fuzziness exponent m (>1) */
    void setFuzziness(double m);

    /** @brief Set entropy regularization weight */
    void setEntropyWeight(double alpha);

    /** @brief Set possibilistic membership scale */
    void setPossibilisticScale(double eta);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Set max iterations */
    void setMaxIterations(int iter);

    /** @brief Run fuzzy c-means clustering */
    ClusterResult fit(const QVector<QVector<double>>& data, int k);

    /** @brief Predict membership for new points */
    QVector<QVector<double>> predict(const QVector<QVector<double>>& points,
                                      const QVector<QVector<double>>& centroids) const;

    /** @brief Compute partition coefficient (validity index) */
    double partitionCoefficient(const QVector<QVector<double>>& membership) const;

    /** @brief Compute Xie-Beni validity index */
    double xieBeniIndex(const QVector<QVector<double>>& data,
                         const QVector<QVector<double>>& centroids,
                         const QVector<QVector<double>>& membership) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringDone(int k, int iterations, double objective, double timeMs);
    void iterationUpdate(int iter, double objective);

private:
    double m_fuzziness = 2.0;
    double m_entropyWeight = 0.5;
    double m_possScale = 1.0;
    double m_tolerance = 1e-5;
    int m_maxIter = 200;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize membership matrix randomly */
    QVector<QVector<double>> initMembership(int n, int k) const;

    /** @brief Update centroids from membership and data */
    QVector<QVector<double>> updateCentroids(const QVector<QVector<double>>& data,
                                              const QVector<QVector<double>>& U) const;

    /** @brief Update membership with entropy regularization + possibilistic term */
    QVector<QVector<double>> updateMembership(const QVector<QVector<double>>& data,
                                               const QVector<QVector<double>>& centroids) const;

    /** @brief Compute objective function value */
    double computeObjective(const QVector<QVector<double>>& data,
                             const QVector<QVector<double>>& centroids,
                             const QVector<QVector<double>>& U) const;

    /** @brief Euclidean distance squared between two vectors */
    double distSq(const QVector<double>& a, const QVector<double>& b) const;
};
