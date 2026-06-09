/**
 * @file GaussianMixture28.h
 * @brief 高斯混合模型(在线序列EM+遗忘因子流式自适应) — Gaussian Mixture Model with Online Sequential EM and Forgetting Factor for Streaming Data Adaptation
 *
 * 功能: 实现高斯混合模型(GMM)的在线序列EM算法，采用遗忘因子
 *       (forgetting factor)实现流式数据自适应，支持增量更新和
 *       实时聚类，适合非平稳数据分布。
 *
 * 协作: DBSCAN15(HDBSCAN聚类) / KMeans26(核K均值) / OPTICS11(OPTICS聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型(在线序列EM+遗忘因子流式自适应)
 */
class GaussianMixture28 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numComponents = 0;
        int numSamples = 0;
        int numDimensions = 0;
        double logLikelihood = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Single Gaussian component */
    struct Component {
        double weight = 1.0;
        QVector<double> mean;
        QVector<double> variance;      // Diagonal covariance
        double logWeight = 0.0;
    };

    explicit GaussianMixture28(QObject *parent = nullptr);
    ~GaussianMixture28() override;

    /** @brief Set number of Gaussian components */
    void setNumComponents(int k);

    /** @brief Set forgetting factor (0 < alpha <= 1) */
    void setForgettingFactor(double alpha);

    /** @brief Initialize from batch data via k-means++ seeding */
    bool initialize(const QVector<QVector<double>>& data);

    /** @brief Online update with single sample */
    void update(const QVector<double>& sample);

    /** @brief Batch EM iteration */
    void batchEM(const QVector<QVector<double>>& data, int maxIter = 50);

    /** @brief Get component responsibilities for a sample */
    QVector<double> responsibilities(const QVector<double>& sample) const;

    /** @brief Predict cluster label for a sample */
    int predict(const QVector<double>& sample) const;

    /** @brief Get all components */
    QVector<Component> components() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void modelUpdated(int components, int samples, double logLikelihood, double timeMs);

private:
    int m_k = 3;
    double m_alpha = 0.98;         // Forgetting factor
    int m_dims = 0;
    int m_sampleCount = 0;

    QVector<Component> m_components;
    double m_logLikelihood = 0.0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Compute Gaussian PDF (diagonal covariance) */
    double gaussianPdf(const QVector<double>& x, const Component& c) const;

    /** @brief Compute log Gaussian PDF for numerical stability */
    double logGaussianPdf(const QVector<double>& x, const Component& c) const;

    /** @brief Sufficient statistics update with forgetting */
    void updateSufficientStats(const QVector<double>& x, const QVector<double>& resp);

    /** @brief K-means++ seeding for initialization */
    void kMeansPPSeeding(const QVector<QVector<double>>& data);

    /** @brief Recompute component parameters from stats */
    void recomputeParameters();
};
