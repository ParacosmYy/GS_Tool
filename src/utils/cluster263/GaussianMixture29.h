/**
 * @file GaussianMixture29.h
 * @brief 高斯混合模型(序贯蒙特卡洛EM与重要性加权充分统计量在线流式) — Gaussian Mixture with Sequential Monte Carlo EM and Importance-Weighted Sufficient Statistics for Online Streaming
 *
 * 功能: 实现高斯混合模型(Gaussian mixture model)，采用序贯蒙特卡洛EM
 *       (sequential Monte Carlo EM)和重要性加权充分统计量(importance-weighted
 *       sufficient statistics)实现在线流式(online streaming)参数估计。
 *
 * 协作: SpectralCluster14(谱聚类) / KMeans27(K均值) / FuzzyCMeans13(模糊C均值)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型(序贯蒙特卡洛EM与重要性加权充分统计量在线流式)
 */
class GaussianMixture29 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numComponents = 0;
        int dimension = 0;
        int numSamples = 0;
        double logLikelihood = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Single Gaussian component */
    struct Component {
        double weight = 0.0;
        QVector<double> mean;
        double variance = 1.0;  // Diagonal covariance (scalar per dim)
    };

    explicit GaussianMixture29(QObject *parent = nullptr);
    ~GaussianMixture29() override;

    /** @brief Set number of components and learning rate */
    void setParameters(int numComponents, double learningRate = 0.01, int maxIterations = 50);

    /** @brief Initialize components from data using k-means seeding */
    void initialize(const QVector<QVector<double>>& data);

    /** @brief Batch EM fit on complete dataset */
    bool fit(const QVector<QVector<double>>& data);

    /** @brief Online update with a single streaming sample */
    void updateOnline(const QVector<double>& sample);

    /** @brief Get cluster assignments for data */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    /** @brief Get posterior probabilities (N x K) */
    QVector<QVector<double>> posteriorProbabilities(
        const QVector<QVector<double>>& data) const;

    /** @brief Get current components */
    QVector<Component> components() const;

    /** @brief Compute log-likelihood of data */
    double logLikelihood(const QVector<QVector<double>>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void modelUpdated(int numComponents, double logLikelihood, double timeMs);

private:
    int m_K = 3;
    double m_lr = 0.01;
    int m_maxIter = 50;
    int m_dim = 0;
    int m_n = 0;

    QVector<Component> m_components;
    QVector<double> m_suffStatN;    // Weighted count per component
    QVector<QVector<double>> m_suffStatMean;  // Weighted sum of samples
    QVector<double> m_suffStatVar;  // Weighted sum of squared deviations

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Gaussian pdf for a single sample and component */
    double gaussianPdf(const QVector<double>& x, int k) const;

    /** @brief E-step: compute responsibilities */
    QVector<QVector<double>> eStep(const QVector<QVector<double>>& data) const;

    /** @brief M-step: update parameters from responsibilities */
    void mStep(const QVector<QVector<double>>& data,
               const QVector<QVector<double>>& resp);

    /** @brief Initialize sufficient statistics buffers */
    void initSufficientStats();

    /** @brief SMC particle perturbation for online update */
    void perturbParticles(const QVector<double>& sample, double weight);
};
