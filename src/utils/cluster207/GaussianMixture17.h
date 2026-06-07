/**
 * @file GaussianMixture17.h
 * @brief 高斯混合模型(增量EM+在线充分统计量流式学习) — Gaussian Mixture Model with Incremental EM and Online Sufficient Statistics for Streaming Data
 *
 * 功能: 实现高斯混合模型，支持增量EM算法、
 *       在线充分统计量更新和流式数据自适应学习。
 *
 * 协作: SpectralCluster10(谱聚类) / KMeans19(K均值) / FuzzyCMeans9(模糊C均值)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型(增量EM+在线充分统计量流式学习)
 */
class GaussianMixture17 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numComponents = 0;
        int numDimensions = 0;
        int numSamples = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussianMixture17(QObject *parent = nullptr);
    ~GaussianMixture17() override;

    void setNumComponents(int k);
    void setMaxIterations(int maxIter);
    void setConvergenceThreshold(double eps);

    /** @brief Initialize GMM with prior parameters */
    void initialize(int dims, int k, const QVector<QVector<double>>& initData);

    /** @brief Batch EM fit */
    void fit(const QVector<QVector<double>>& data);

    /** @brief Incremental EM update with new streaming data chunk */
    void partialFit(const QVector<QVector<double>>& chunk);

    /** @brief Compute posterior probabilities for a single sample */
    QVector<double> predictProba(const QVector<double>& sample) const;

    /** @brief Get component responsibilities (soft assignments) */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    /** @brief Compute log-likelihood of data */
    double logLikelihood(const QVector<QVector<double>>& data) const;

    /** @brief Get mixture weights */
    QVector<double> weights() const;

    /** @brief Get component means */
    QVector<QVector<double>> means() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fittingCompleted(int components, double logLik, double timeMs);

private:
    int m_k = 3;
    int m_dims = 0;
    int m_maxIter = 100;
    double m_eps = 1e-6;

    QVector<double> m_weights;                   // Mixing coefficients
    QVector<QVector<double>> m_means;            // Component means
    QVector<QVector<QVector<double>>> m_covs;    // Component covariances

    // Online sufficient statistics accumulators
    QVector<double> m_suffN;                      // Effective count per component
    QVector<QVector<double>> m_suffMean;          // Sufficient stat for mean
    QVector<QVector<QVector<double>>> m_suffCov;  // Sufficient stat for covariance
    int m_totalSamples = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Evaluate multivariate Gaussian density */
    double gaussianPdf(const QVector<double>& x, int comp) const;

    /** @brief E-step: compute responsibilities */
    QVector<QVector<double>> eStep(const QVector<QVector<double>>& data) const;

    /** @brief M-step: update parameters from responsibilities */
    void mStep(const QVector<QVector<double>>& data,
               const QVector<QVector<double>>& resp);

    /** @brief Update online sufficient statistics */
    void updateSufficientStats(const QVector<QVector<double>>& data,
                               const QVector<QVector<double>>& resp);

    /** @brief Cholesky determinant for covariance */
    double logDetCov(int comp) const;

    /** @brief Inverse covariance via Gauss-Jordan */
    QVector<QVector<double>> inverseCov(int comp) const;
};
