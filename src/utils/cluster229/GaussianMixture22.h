/**
 * @file GaussianMixture22.h
 * @brief 高斯混合模型(序贯贝叶斯更新+充分统计量回收在线学习) — Gaussian Mixture with Sequential Bayesian Updating and Sufficient Statistics Recycling for Online Learning
 *
 * 功能: 实现高斯混合模型(GMM)，采用序贯贝叶斯更新(sequential Bayesian updating)在线学习，
 *       通过充分统计量回收(sufficient statistics recycling)高效更新分量参数，支持增量式聚类。
 *
 * 协作: DBSCAN13(密度聚类) / KMeans22(K均值) / KMedoids18(K中心点)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型(序贯贝叶斯更新+充分统计量回收)
 */
class GaussianMixture22 : public QObject {
    Q_OBJECT

public:
    /** @brief Single mixture component */
    struct Component {
        double weight = 1.0;
        QVector<double> mean;
        QVector<QVector<double>> covariance;
        // Sufficient statistics for recycling
        double nK = 0.0;
        QVector<double> sumX;
        QVector<QVector<double>> sumXXt;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numComponents = 0;
        int numDimensions = 0;
        int numSamples = 0;
        double logLikelihood = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussianMixture22(QObject *parent = nullptr);
    ~GaussianMixture22() override;

    /** @brief Set number of components and prior strength (Dirichlet alpha) */
    void setParameters(int numComponents, int dimensions, double priorAlpha = 1.0);

    /** @brief Batch EM fit on complete dataset */
    bool fit(const QVector<QVector<double>>& data, int maxIter = 100, double tol = 1e-6);

    /** @brief Online update: incorporate a single new sample */
    void updateOnline(const QVector<double>& sample);

    /** @brief Predict cluster assignment for a sample */
    int predict(const QVector<double>& sample) const;

    /** @brief Get posterior probabilities for a sample */
    QVector<double> predictProba(const QVector<double>& sample) const;

    /** @brief Get current component parameters */
    QVector<Component> components() const;

    /** @brief Compute total log-likelihood on given data */
    double logLikelihood(const QVector<QVector<double>>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitCompleted(int components, double logLikelihood, double timeMs);
    void componentUpdated(int componentId, double weight);

private:
    int m_k = 3;
    int m_d = 2;
    double m_priorAlpha = 1.0;
    double m_forgetFactor = 0.99;

    QVector<Component> m_components;
    int m_numSamples = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute Gaussian probability density */
    double gaussianPdf(const QVector<double>& x, const Component& comp) const;

    /** @brief Compute log of Gaussian PDF (numerically stable) */
    double logGaussianPdf(const QVector<double>& x, const Component& comp) const;

    /** @brief E-step: compute responsibilities */
    QVector<QVector<double>> eStep(const QVector<QVector<double>>& data) const;

    /** @brief M-step: update component parameters from responsibilities */
    void mStep(const QVector<QVector<double>>& data,
               const QVector<QVector<double>>& responsibilities);

    /** @brief Initialize components via k-means++ seeding */
    void initializeComponents(const QVector<QVector<double>>& data);

    /** @brief Compute 2x2 matrix determinant (general case via Cholesky) */
    double determinant(const QVector<QVector<double>>& mat) const;

    /** @brief Solve linear system for Mahalanobis distance */
    QVector<double> solveLinear(const QVector<QVector<double>>& A,
                                 const QVector<double>& b) const;

    /** @brief Recycle sufficient statistics with exponential forgetting */
    void recycleStatistics();

    /** @brief Invert a positive-definite matrix */
    QVector<QVector<double>> invertMatrix(const QVector<QVector<double>>& mat) const;
};
