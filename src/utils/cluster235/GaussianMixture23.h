/**
 * @file GaussianMixture23.h
 * @brief 高斯混合模型(变分贝叶斯推断+Dirichlet先验自动剪枝) — Gaussian Mixture with Variational Bayesian Inference and Automatic Component Pruning via Dirichlet Prior
 *
 * 功能: 实现变分贝叶斯高斯混合模型(Variational Bayesian GMM)，采用Dirichlet先验
 *       (Dirichlet prior)进行自动分量剪枝(automatic component pruning)，通过变分推断
 *       (variational inference)迭代优化分量权重、均值和精度矩阵的后验分布。
 *
 * 协作: SpectralCluster12(谱聚类) / KMeans23(K均值) / BirchClustering11(BIRCH聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型(变分贝叶斯推断+Dirichlet先验自动剪枝)
 */
class GaussianMixture23 : public QObject {
    Q_OBJECT

public:
    /** @brief Component posterior parameters */
    struct Component {
        double weight = 0.0;
        double nk = 0.0;
        QVector<double> mean;
        QVector<double> meanSum;
        QVector<QVector<double>> precision;
        QVector<QVector<double>> scatter;
        bool active = true;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numDimensions = 0;
        int maxComponents = 0;
        int activeComponents = 0;
        int iterations = 0;
        double lowerBound = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussianMixture23(QObject *parent = nullptr);
    ~GaussianMixture23() override;

    /** @brief Set maximum number of mixture components */
    void setMaxComponents(int k);

    /** @brief Set Dirichlet concentration prior alpha_0 */
    void setAlpha0(double alpha);

    /** @brief Set max VB iterations */
    void setMaxIterations(int iter);

    /** @brief Set convergence tolerance for lower bound */
    void setTolerance(double tol);

    /** @brief Fit model to [n x d] data */
    bool fit(const QVector<QVector<double>>& data);

    /** @brief Predict component for a new point */
    int predict(const QVector<double>& point) const;

    /** @brief Get responsibilities [n x k] */
    QVector<QVector<double>> responsibilities() const;

    /** @brief Get active component parameters */
    QVector<Component> components() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void iterationCompleted(int iter, double lowerBound, int active);
    void fitCompleted(int active, double lb, double timeMs);

private:
    int m_maxK = 10;
    double m_alpha0 = 1.0;
    int m_maxIter = 200;
    double m_tol = 1e-6;

    QVector<QVector<double>> m_data;
    QVector<QVector<double>> m_resp;  // [n x k]
    QVector<Component> m_components;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize parameters via K-means++ seeding */
    void initialize();

    /** @brief E-step: compute responsibilities */
    void eStep();

    /** @brief M-step: update component posteriors */
    void mStep();

    /** @brief Compute variational lower bound */
    double computeLowerBound() const;

    /** @brief Prune inactive components */
    void pruneComponents();

    /** @brief Multivariate Gaussian log-pdf */
    double logGaussian(const QVector<double>& x, const Component& c) const;

    /** @brief Cholesky decomposition of precision matrix */
    QVector<double> choleskySolve(const QVector<QVector<double>>& A,
                                  const QVector<double>& b) const;

    /** @brief Log of determinant from diagonal */
    double logDet(const QVector<QVector<double>>& mat) const;
};
