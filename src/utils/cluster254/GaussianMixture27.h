/**
 * @file GaussianMixture27.h
 * @brief 高斯混合模型(确定性退火EM+温度调度全局最优收敛) — Gaussian Mixture with Deterministic Annealing EM and Temperature Scheduling for Global Optimum Convergence
 *
 * 功能: 实现高斯混合模型(Gaussian Mixture Model)，通过确定性退火EM
 *       (Deterministic Annealing EM)算法配合温度调度(temperature
 *       scheduling)逐步冷却，避免局部最优，实现全局收敛。
 *
 * 协作: KMeans16(K均值聚类) / OPTICS11(OPTICS聚类) / EMCluster14(EM聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型(确定性退火EM+温度调度)
 */
class GaussianMixture27 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numComponents = 0;
        int numPoints = 0;
        int numIterations = 0;
        double finalLogLikelihood = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief A single Gaussian component */
    struct Component {
        QVector<double> mean;
        double weight = 0.0;
        double variance = 1.0;  // Diagonal covariance (scalar per dim)
    };

    explicit GaussianMixture27(QObject *parent = nullptr);
    ~GaussianMixture27() override;

    /** @brief Set number of Gaussian components */
    void setComponents(int k);

    /** @brief Set initial temperature for annealing */
    void setInitialTemperature(double t0);

    /** @brief Set cooling rate (0 < rate < 1) */
    void setCoolingRate(double rate);

    /** @brief Set max EM iterations per temperature */
    void setMaxIterations(int iters);

    /** @brief Fit GMM to data using annealed EM */
    bool fit(const QVector<QVector<double>>& data);

    /** @brief Predict posterior probabilities for a sample */
    QVector<double> predict(const QVector<double>& sample) const;

    /** @brief Get fitted components */
    QVector<Component> components() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fittingCompleted(int components, double logLikelihood, double timeMs);

private:
    int m_k = 3;
    double m_t0 = 2.0;
    double m_coolRate = 0.9;
    int m_maxIter = 50;
    int m_dims = 0;
    int m_n = 0;

    QVector<Component> m_components;
    QVector<QVector<double>> m_resp;  // Responsibilities (n x k)
    QVector<double> m_dataMin;
    QVector<double> m_dataMax;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize components via k-means++ seeding */
    void initialize(const QVector<QVector<double>>& data);

    /** @brief E-step with temperature-scaled responsibilities */
    void eStep(double temperature);

    /** @brief M-step: update means, variances, weights */
    void mStep();

    /** @brief Compute log-likelihood of current model */
    double logLikelihood() const;

    /** @brief Gaussian PDF (diagonal covariance) */
    double gaussianPdf(const QVector<double>& x,
                       const Component& comp) const;
};
