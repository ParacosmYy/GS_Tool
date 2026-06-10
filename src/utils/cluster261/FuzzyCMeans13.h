/**
 * @file FuzzyCMeans13.h
 * @brief 模糊C均值聚类(Gustafson-Kessel距离协方差矩阵椭球簇自适应形状) — Fuzzy C-means with Gustafson-Kessel Distance and Adaptive Cluster Shape via Covariance Matrix for Ellipsoidal Clusters
 *
 * 功能: 实现模糊C均值聚类(Fuzzy C-means)，采用Gustafson-Kessel距离(GK distance)
 *       和协方差矩阵(covariance matrix)实现椭球簇自适应形状(ellipsoidal cluster shape)。
 *
 * 协作: BirchClustering13(BIRCH聚类) / KMeans27(K均值) / DBSCAN15(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 模糊C均值聚类(Gustafson-Kessel距离协方差矩阵椭球簇自适应形状)
 */
class FuzzyCMeans13 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numDimensions = 0;
        int numClusters = 0;
        int numIterations = 0;
        double finalObjective = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FuzzyCMeans13(QObject *parent = nullptr);
    ~FuzzyCMeans13() override;

    /** @brief Set number of clusters C, fuzziness m, and termination epsilon */
    void setParameters(int numClusters, double fuzziness = 2.0, double epsilon = 1e-5,
                       int maxIterations = 100);

    /** @brief Run clustering on data, returns true on convergence */
    bool fit(const QVector<QVector<double>>& data);

    /** @brief Get cluster centers (C x D) */
    QVector<QVector<double>> centers() const;

    /** @brief Get fuzzy membership matrix (N x C) */
    QVector<QVector<double>> membershipMatrix() const;

    /** @brief Get hard labels (argmax membership per point) */
    QVector<int> labels() const;

    /** @brief Get covariance matrices per cluster (C x D x D flattened) */
    QVector<QVector<QVector<double>>> covariances() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringUpdated(int numClusters, int iterations, double objective, double timeMs);

private:
    int m_C = 3;
    double m_m = 2.0;
    double m_eps = 1e-5;
    int m_maxIter = 100;
    int m_dims = 0;

    QVector<QVector<double>> m_data;
    QVector<QVector<double>> m_U;       // membership matrix N x C
    QVector<QVector<double>> m_centers;  // C x D
    QVector<QVector<QVector<double>>> m_cov; // C x D x D

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize membership matrix randomly */
    void initMembership(int n);

    /** @brief Update cluster centers from membership */
    void updateCenters();

    /** @brief Compute covariance matrices per cluster */
    void updateCovariances();

    /** @brief Compute Gustafson-Kessel distance from point to cluster */
    double gkDistance(const QVector<double>& point, int cluster) const;

    /** @brief Update membership matrix, returns max change */
    double updateMembership();

    /** @brief Compute objective function J_m */
    double computeObjective() const;
};
