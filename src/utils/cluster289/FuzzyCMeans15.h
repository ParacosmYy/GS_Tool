/**
 * @file FuzzyCMeans15.h
 * @brief 模糊C均值聚类(可能性隶属度与簇有效性指数引导的自适应指数调整) — Fuzzy C-means with Possibilistic Membership and Cluster Validity Index Guided Automatic Exponent Tuning
 *
 * 功能: 实现模糊C均值聚类(Fuzzy C-means)，采用可能性隶属度(possibilistic membership)
 *       与簇有效性指数(cluster validity index)引导自适应指数调整(automatic exponent tuning)。
 *
 * 协作: BirchClustering15(BIRCH聚类) / KMeans31(K-means) / DBSCAN17(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 模糊C均值聚类(可能性隶属度与簇有效性指数引导的自适应指数调整)
 */
class FuzzyCMeans15 : public QObject {
    Q_OBJECT

public:
    /** @brief Clustering result with fuzzy memberships */
    struct ClusterResult {
        QVector<int> labels;               // Hard assignment per point
        QVector<QVector<double>> membership; // Fuzzy membership matrix [n×k]
        QVector<QVector<double>> centroids; // Cluster centers [k×d]
        int numClusters = 0;
        double objectiveValue = 0.0;        // Final Jm objective
    };

    /** @brief Cluster validity indices */
    struct ValidityIndex {
        double partitionCoefficient = 0.0;  // PC index
        double xieBeni = 0.0;               // Xie-Beni index
        double optimalM = 0.0;              // Auto-tuned exponent
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numClusters = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FuzzyCMeans15(QObject *parent = nullptr);
    ~FuzzyCMeans15() override;

    void setNumClusters(int k);
    void setFuzziness(double m);
    void setMaxIterations(int maxIter);
    void setConvergenceThreshold(double eps);
    void setAutoTuneM(bool enabled);

    /** @brief Run FCM clustering on data matrix [n×d] */
    ClusterResult fit(const QVector<QVector<double>>& data);

    /** @brief Evaluate cluster validity for given result */
    ValidityIndex evaluateValidity(const QVector<QVector<double>>& data,
                                    const ClusterResult& result) const;

    /** @brief Get possibilistic membership (typicality) from fuzzy membership */
    QVector<QVector<double>> computePossibilisticMembership(
        const QVector<QVector<double>>& data,
        const QVector<QVector<double>>& centroids) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitDone(int n, int k, double objVal, double timeMs);

private:
    int m_k = 3;
    double m_m = 2.0;                   // Fuzziness exponent
    int m_maxIter = 200;
    double m_eps = 1e-5;
    bool m_autoTuneM = false;
    int m_dims = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize membership matrix randomly */
    QVector<QVector<double>> initMembership(int n) const;

    /** @brief Update centroids from membership matrix */
    QVector<QVector<double>> updateCentroids(
        const QVector<QVector<double>>& data,
        const QVector<QVector<double>>& U) const;

    /** @brief Update membership matrix from centroids */
    QVector<QVector<double>> updateMembership(
        const QVector<QVector<double>>& data,
        const QVector<QVector<double>>& centroids) const;

    /** @brief Compute Jm objective function value */
    double computeObjective(const QVector<QVector<double>>& data,
                             const QVector<QVector<double>>& U,
                             const QVector<QVector<double>>& centroids) const;

    /** @brief Compute squared Euclidean distance */
    double squaredDist(const QVector<double>& a,
                        const QVector<double>& b) const;

    /** @brief Auto-tune fuzziness exponent using validity index */
    double autoTuneExponent(const QVector<QVector<double>>& data,
                             int k) const;
};
