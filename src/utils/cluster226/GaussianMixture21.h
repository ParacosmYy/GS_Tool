/**
 * @file GaussianMixture21.h
 * @brief 高斯混合(蒙特卡洛EM+重要性采样不可解析E步近似) — Gaussian Mixture with Monte Carlo EM and Importance Sampling for Intractable E-step Approximation
 *
 * 功能: 实现高斯混合模型(GMM)，采用蒙特卡洛EM算法，
 *       通过重要性采样(importance sampling)近似不可解析的E步期望，
 *       支持多维数据的聚类与密度估计。
 *
 * 协作: OPTICS9(密度聚类) / KMedoids18(K中心点) / SubspaceCluster8(子空间聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合(蒙特卡洛EM+重要性采样E步近似)
 */
class GaussianMixture21 : public QObject {
    Q_OBJECT

public:
    /** @brief Single Gaussian component */
    struct Component {
        double weight = 0.0;
        QVector<double> mean;
        double variance = 1.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numComponents = 0;
        int numSamples = 0;
        int emIterations = 0;
        double logLikelihood = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussianMixture21(QObject *parent = nullptr);
    ~GaussianMixture21() override;

    /** @brief Set number of mixture components and MC samples */
    void setParameters(int numComponents, int mcSamples);

    /** @brief Fit GMM to 1-D data via Monte Carlo EM */
    bool fit(const QVector<double>& data, int maxIter = 100, double tol = 1e-6);

    /** @brief Compute posterior probabilities for a value */
    QVector<double> predict(double x) const;

    /** @brief Sample from the fitted mixture */
    QVector<double> sample(int count) const;

    /** @brief Get component parameters */
    QVector<Component> components() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitCompleted(int iters, double logLikelihood, double timeMs);
    void iterationProgress(int iter, double logLikelihood);

private:
    int m_numComponents = 3;
    int m_mcSamples = 200;
    int m_seed = 42;

    QVector<Component> m_components;
    QVector<double> m_data;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Evaluate Gaussian PDF */
    double gaussianPdf(double x, double mean, double var) const;

    /** @brief Importance sampling E-step */
    void monteCarloEStep(QVector<QVector<double>>& responsibilities);

    /** @brief M-step: update parameters from responsibilities */
    void mStep(const QVector<QVector<double>>& responsibilities);

    /** @brief Compute total log-likelihood */
    double computeLogLikelihood() const;

    /** @brief Simple LCG random number generator */
    double randUniform();
    double randNormal();
};
