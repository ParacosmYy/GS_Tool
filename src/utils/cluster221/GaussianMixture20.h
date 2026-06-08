/**
 * @file GaussianMixture20.h
 * @brief 高斯混合模型(折叠Gibbs采样+Dirichlet过程非参数分量数) — Gaussian Mixture Model with Collapsed Gibbs Sampling and Dirichlet Process Nonparametric Component Count
 *
 * 功能: 实现高斯混合模型推理，采用折叠Gibbs采样消除显式参数积分，
 *       集成Dirichlet过程先验自动推断最优分量数。
 *
 * 协作: SpectralCluster11(谱聚类) / KMeans21(K均值) / FuzzyCMeans10(模糊聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型(折叠Gibbs+Dirichlet过程)
 */
class GaussianMixture20 : public QObject {
    Q_OBJECT

public:
    /** @brief Sampling result with assignments and parameters */
    struct GMMResult {
        QVector<int> assignments;
        QVector<QVector<double>> means;
        QVector<QVector<QVector<double>>> covariances;
        QVector<double> weights;
        int numComponents = 0;
        double logLikelihood = 0.0;
        int iterations = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int dim = 0;
        int maxComponents = 0;
        int activeComponents = 0;
        int totalIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussianMixture20(QObject *parent = nullptr);
    ~GaussianMixture20() override;

    /** @brief Set hyperparameters: alpha (DP concentration), max components, burn-in, samples */
    void setParameters(int maxComponents = 20, double alpha = 1.0,
                       int burnIn = 100, int numSamples = 200);

    /** @brief Fit model with automatic component count via Dirichlet process */
    GMMResult fit(const QVector<QVector<double>>& data);

    /** @brief Predict cluster assignments for new data */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    /** @brief Get component means */
    QVector<QVector<double>> means() const;

    /** @brief Get component weights */
    QVector<double> weights() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void samplingCompleted(int components, int iterations, double logLikelihood, double timeMs);

private:
    int m_maxComponents = 20;
    double m_alpha = 1.0;
    int m_burnIn = 100;
    int m_numSamples = 200;
    int m_dim = 0;
    int m_n = 0;

    QVector<int> m_assignments;
    QVector<QVector<double>> m_means;
    QVector<double> m_weights;
    QVector<int> m_counts;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute squared Mahalanobis-like distance (diagonal covariance) */
    double clusterDistance(const QVector<double>& point, int comp) const;

    /** @brief Compute predictive likelihood under collapsed Gibbs */
    double collapsedLikelihood(const QVector<double>& point, int comp, int compCount) const;

    /** @brief Compute marginal likelihood for new component */
    double marginalLikelihood(const QVector<double>& point) const;

    /** @brief Sample assignment for one data point */
    int sampleAssignment(int pointIdx, const QVector<QVector<double>>& data);

    /** @brief Update sufficient statistics after full sweep */
    void updateStatistics(const QVector<QVector<double>>& data);

    /** @brief Compute log-likelihood of current assignment */
    double computeLogLikelihood(const QVector<QVector<double>>& data) const;

    /** @brief Multivariate log-gamma helper */
    static double logMVGamma(int d, double x);
};
