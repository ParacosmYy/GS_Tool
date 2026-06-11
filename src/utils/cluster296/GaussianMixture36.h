/**
 * @file GaussianMixture36.h
 * @brief 高斯混合模型(序贯蒙特卡洛EM与Rao-Blackwellized粒子采样实现非高斯分量混合建模) — Gaussian Mixture with Sequential Monte Carlo EM and Rao-Blackwellized Particle Sampling for Non-Gaussian Component Mixture Modeling
 *
 * 功能: 实现高斯混合模型(Gaussian mixture model)，采用序贯蒙特卡洛EM(sequential Monte Carlo EM)
 *       与Rao-Blackwellized粒子采样(Rao-Blackwellized particle sampling)实现非高斯分量混合建模(non-Gaussian component mixture modeling)。
 *
 * 协作: OPTICS14(密度聚类) / DBSCAN(密度聚类) / KMeans(K均值聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

class GaussianMixture36 : public QObject {
    Q_OBJECT

public:
    /** @brief Single Gaussian component parameters */
    struct Component {
        double weight = 0.0;
        double mean = 0.0;
        double variance = 1.0;
        QVector<double> sufficientStats; // running sufficient statistics
    };

    /** @brief SMC particle for Rao-Blackwellized sampling */
    struct Particle {
        QVector<Component> components;
        double logWeight = 0.0;
        double normalizedWeight = 0.0;
    };

    /** @brief EM iteration result */
    struct EMResult {
        QVector<Component> components;
        double logLikelihood = -1e300;
        int iterations = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalFits = 0;
        int numComponents = 0;
        int numParticles = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussianMixture36(QObject *parent = nullptr);
    ~GaussianMixture36() override;

    void setNumComponents(int k);
    void setNumParticles(int n);
    void setMaxEMIterations(int maxIter);
    void setConvergenceTolerance(double tol);

    /** @brief Fit GMM to 1-D data using SMC-EM with Rao-Blackwellized particles */
    EMResult fit(const QVector<double>& data);

    /** @brief Compute posterior probabilities (responsibilities) */
    QVector<QVector<double>> posteriorProbabilities(const QVector<double>& data) const;

    /** @brief Sample from the fitted mixture */
    QVector<double> sample(int count) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitDone(int k, int iters, double ll, double timeMs);

private:
    int m_k = 3;
    int m_numParticles = 64;
    int m_maxIter = 100;
    double m_tol = 1e-6;
    QVector<Component> m_components;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Evaluate Gaussian PDF at x */
    double gaussianPdf(double x, double mean, double var) const;

    /** @brief Initialize particles via k-means++ seeding */
    QVector<Particle> initializeParticles(const QVector<double>& data) const;

    /** @brief E-step: compute responsibilities for one particle */
    void eStep(Particle& p, const QVector<double>& data) const;

    /** @brief M-step: update component parameters via sufficient stats */
    void mStep(Particle& p, const QVector<double>& data);

    /** @brief SMC resampling via systematic resampling */
    QVector<Particle> systematicResample(const QVector<Particle>& particles) const;

    /** @brief Move particles via Rao-Blackwellized proposal */
    void raoBlackwellizedMove(Particle& p, const QVector<double>& data);

    /** @brief Compute log-likelihood of data under given components */
    double logLikelihood(const QVector<Component>& comps, const QVector<double>& data) const;

    /** @brief Normalize particle weights */
    void normalizeWeights(QVector<Particle>& particles) const;
};
