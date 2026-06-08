/**
 * @file GaussianMixture19.h
 * @brief 高斯混合模型(确定性退火EM+温度调度全局最优逃逸) — Gaussian Mixture with Deterministic Annealing EM and Temperature Schedule for Global Optimum Escape
 *
 * 功能: 实现高斯混合模型聚类，采用确定性退火EM算法配合温度调度策略，
 *       避免陷入局部最优，支持BIC/AIC模型选择和在线增量更新。
 *
 * 协作: DBSCAN12(密度聚类) / KMeans20(核K均值) / GaussianMixture18(高斯混合)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型(确定性退火EM+温度调度)
 */
class GaussianMixture19 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        int numComponents = 0;
        int dimensions = 0;
        double logLikelihood = 0.0;
        double bic = 0.0;
        double finalTemperature = 0.0;
        int emIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussianMixture19(QObject *parent = nullptr);
    ~GaussianMixture19() override;

    /** @brief Set parameters: components, max iterations, initial temperature */
    void setParameters(int components, int maxIter = 200,
                       double initTemp = 2.0, double coolingRate = 0.95);

    /** @brief Fit model to data using annealed EM */
    void fit(const QVector<QVector<double>>& data);

    /** @brief Predict cluster assignment for each sample */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    /** @brief Get posterior probabilities (responsibilities) */
    QVector<QVector<double>> responsibilities() const;

    /** @brief Get component means */
    QVector<QVector<double>> means() const;

    /** @brief Compute log-likelihood of data */
    double logLikelihood(const QVector<QVector<double>>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fittingCompleted(int components, double logLik, double timeMs);

private:
    int m_components = 3;
    int m_maxIter = 200;
    double m_initTemp = 2.0;
    double m_coolingRate = 0.95;
    int m_dim = 0;

    // GMM parameters
    QVector<QVector<double>> m_means;
    QVector<QVector<QVector<double>>> m_covariances;
    QVector<double> m_weights;
    QVector<QVector<double>> m_resp;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief E-step: compute responsibilities at given temperature */
    void eStep(const QVector<QVector<double>>& data, double temperature);

    /** @brief M-step: update parameters from responsibilities */
    void mStep(const QVector<QVector<double>>& data);

    /** @brief Evaluate multivariate Gaussian density */
    double gaussianPdf(const QVector<double>& x, int comp) const;

    /** @brief Initialize parameters via k-means++ seeding */
    void initialize(const QVector<QVector<double>>& data);

    /** @brief Compute BIC score */
    double computeBIC(const QVector<QVector<double>>& data) const;
};
