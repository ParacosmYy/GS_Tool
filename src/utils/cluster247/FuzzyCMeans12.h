/**
 * @file FuzzyCMeans12.h
 * @brief 模糊C均值聚类(可能性隶属度松弛+聚类有效性指标噪声鲁棒模糊划分) — Fuzzy C-Means with Possibilistic Membership Relaxation and Cluster Validity Index for Noise-Robust Fuzzy Partitioning
 *
 * 功能: 实现模糊C均值聚类算法(Fuzzy C-Means)，结合可能性隶属度松弛
 *       (possibilistic membership relaxation)抑制噪声影响，通过聚类有效性
 *       指标(cluster validity index)自动评估最优聚类数。
 *
 * 协作: BirchClustering12(BIRCH聚类) / KMeans25(K-means聚类) / GaussianMixture25(高斯混合模型)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 模糊C均值聚类(可能性松弛+有效性指标)
 */
class FuzzyCMeans12 : public QObject {
    Q_OBJECT

public:
    /** @brief Cluster validity indices */
    struct ValidityIndex {
        double partitionCoefficient = 0.0;
        double xieBeni = 0.0;
        double fukuyamaSugeno = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        int numDimensions = 0;
        int numClusters = 0;
        int numIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FuzzyCMeans12(QObject *parent = nullptr);
    ~FuzzyCMeans12() override;

    /** @brief Set number of clusters C */
    void setNumClusters(int c);

    /** @brief Set fuzzifier exponent m (>1.0) */
    void setFuzzifier(double m);

    /** @brief Set maximum iterations and convergence tolerance */
    void setMaxIterations(int maxIter, double tolerance);

    /** @brief Run clustering and return membership matrix (N x C) */
    QVector<QVector<double>> fit(const QVector<QVector<double>>& data);

    /** @brief Get cluster centers */
    QVector<QVector<double>> centers() const;

    /** @brief Compute cluster validity indices */
    ValidityIndex computeValidity() const;

    /** @brief Defuzzify: return hard labels from membership */
    QVector<int> hardLabels() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void clusteringCompleted(int numClusters, int iterations, double timeMs);

private:
    int m_numClusters = 3;
    double m_fuzzifier = 2.0;
    int m_maxIter = 200;
    double m_tolerance = 1e-5;

    QVector<QVector<double>> m_data;      // N x D
    QVector<QVector<double>> m_membership; // N x C
    QVector<QVector<double>> m_centers;    // C x D
    ValidityIndex m_validity;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize membership matrix randomly */
    void initMembership(int n);

    /** @brief Update cluster centers from membership */
    void updateCenters();

    /** @brief Update membership with possibilistic relaxation */
    void updateMembership();

    /** @brief Compute distance between point and center */
    double distance(const QVector<double>& a,
                    const QVector<double>& b) const;

    /** @brief Compute eta values for possibilistic term */
    QVector<double> computeEta() const;

    /** @brief Check convergence via membership shift */
    bool checkConvergence(const QVector<QVector<double>>& prev) const;
};
