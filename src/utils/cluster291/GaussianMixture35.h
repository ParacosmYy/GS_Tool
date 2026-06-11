/**
 * @file GaussianMixture35.h
 * @brief 高斯混合模型(增量EM与充分统计量缓存实现在线流式数据模型更新) — Gaussian Mixture with Incremental EM and Sufficient Statistics Caching for Online Streaming Data Model Updates
 *
 * 功能: 实现高斯混合模型(Gaussian mixture model)，采用增量EM(incremental EM)
 *       与充分统计量缓存(sufficient statistics caching)实现在线流式数据模型更新(online streaming data model updates)。
 *
 * 协作: SpectralCluster16(谱聚类) / KMeans31(K-means) / BirchClustering15(BIRCH聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

class GaussianMixture35 : public QObject {
    Q_OBJECT

public:
    /** @brief GMM component parameters */
    struct Component {
        double weight = 0.0;
        QVector<double> mean;
        QVector<double> variance;       // Diagonal covariance
    };

    /** @brief Sufficient statistics for incremental EM */
    struct SufficientStats {
        QVector<double> nk;             // Effective count per component
        QVector<QVector<double>> sumX;  // Sum of weighted observations
        QVector<QVector<double>> sumX2; // Sum of weighted squared observations
    };

    /** @brief Fit result */
    struct GMMResult {
        QVector<Component> components;
        QVector<int> assignments;
        double logLikelihood = 0.0;
        int numIterations = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalUpdates = 0;
        int numComponents = 0;
        int numPointsProcessed = 0;
        double avgLogLikelihood = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit GaussianMixture35(QObject *parent = nullptr);
    ~GaussianMixture35() override;

    void setNumComponents(int k);
    void setMaxIterations(int maxIter);
    void setConvergenceThreshold(double threshold);
    void setLearningRate(double alpha);

    /** @brief Batch fit on full dataset */
    GMMResult fit(const QVector<QVector<double>>& data);

    /** @brief Online update with new mini-batch */
    GMMResult updateOnline(const QVector<QVector<double>>& batch);

    /** @brief Predict component assignment probabilities */
    QVector<QVector<double>> predictProba(const QVector<QVector<double>>& data) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fitDone(int n, int k, double logLik, double timeMs);
    void onlineUpdated(int batchSize, double logLik, double timeMs);

private:
    int m_k = 3;
    int m_maxIter = 100;
    double m_tol = 1e-6;
    double m_alpha = 0.1;             // Online learning rate
    int m_dims = 0;
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_llSum = 0.0;

    QVector<Component> m_components;
    SufficientStats m_stats2;

    /** @brief Initialize components via k-means++ seeding */
    void initializeComponents(const QVector<QVector<double>>& data);

    /** @brief Compute log probability of point under component */
    double logComponentProb(const QVector<double>& x, const Component& c) const;

    /** @brief E-step: compute responsibilities */
    QVector<QVector<double>> eStep(const QVector<QVector<double>>& data) const;

    /** @brief M-step: update parameters from responsibilities */
    void mStep(const QVector<QVector<double>>& data,
               const QVector<QVector<double>>& resp);

    /** @brief Accumulate sufficient statistics */
    void accumulateStats(const QVector<QVector<double>>& data,
                         const QVector<QVector<double>>& resp);

    /** @brief Compute total log-likelihood */
    double computeLogLikelihood(const QVector<QVector<double>>& data) const;
};
