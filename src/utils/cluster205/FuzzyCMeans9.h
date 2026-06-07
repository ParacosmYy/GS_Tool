/**
 * @file FuzzyCMeans9.h
 * @brief 模糊C均值聚类(Gustafson-Kessel距离+自适应簇体积约束) — Fuzzy C-Means with Gustafson-Kessel Distance and Adaptive Cluster Volume Constraint
 *
 * 功能: 实现模糊C均值(FCM)聚类算法，支持Gustafson-Kessel距离度量、
 *       自适应簇体积约束调整和模糊分区矩阵优化。
 *
 * 协作: BirchClustering9(BIRCH增量聚类) / KMeans19(K均值) / DBSCAN8(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 模糊C均值聚类(Gustafson-Kessel距离+自适应簇体积约束)
 */
class FuzzyCMeans9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numClusters = 0;
        int iterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FuzzyCMeans9(QObject *parent = nullptr);
    ~FuzzyCMeans9() override;

    void setClusterCount(int k);
    void setFuzziness(double m);
    void setMaxIterations(int maxIter);
    void setConvergenceThreshold(double eps);

    /** @brief Fit model to data with Gustafson-Kessel distance */
    void fit(const QVector<QVector<double>>& data);

    /** @brief Get fuzzy partition matrix (n x k membership values) */
    QVector<QVector<double>> getMembershipMatrix() const;

    /** @brief Get cluster centers */
    QVector<QVector<double>> getCenters() const;

    /** @brief Get hard cluster assignments */
    QVector<int> getLabels() const;

    /** @brief Compute adaptive volume constraint for each cluster */
    QVector<double> computeVolumeConstraints() const;

    /** @brief Update cluster covariance matrices */
    void updateCovariances();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, int iterations, double timeMs);

private:
    int m_k = 3;
    double m_fuzziness = 2.0;
    int m_maxIter = 200;
    double m_eps = 1e-5;
    int m_dim = 0;

    QVector<QVector<double>> m_data;
    QVector<QVector<double>> m_U;         // Membership matrix
    QVector<QVector<double>> m_centers;
    QVector<QVector<QVector<double>>> m_covariances;
    QVector<double> m_volumeConstraints;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize membership matrix randomly */
    void initMembership(int n);

    /** @brief Update cluster centers from membership */
    void updateCenters();

    /** @brief Gustafson-Kessel distance: (x-v)^T * F^{-1} * (x-v) */
    double gkDistance(const QVector<double>& x, const QVector<double>& v,
                      int clusterIdx) const;

    /** @brief Update membership matrix using GK distance */
    void updateMembership();

    /** @brief Compute 2x2 inverse (sufficient for most embedded signals) */
    static QVector<QVector<double>> invert2x2(
        const QVector<QVector<double>>& mat);

    /** @brief Trace of square matrix */
    static double trace(const QVector<QVector<double>>& mat);
};
