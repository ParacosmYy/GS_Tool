/**
 * @file GaussianMixture26.h
 * @brief 高斯混合模型(ECM期望条件最大化+ARD自动相关性确定剪枝) — Gaussian Mixture with Expectation-Conditional-Maximization and Automatic Relevance Determination Pruning
 *
 * 功能: 实现高斯混合模型(Gaussian Mixture Model)，使用ECM算法
 *       (Expectation-Conditional-Maximization)进行参数估计，ARD
 *       (Automatic Relevance Determination)自动相关性确定实现分量剪枝，
 *       无需手动指定分量数即可自动确定最优模型复杂度。
 *
 * 协作: SpectralCluster13(谱聚类) / KMeans25(K-means) / FuzzyCMeans12(模糊C均值)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型(ECM+ARD自动剪枝)
 */
class GaussianMixture26 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        int numDimensions = 0;
        int numComponents = 0;
        int numIterations = 0;
        double logLikelihood = 0.0;
        double bicScore = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussianMixture26(QObject *parent = nullptr);
    ~GaussianMixture26() override;

    /** @brief Set max components and max iterations */
    void setParams(int maxComponents, int maxIter);

    /** @brief Set ARD concentration prior (higher = more pruning) */
    void setARDConcentration(double alpha);

    /** @brief Fit GMM to data, return component labels */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Get mixing weights from last fit */
    QVector<double> weights() const;

    /** @brief Get means of each component (K x D) */
    QVector<QVector<double>> means() const;

    /** @brief Compute log-likelihood of data under current model */
    double logLikelihood() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitCompleted(int numComponents, double logLik, double timeMs);

private:
    int m_maxComponents = 10;
    int m_maxIter = 100;
    double m_alpha = 1.0;   // ARD concentration

    int m_n = 0;            // Number of samples
    int m_d = 0;            // Dimensions
    int m_k = 0;            // Active components
    QVector<double> m_weights;
    QVector<QVector<double>> m_means;       // K x D
    QVector<QVector<QVector<double>>> m_covs; // K x D x D
    QVector<QVector<double>> m_resp;         // N x K (responsibilities)
    double m_logLik = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize via k-means++ seeding */
    void initialize(const QVector<QVector<double>>& data);

    /** @brief E-step: compute responsibilities */
    void eStep();

    /** @brief CM-step: update weights with ARD pruning */
    void cmStepWeights();

    /** @brief CM-step: update means */
    void cmStepMeans();

    /** @brief CM-step: update covariances */
    void cmStepCovariances();

    /** @brief Compute log N(x | mu, Sigma) */
    double logGaussian(const QVector<double>& x, int comp) const;

    /** @brief Prune components with weight below threshold */
    void pruneComponents();

    /** @brief Compute 2x2 determinant helper */
    double det2x2(const QVector<QVector<double>>& m) const;
};
