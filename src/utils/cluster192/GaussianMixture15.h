/**
 * @file GaussianMixture15.h
 * @brief 高斯混合模型(增量EM+充分统计遗忘因子) — Gaussian Mixture Model with Incremental EM for Streaming Data and Sufficient Statistics Forgetting Factor
 *
 * 功能: 实现GMM聚类，支持增量EM算法处理流式数据、
 *       充分统计量遗忘因子自适应更新和在线模型维护。
 *
 * 协作: SpectralCluster8(谱聚类) / KMeans17(K均值) / FuzzyCMeans8(模糊C均值)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 高斯混合模型(增量EM+遗忘因子)
 */
class GaussianMixture15 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalUpdates = 0;
        int numComponents = 0;
        int numDimensions = 0;
        int streamingBatches = 0;
        double avgLogLikelihood = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussianMixture15(QObject *parent = nullptr);
    ~GaussianMixture15() override;

    void setComponents(int k);
    void setForgettingFactor(double alpha);
    void setMaxIterations(int iter);
    void setConvergenceThreshold(double eps);

    /** @brief Batch fit on full dataset */
    QVector<int> fit(const QVector<QVector<double>>& data, int k);

    /** @brief Incremental EM update with new streaming batch */
    void partialFit(const QVector<QVector<double>>& batch);

    /** @brief Predict component labels for data */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    /** @brief Compute log-likelihood of data under model */
    double logLikelihood(const QVector<QVector<double>>& data) const;

    /** @brief Get component weights */
    QVector<double> weights() const;

    /** @brief Get component means */
    QVector<QVector<double>> means() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void modelUpdated(int components, double logLik, double timeMs);

private:
    int m_k = 3;
    double m_alpha = 0.95;     // forgetting factor
    int m_maxIter = 100;
    double m_eps = 1e-6;

    // Sufficient statistics for each component
    QVector<double> m_weights;
    QVector<QVector<double>> m_means;
    QVector<QVector<QVector<double>>> m_covs;  // k x d x d
    int m_dim = 0;
    double m_totalN = 0.0;     // effective sample count

    Stats m_stats;
    double m_timeSum = 0.0;
    double m_llSum = 0.0;

    /** @brief E-step: compute responsibilities */
    QVector<QVector<double>> eStep(
        const QVector<QVector<double>>& data) const;

    /** @brief M-step: update parameters from responsibilities */
    void mStep(const QVector<QVector<double>>& data,
               const QVector<QVector<double>>& resp);

    /** @brief Multivariate Gaussian log-pdf */
    double logGaussian(const QVector<double>& x, int comp) const;

    /** @brief Compute log determinant of matrix */
    double logDet(const QVector<QVector<double>>& mat) const;

    /** @brief Solve linear system Ax=b via Cholesky */
    QVector<double> choleskySolve(
        const QVector<QVector<double>>& A,
        const QVector<double>& b) const;
};
