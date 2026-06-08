/**
 * @file FuzzyCMeans11.h
 * @brief 模糊C均值聚类(Gustafson-Kessel距离+自适应簇体积约束矩阵估计) — Fuzzy C-Means with Gustafson-Kessel Distance and Adaptive Cluster Volume Constraint
 *
 * 功能: 实现模糊C均值聚类(Fuzzy C-Means, FCM)，采用Gustafson-Kessel(GK)距离度量
 *       替代标准欧氏距离，结合自适应簇体积约束矩阵(volume constraint matrix)估计，
 *       支持非超球形簇的检测与椭球型数据分布的聚类。
 *
 * 协作: BirchClustering11(BIRCH聚类) / KMeans23(K均值) / DBSCAN13(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 模糊C均值聚类(Gustafson-Kessel距离+自适应簇体积约束矩阵估计)
 */
class FuzzyCMeans11 : public QObject {
    Q_OBJECT

public:
    /** @brief Cluster result for each data point */
    struct Membership {
        int bestCluster = -1;
        double bestMembership = 0.0;
        QVector<double> degrees;  // membership degree per cluster
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numDimensions = 0;
        int numClusters = 0;
        int iterationsUsed = 0;
        double finalObjective = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FuzzyCMeans11(QObject *parent = nullptr);
    ~FuzzyCMeans11() override;

    /** @brief Set number of clusters c */
    void setNumClusters(int c);

    /** @brief Set fuzziness exponent m (> 1.0) */
    void setFuzziness(double m);

    /** @brief Set max iterations */
    void setMaxIterations(int iter);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Set volume constraint rho for GK distance */
    void setVolumeConstraint(double rho);

    /** @brief Run FCM-GK clustering on [n x d] data */
    bool fit(const QVector<QVector<double>>& data);

    /** @brief Predict membership for a new point */
    Membership predict(const QVector<double>& point) const;

    /** @brief Get cluster centroids */
    QVector<QVector<double>> centroids() const;

    /** @brief Get membership matrix [n x c] */
    QVector<Membership> memberships() const;

    /** @brief Get cluster covariance matrices */
    QVector<QVector<QVector<double>>> covariances() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationCompleted(int iter, double objective);
    void fitCompleted(int clusters, int iterations, double timeMs);

private:
    int m_numClusters = 3;
    double m_fuzziness = 2.0;
    int m_maxIterations = 200;
    double m_tolerance = 1e-6;
    double m_volumeConstraint = 1.0;

    QVector<QVector<double>> m_data;
    QVector<QVector<double>> m_centroids;
    QVector<Membership> m_memberships;
    QVector<QVector<QVector<double>>> m_covariances;  // [c x d x d]

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize membership matrix randomly */
    void initMembership(int n);

    /** @brief Compute Gustafson-Kessel distance from point to cluster i */
    double gkDistance(const QVector<double>& point, int clusterIdx) const;

    /** @brief Compute cluster covariance (fuzzy) for cluster i */
    QVector<QVector<double>> fuzzyCovariance(int clusterIdx) const;

    /** @brief Update centroids from membership matrix */
    void updateCentroids();

    /** @brief Update membership matrix from GK distances */
    void updateMembership();

    /** @brief Compute objective function J_m */
    double computeObjective() const;

    /** @brief Matrix determinant (d x d) */
    double determinant(const QVector<QVector<double>>& mat) const;

    /** @brief Invert d x d matrix via Gauss-Jordan */
    QVector<QVector<double>> invertMatrix(const QVector<QVector<double>>& mat) const;
};
