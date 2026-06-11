/**
 * @file GaussianMixture37.h
 * @brief 高斯混合模型(变分贝叶斯推断与自动相关性确定实现无需交叉验证的原理化模型选择) — Gaussian Mixture with Variational Bayesian Inference and Automatic Relevance Determination for Principled Model Selection Without Cross-Validation
 *
 * 功能: 实现高斯混合模型(Gaussian mixture model)，采用变分贝叶斯推断(variational Bayesian inference)
 *       与自动相关性确定(automatic relevance determination)实现无需交叉验证的原理化模型选择(principled model selection)。
 *
 * 协作: DBSCAN18(密度聚类) / KMeans32(K均值聚类) / GaussianMixture36(高斯混合模型)
 */
#pragma once

#include <QObject>
#include <QVector>

class GaussianMixture37 : public QObject {
    Q_OBJECT

public:
    /** @brief Single Gaussian component */
    struct Component {
        double weight = 0.0;
        QVector<double> mean;
        QVector<QVector<double>> covariance;
        double relevance = 1.0;          // ARD weight: near 0 => pruned
    };

    /** @brief Fit result */
    struct FitResult {
        QVector<Component> components;
        QVector<int> assignments;
        double lowerBound = 0.0;         // variational lower bound (ELBO)
        int activeComponents = 0;
        int iterations = 0;
        double elapsedMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalFits = 0;
        int maxComponentsUsed = 0;
        double avgIterations = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussianMixture37(QObject *parent = nullptr);
    ~GaussianMixture37() override;

    void setMaxComponents(int k);
    void setMaxIterations(int iter);
    void setConvergenceThreshold(double tol);
    void setARDPrior(double alpha0);

    /** @brief Fit with variational Bayesian inference and ARD pruning */
    FitResult fit(const QVector<QVector<double>>& data);

    /** @brief Predict cluster assignments */
    QVector<int> predict(const QVector<QVector<double>>& data) const;

    /** @brief Compute responsibility matrix (N x K) */
    QVector<QVector<double>> responsibilities(const QVector<QVector<double>>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitDone(int activeComponents, double lowerBound, int iterations, double timeMs);

private:
    int m_maxK = 10;
    int m_maxIter = 200;
    double m_tol = 1e-6;
    double m_alpha0 = 1.0;              // ARD prior (Dirichlet concentration)
    int m_dims = 0;
    QVector<Component> m_components;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize components via k-means++ seeding */
    void initialize(const QVector<QVector<double>>& data);

    /** @brief Compute log N(x | mu, Sigma) */
    double logGaussian(const QVector<double>& x, const Component& comp) const;

    /** @brief E-step: compute responsibilities */
    QVector<QVector<double>> eStep(const QVector<QVector<double>>& data) const;

    /** @brief M-step: update parameters with ARD */
    void mStep(const QVector<QVector<double>>& data,
               const QVector<QVector<double>>& resp);

    /** @brief Compute variational lower bound (ELBO) */
    double computeLowerBound(const QVector<QVector<double>>& data,
                             const QVector<QVector<double>>& resp) const;

    /** @brief Prune components with low relevance */
    int pruneComponents();

    /** @brief 2D matrix determinant */
    double determinant(const QVector<QVector<double>>& mat) const;

    /** @brief 2D matrix inverse */
    QVector<QVector<double>> inverse(const QVector<QVector<double>>& mat) const;
};
