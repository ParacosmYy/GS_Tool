/**
 * @file GaussianMixture18.h
 * @brief 高斯混合模型(变分贝叶斯推断+ELBO最大化自动剪枝) — Gaussian Mixture with Variational Bayes Inference and Automatic Component Pruning via ELBO Maximization
 *
 * 功能: 实现变分贝叶斯高斯混合模型，支持ELBO下界最大化、
 *       自动分量剪枝和狄利克雷先验浓度控制。
 *
 * 协作: OPTICS8(密度聚类) / KMedoids17(K中心点) / SubspaceCluster7(子空间聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型(变分贝叶斯+ELBO自动剪枝)
 */
class GaussianMixture18 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numSamples = 0;
        int activeComponents = 0;
        int maxComponents = 0;
        int iterations = 0;
        double elbo = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief A single Gaussian component */
    struct Component {
        QVector<double> mean;
        QVector<QVector<double>> covariance;
        double weight = 0.0;
        double dirichletConc = 1.0;
        bool active = true;
    };

    explicit GaussianMixture18(QObject *parent = nullptr);
    ~GaussianMixture18() override;

    /** @brief Set maximum components and Dirichlet concentration */
    void setParameters(int maxComponents, double dirichletAlpha = 0.1);

    /** @brief Fit model to data */
    void fit(const QVector<QVector<double>>& data, int maxIter = 200,
             double tol = 1e-6);

    /** @brief Predict component responsibilities for a sample */
    QVector<double> predictResponsibilities(const QVector<double>& sample) const;

    /** @brief Get active components after pruning */
    QVector<Component> activeComponents() const;

    /** @brief Compute ELBO evidence lower bound */
    double computeELBO() const;

    /** @brief Compute log-likelihood of data */
    double logLikelihood(const QVector<QVector<double>>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fittingCompleted(int components, int iterations, double elbo, double timeMs);

private:
    int m_maxK = 10;
    double m_alpha0 = 0.1;
    int m_dim = 0;
    int m_n = 0;
    double m_beta0 = 1.0;
    double m_nu0 = 2.0;

    QVector<Component> m_components;
    QVector<QVector<double>> m_resp;      // Responsibilities N x K
    QVector<double> m_Nk;                 // Effective count per component
    double m_prevElbo = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize components via k-means++ seeding */
    void initialize(const QVector<QVector<double>>& data);

    /** @brief E-step: compute responsibilities */
    void eStep(const QVector<QVector<double>>& data);

    /** @brief M-step: update parameters */
    void mStep(const QVector<QVector<double>>& data);

    /** @brief Prune components with negligible weight */
    void pruneComponents(double threshold = 1e-3);

    /** @brief Compute multivariate Gaussian log-pdf */
    double gaussianLogPdf(const QVector<double>& x,
                          const QVector<double>& mean,
                          const QVector<QVector<double>>& cov) const;

    /** @brief Compute log-det of a matrix via Cholesky */
    double logDeterminant(const QVector<QVector<double>>& mat) const;

    /** @brief Invert a positive-definite matrix */
    QVector<QVector<double>> invertMatrix(
        const QVector<QVector<double>>& mat) const;
};
