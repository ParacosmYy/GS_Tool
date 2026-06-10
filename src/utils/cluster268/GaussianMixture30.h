/**
 * @file GaussianMixture30.h
 * @brief 高斯混合模型(变分贝叶斯推断与ELBO优化自动分量选择) — Gaussian Mixture with Variational Bayesian Inference and Automatic Component Selection via ELBO Optimization
 *
 * 功能: 实现高斯混合模型(Gaussian mixture model)，采用变分贝叶斯推断(variational Bayesian inference)
 *       与ELBO优化(ELBO optimization)实现自动分量数选择(automatic component number selection)。
 *
 * 协作: OPTICS12(密度聚类) / KMedoids21(K-中心点) / SubspaceCluster11(子空间聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型(变分贝叶斯推断与ELBO优化自动分量选择)
 */
class GaussianMixture30 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numComponents = 0;
        int maxComponents = 0;
        double elbo = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief A single Gaussian component */
    struct Component {
        QVector<double> mean;
        double weight = 0.0;
        double precision = 1.0;
    };

    explicit GaussianMixture30(QObject *parent = nullptr);
    ~GaussianMixture30() override;

    /** @brief Set maximum number of components */
    void setMaxComponents(int k);

    /** @brief Set convergence threshold for ELBO change */
    void setTolerance(double tol);

    /** @brief Set maximum variational iterations */
    void setMaxIterations(int iters);

    /** @brief Fit model to 2D data, returns active components */
    QVector<Component> fit(const QVector<QVector<double>>& data);

    /** @brief Predict cluster assignments for data */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    /** @brief Get per-sample responsibilities (soft assignments) */
    QVector<QVector<double>> responsibilities() const;

    /** @brief Get final ELBO value */
    double elbo() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void modelUpdated(int components, double elbo, double timeMs);

private:
    int m_maxK = 10;
    double m_tol = 1e-4;
    int m_maxIter = 200;
    int m_n = 0;
    int m_dim = 0;

    QVector<Component> m_components;
    QVector<QVector<double>> m_resp;    // responsibilities [n x k]
    double m_elbo = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize parameters via k-means++ seeding */
    void initialize(const QVector<QVector<double>>& data, int k);

    /** @brief E-step: compute responsibilities */
    void eStep(const QVector<QVector<double>>& data);

    /** @brief M-step: update variational parameters */
    void mStep(const QVector<QVector<double>>& data);

    /** @brief Compute evidence lower bound */
    double computeELBO(const QVector<QVector<double>>& data) const;

    /** @brief Multivariate Gaussian log-pdf (diagonal covariance) */
    double logGaussian(const QVector<double>& x,
                       const QVector<double>& mean,
                       double precision) const;

    /** @brief Prune near-zero weight components */
    int pruneComponents();
};
