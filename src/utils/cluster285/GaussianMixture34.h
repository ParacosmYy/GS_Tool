/**
 * @file GaussianMixture34.h
 * @brief 高斯混合模型(分裂-合并EM与组件退火自动模型阶数确定) — Gaussian Mixture with Split-and-Merge EM and Component Annealing for Automatic Model Order Determination
 *
 * 功能: 实现高斯混合模型(GMM)，采用分裂-合并EM(split-and-merge EM)
 *       与组件退火(component annealing)实现自动模型阶数确定(automatic model order determination)。
 *
 * 协作: DBSCAN17(密度聚类) / KMeans30(K-means) / OPTICS13(光学聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型(分裂-合并EM与组件退火自动模型阶数确定)
 */
class GaussianMixture34 : public QObject {
    Q_OBJECT

public:
    /** @brief Single Gaussian component */
    struct Component {
        QVector<double> mean;
        QVector<QVector<double>> covariance;
        double weight = 0.0;
        double logLikelihood = 0.0;
    };

    /** @brief Fit result */
    struct FitResult {
        QVector<Component> components;
        QVector<int> labels;
        double totalLogLikelihood = 0.0;
        double bic = 0.0;
        int optimalK = 0;
        int iterations = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numPoints = 0;
        int numComponents = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussianMixture34(QObject *parent = nullptr);
    ~GaussianMixture34() override;

    void setMaxComponents(int k);
    void setMinComponents(int k);
    void setMaxIter(int iters);
    void setTolerance(double tol);
    void setAnnealingRate(double rate);

    /** @brief Fit GMM with automatic model order determination */
    FitResult fit(const QVector<QVector<double>>& data);

    /** @brief Predict cluster probabilities for new samples */
    QVector<QVector<double>> predictProba(const QVector<QVector<double>>& samples) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitDone(int n, int k, double bic, double timeMs);

private:
    int m_maxK = 10;
    int m_minK = 1;
    int m_maxIter = 200;
    double m_tol = 1e-6;
    double m_annealRate = 0.95;
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<Component> m_components;
    int m_dim = 0;

    /** @brief Compute log of multivariate normal density */
    double logGaussian(const QVector<double>& x, const Component& comp) const;

    /** @brief E-step: compute responsibilities */
    QVector<QVector<double>> eStep(const QVector<QVector<double>>& data,
                                    const QVector<Component>& comps) const;

    /** @brief M-step: update parameters from responsibilities */
    QVector<Component> mStep(const QVector<QVector<double>>& data,
                              const QVector<QVector<double>>& resp) const;

    /** @brief Total log-likelihood */
    double totalLogLik(const QVector<QVector<double>>& data,
                       const QVector<Component>& comps) const;

    /** @brief Bayesian Information Criterion */
    double computeBIC(const QVector<QVector<double>>& data,
                      const QVector<Component>& comps) const;

    /** @brief Initialize components via k-means++ seeding */
    QVector<Component> initComponents(const QVector<QVector<double>>& data, int k) const;

    /** @brief Split a component with largest covariance trace */
    int findSplitCandidate(const QVector<Component>& comps) const;

    /** @brief Merge two closest components by KL divergence */
    void findMergePair(const QVector<Component>& comps, int& i, int& j) const;

    /** @brief Run standard EM for k components */
    FitResult runEM(const QVector<QVector<double>>& data, int k) const;

    /** @brief Matrix determinant (Cholesky-based) */
    double determinant(const QVector<QVector<double>>& mat) const;

    /** @brief Invert positive-definite matrix */
    QVector<QVector<double>> invertMatrix(const QVector<QVector<double>>& mat) const;
};
