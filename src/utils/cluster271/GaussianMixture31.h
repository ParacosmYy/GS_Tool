/**
 * @file GaussianMixture31.h
 * @brief 高斯混合模型(期望条件最大化与充分统计量缓存增量更新) — Gaussian Mixture with Expectation-Conditional-Maximization and Sufficient Statistics Caching for Incremental Updates
 *
 * 功能: 实现高斯混合模型(Gaussian mixture model)，采用期望条件最大化(expectation-conditional-maximization)
 *       与充分统计量缓存(sufficient statistics caching)实现增量更新(incremental updates)。
 *
 * 协作: DBSCAN16(密度聚类) / KMeans28(K均值) / OPTICS12(排序聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型(期望条件最大化与充分统计量缓存增量更新)
 */
class GaussianMixture31 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numComponents = 0;
        int numSamples = 0;
        int numIterations = 0;
        double logLikelihood = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussianMixture31(QObject *parent = nullptr);
    ~GaussianMixture31() override;

    /** @brief Set number of Gaussian components */
    void setComponents(int k);

    /** @brief Set maximum EM iterations */
    void setMaxIterations(int iters);

    /** @brief Set convergence threshold for log-likelihood */
    void setTolerance(double tol);

    /** @brief Fit model to 2D data, returns soft cluster assignments */
    QVector<QVector<double>> fit(const QVector<QVector<double>>& data);

    /** @brief Incremental update with new data points */
    void updateIncremental(const QVector<QVector<double>>& newData);

    /** @brief Predict cluster probabilities for a point */
    QVector<double> predict(const QVector<double>& point) const;

    /** @brief Get component means */
    QVector<QVector<double>> means() const;

    /** @brief Get component weights */
    QVector<double> weights() const;

    /** @brief Get final log-likelihood */
    double logLikelihood() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fittingDone(int iterations, double logLikelihood, double timeMs);

private:
    int m_k = 3;
    int m_maxIter = 100;
    double m_tol = 1e-6;

    // ECM parameters: weights, means, covariances
    QVector<double> m_weights;
    QVector<QVector<double>> m_means;
    QVector<QVector<QVector<double>>> m_covs;

    // Sufficient statistics cache for incremental updates
    QVector<double> m_ssN;                          // effective count per component
    QVector<QVector<double>> m_ssSum;               // sum of weighted points
    QVector<QVector<QVector<double>>> m_ssOuter;    // sum of weighted outer products
    int m_totalSamples = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize parameters via k-means++ seeding */
    void initParams(const QVector<QVector<double>>& data);

    /** @brief E-step: compute responsibilities */
    QVector<QVector<double>> eStep(const QVector<QVector<double>>& data) const;

    /** @brief CM-step: update weights and means */
    void cmStepWeightsMeans(const QVector<QVector<double>>& data,
                            const QVector<QVector<double>>& resp);

    /** @brief CM-step: update covariances */
    void cmStepCovariances(const QVector<QVector<double>>& data,
                           const QVector<QVector<double>>& resp);

    /** @brief Compute log-likelihood */
    double computeLogLikelihood(const QVector<QVector<double>>& data) const;

    /** @brief Evaluate Gaussian pdf at x for component k */
    double gaussianPdf(const QVector<double>& x, int k) const;

    /** @brief Accumulate sufficient statistics from data and responsibilities */
    void accumulateSS(const QVector<QVector<double>>& data,
                      const QVector<QVector<double>>& resp);
};
