/**
 * @file GaussianMixture16.h
 * @brief 高斯混合模型(在线变分贝叶斯+自动相关性确定) — GMM with Online Variational Bayes for Infinite Mixture and Automatic Relevance Determination
 *
 * 功能: 实现高斯混合模型，支持在线变分贝叶斯推理、
 *       无限混合自动相关性确定和增量更新。
 *
 * 协作: DBSCAN11(DBSCAN聚类) / KMeans18(K均值) / BirchClustering8(BIRCH聚类)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 高斯混合模型(在线变分贝叶斯+ARD)
 */
class GaussianMixture16 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFits = 0;
        int numSamples = 0;
        int activeComponents = 0;
        double lowerBound = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Gaussian component parameters */
    struct Component {
        double weight = 0.0;
        QVector<double> mean;
        QVector<double> precision;   // diagonal precision
        double shape = 0.0;          // Wishart shape
        double rate = 0.0;           // Wishart rate
        double nk = 0.0;            // effective count
    };

    explicit GaussianMixture16(QObject *parent = nullptr);
    ~GaussianMixture16() override;

    void setMaxComponents(int k);
    void setConvergenceTolerance(double tol);
    void setMaxIterations(int iters);
    void setPriorStrength(double alpha0);

    /** @brief Fit GMM to data using variational Bayes */
    QVector<int> fit(const QVector<QVector<double>>& data);

    /** @brief Online update with new mini-batch */
    void partialFit(const QVector<QVector<double>>& batch);

    /** @brief Predict cluster assignment probabilities */
    QVector<QVector<double>> predictProba(const QVector<QVector<double>>& data) const;

    /** @brief Compute variational lower bound */
    double computeLowerBound() const;

    /** @brief Get active (non-pruned) components */
    QVector<Component> activeComponents() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void fittingCompleted(int components, double lowerBound, double timeMs);

private:
    int m_maxK = 20;
    double m_tol = 1e-6;
    int m_maxIter = 200;
    double m_alpha0 = 1.0;

    int m_dim = 0;
    int m_totalSamples = 0;
    QVector<Component> m_components;

    // Sufficient statistics for online update
    QVector<QVector<double>> m_sumX;
    QVector<QVector<double>> m_sumXX;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize components via k-means++ seeding */
    void initialize(const QVector<QVector<double>>& data);

    /** @brief E-step: compute responsibilities */
    QVector<QVector<double>> eStep(const QVector<QVector<double>>& data) const;

    /** @brief M-step: update variational parameters */
    void mStep(const QVector<QVector<double>>& data,
               const QVector<QVector<double>>& resp);

    /** @brief Prune components with negligible weight */
    int pruneComponents(double threshold = 1e-3);

    /** @brief Log gaussian density with diagonal precision */
    double logGaussDiag(const QVector<double>& x, const Component& c) const;

    /** @brief Compute digamma approximation */
    static double digamma(double x);
};
