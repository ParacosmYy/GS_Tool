/**
 * @file FuzzyCMeans8.h
 * @brief 模糊C均值聚类(可能性隶属度PCM+收敛加速交替优化) — Fuzzy C-Means with Possibilistic Membership (PCM) and Convergence-Accelerated Alternating Optimization
 *
 * 功能: 实现模糊C均值聚类算法，支持可能性隶属度(PCM)模型、
 *       收敛加速的交替优化策略和可配置模糊指数/终止阈值。
 *
 * 协作: BirchClustering7(BIRCH) / KMeans17(K均值) / DBSCAN10(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 模糊C均值聚类器(PCM+加速收敛)
 */
class FuzzyCMeans8 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalRuns = 0;
        int numPoints = 0;
        int numClusters = 0;
        int iterations = 0;
        double finalObjective = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FuzzyCMeans8(QObject *parent = nullptr);
    ~FuzzyCMeans8() override;

    void setFuzziness(double m);
    void setMaxIterations(int iter);
    void setEpsilon(double eps);
    void setPcmWeight(double eta);

    /** @brief Fit model to data, return per-point soft membership matrix */
    QVector<QVector<double>> fit(const QVector<QVector<double>>& data,
                                  int k);

    /** @brief Get hard cluster labels from membership */
    QVector<int> hardLabels(const QVector<QVector<double>>& membership) const;

    /** @brief Get cluster centroids */
    QVector<QVector<double>> centroids() const;

    /** @brief Compute PCM typicality matrix for given data */
    QVector<QVector<double>> computeTypicality(
        const QVector<QVector<double>>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int clusters, int iterations, double objective);

private:
    double m_fuzziness = 2.0;
    int m_maxIter = 200;
    double m_epsilon = 1e-5;
    double m_pcmWeight = 1.0;

    QVector<QVector<double>> m_centroids;
    QVector<QVector<double>> m_membership;
    QVector<double> m_eta;       // PCM bandwidth per cluster
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize membership matrix randomly */
    void initMembership(int n, int k);

    /** @brief Update centroids from weighted membership */
    void updateCentroids(const QVector<QVector<double>>& data);

    /** @brief Update membership with convergence acceleration */
    void updateMembership(const QVector<QVector<double>>& data);

    /** @brief Compute PCM typicality for one point/cluster */
    double typicality(int pointIdx, int cluster,
                      const QVector<QVector<double>>& data) const;

    /** @brief Compute objective function J */
    double objective(const QVector<QVector<double>>& data) const;

    /** @brief Squared Euclidean distance */
    double distSq(const QVector<double>& a,
                  const QVector<double>& b) const;
};
