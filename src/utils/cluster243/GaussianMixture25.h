/**
 * @file GaussianMixture25.h
 * @brief 高斯混合模型(分裂合并蒙特卡洛+贝叶斯信息准则最优分量选择) — Gaussian Mixture with Split-Merge Monte Carlo and BIC for Optimal Component Count
 *
 * 功能: 实现高斯混合模型(Gaussian Mixture Model)，使用分裂合并蒙特卡洛
 *       (split-merge Monte Carlo)采样进行分量操作，通过贝叶斯信息准则
 *       (BIC/Bayesian Information Criterion)自动选择最优分量数。
 *
 * 协作: DBSCAN14(密度聚类) / KMeans24(K-means) / OPTICS10(排序聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型(分裂合并蒙特卡洛+BIC最优分量选择)
 */
class GaussianMixture25 : public QObject {
    Q_OBJECT

public:
    /** @brief Single Gaussian component */
    struct Component {
        QVector<double> mean;
        QVector<QVector<double>> covariance;
        double weight = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numComponents = 0;
        int numSamples = 0;
        int numDimensions = 0;
        double bicValue = 0.0;
        double logLikelihood = 0.0;
        int emIterations = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussianMixture25(QObject *parent = nullptr);
    ~GaussianMixture25() override;

    /** @brief Set maximum number of components */
    void setMaxComponents(int k);

    /** @brief Set minimum components */
    void setMinComponents(int k);

    /** @brief Set EM convergence threshold */
    void setConvergenceThreshold(double eps);

    /** @brief Set maximum EM iterations */
    void setMaxIterations(int iters);

    /** @brief Fit model to data, auto-select component count via BIC */
    void fit(const QVector<QVector<double>>& data);

    /** @brief Get cluster assignments (hard assignment via posterior) */
    QVector<int> labels() const;

    /** @brief Get soft assignments (posterior probabilities) per sample */
    QVector<QVector<double>> posteriors() const;

    /** @brief Get fitted components */
    QVector<Component> components() const;

    /** @brief Compute BIC for given component count */
    double computeBIC(const QVector<QVector<double>>& data, int k) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fittingCompleted(int numComponents, double bic, double timeMs);

private:
    int m_maxK = 10;
    int m_minK = 1;
    double m_tol = 1e-6;
    int m_maxIter = 200;

    QVector<QVector<double>> m_data;
    QVector<Component> m_components;
    QVector<QVector<double>> m_posteriors;
    QVector<int> m_labels;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize components via k-means++ seeding */
    void initKMeansPP(int k);

    /** @brief EM algorithm for fixed component count */
    double runEM(int k);

    /** @brief E-step: compute posterior probabilities */
    void eStep(int k);

    /** @brief M-step: update parameters from posteriors */
    void mStep(int k);

    /** @brief Multivariate Gaussian PDF */
    double gaussianPDF(const QVector<double>& x, const Component& c) const;

    /** @brief Split-merge move to escape local optima */
    void splitMergeMove(int k);

    /** @brief Compute log-likelihood */
    double logLikelihood() const;

    /** @brief Determinant of matrix */
    double determinant(const QVector<QVector<double>>& mat) const;

    /** @brief Invert positive-definite matrix (Cholesky-based) */
    QVector<QVector<double>> invertMatrix(const QVector<QVector<double>>& mat) const;

    /** @brief Random integer in [a, b) */
    int randInt(int a, int b) const;
};
