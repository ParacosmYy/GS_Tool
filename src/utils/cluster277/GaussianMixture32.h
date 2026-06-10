/**
 * @file GaussianMixture32.h
 * @brief 高斯混合模型(分裂合并EM与BIC自动模型选择) — Gaussian Mixture with Split-and-Merge EM and Bayesian Information Criterion for Automatic Model Selection
 *
 * 功能: 实现高斯混合模型(Gaussian mixture model)，采用分裂合并EM(split-and-merge EM)
 *       与贝叶斯信息准则(Bayesian information criterion)实现自动模型选择(automatic model selection)。
 *
 * 协作: SpectralCluster15(谱聚类) / KMeans29(K均值) / DBSCAN16(密度聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型(分裂合并EM与BIC自动模型选择)
 */
class GaussianMixture32 : public QObject {
    Q_OBJECT

public:
    /** @brief Single Gaussian component */
    struct Component {
        double weight = 1.0;
        QVector<double> mean;
        QVector<QVector<double>> covariance;   // Full covariance matrix
        double logLikelihood = 0.0;
    };

    /** @brief Fitting result */
    struct GMMResult {
        QVector<Component> components;
        QVector<int> labels;
        double bic = 0.0;
        double logLikelihood = 0.0;
        int optimalK = 0;
        int iterations = 0;
        bool converged = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numComponents = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussianMixture32(QObject *parent = nullptr);
    ~GaussianMixture32() override;

    /** @brief Set max number of components for model selection */
    void setMaxComponents(int k);

    /** @brief Set convergence tolerance */
    void setTolerance(double tol);

    /** @brief Set max EM iterations per run */
    void setMaxIterations(int iter);

    /** @brief Fit GMM with automatic k selection via BIC */
    GMMResult fit(const QVector<QVector<double>>& data, int k = 0);

    /** @brief Compute BIC for given k */
    double computeBIC(const QVector<QVector<double>>& data, int k) const;

    /** @brief Compute log-likelihood of data under model */
    double logLikelihood(const QVector<QVector<double>>& data,
                          const QVector<Component>& comps) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fittingDone(int k, double bic, double timeMs);
    void componentSplit(int idx, double newBIC);
    void componentMerged(int i, int j, double newBIC);

private:
    int m_maxComponents = 10;
    double m_tolerance = 1e-6;
    int m_maxIter = 200;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Run EM for fixed k */
    QVector<Component> runEM(const QVector<QVector<double>>& data, int k,
                              QVector<int>& labels, double& ll);

    /** @brief E-step: compute responsibilities */
    void eStep(const QVector<QVector<double>>& data,
               const QVector<Component>& comps,
               QVector<QVector<double>>& resp);

    /** @brief M-step: update parameters */
    void mStep(const QVector<QVector<double>>& data,
               const QVector<QVector<double>>& resp,
               QVector<Component>& comps);

    /** @brief Multivariate Gaussian log-pdf */
    double gaussianLogPdf(const QVector<double>& x,
                           const Component& comp) const;

    /** @brief Compute determinant of matrix */
    double determinant(const QVector<QVector<double>>& mat) const;

    /** @brief Invert positive-definite matrix */
    QVector<QVector<double>> invertMatrix(const QVector<QVector<double>>& mat) const;

    /** @brief Split-and-merge step to escape local optima */
    void splitAndMerge(QVector<Component>& comps,
                        const QVector<QVector<double>>& data,
                        const QVector<int>& labels);

    /** @brief Euclidean distance squared */
    double distSq(const QVector<double>& a, const QVector<double>& b) const;
};
